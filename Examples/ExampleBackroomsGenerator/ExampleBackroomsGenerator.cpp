#include "BBE/BrotBoxEngine.h"
#include "Rooms.h"
#include "AssetStore.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// Necessary before showcase:
// TODO: Relaxed Baking mode for rooms that aren't directly visible but probably will be soon
// TODO: Wall Collision with player
// TODO: power outlet
// TODO: Footstep sounds
// TODO: Does Laptop hit 60 FPS?
// TODO: Bloom for lights?
// TODO: Disable imgui and debug drawing in release mode
// TODO: Emscripten
// TODO: Proper Emscripten page
// TODO: Different humming frequencies for lights
// 
// Nice to have (I guess?)
// TODO: VAO buffers?
// TODO: Experiments with Shadow Maps (first experiments indicate that they are waaaay too slow. Can we do better? Heavily reduced the lights that need to be taken into account, maybe it's better now)
// TODO: Variable room height (PROBLEM: If all lights in neighboring rooms are higher than the own ceiling, the ceiling will not be lit by any of them. If we don't have a light ourselves, the ceiling is completely black)?
// TODO: Flickering lights (PROBLEM: We are currently rendering in forward mode with purely baked lights)

namespace br
{
	class BackgroundHum : public bbe::SoundDataSourceDynamic
	{
	public:
		static constexpr uint32_t hz = 44100;

		struct SimpleWave
		{
			float val = 0.0f;
			float inc = 0.0f;

			SimpleWave(float inc) : inc(inc) {}
			float sample()
			{
				val += inc;
				if (val > 1.0f && inc > 0)
				{
					inc *= -1.0f;
					val = 1.0f;
				}
				else if (val < 0.0f && inc < 0)
				{
					inc *= -1.0f;
					val = 0.0f;
				}
				return val;
			}
		};
		mutable bbe::List<SimpleWave> waves;
		bbe::Random rand;

		BackgroundHum()
		{
			rand.setSeed(17);
			newSound();
		}

		void newSound()
		{
			waves.clear();
			for (size_t i = 0; i < 10; i++)
			{
				waves.add(SimpleWave(rand.randomFloat() / 1000.f));
			}
		}

		virtual float getSample(size_t i, uint32_t channel) const override
		{
			float retVal = 0;
			for (size_t i = 0; i < waves.getLength(); i++)
			{
				retVal += waves[i].sample();
			}
			retVal /= waves.getLength();

			return retVal * 0.5f;
		}

		virtual uint32_t getHz() const override
		{
			return hz;
		}

		virtual uint32_t getAmountOfChannels() const override
		{
			return 1;
		}
	};

	class LightBuzz : public bbe::SoundDataSourceDynamic
	{
	public:
		virtual float getSample(size_t i, uint32_t channel) const override
		{
			float fadeIn = 1.0f;
			size_t fadeInTime = 1000;
			if (i < fadeInTime)
			{
				fadeIn = (float)i / (float)fadeInTime;
			}
			return float(bbe::Math::cos(i * 0.01233f) + bbe::Math::cos(i * 0.016435f) + bbe::Math::cos(i * 0.01f)) * 0.03f * fadeIn;
		}

		virtual uint32_t getHz() const override
		{
			return 44100;
		}

		virtual uint32_t getAmountOfChannels() const override
		{
			return 1;
		}
	};

	class BackroomsGenerator : public bbe::Game
	{
	private:
		bbe::Random rand;
		Rooms rooms;
		bbe::Vector2 cameraPos;

		int32_t expandRoom = 0;

		float roomAlpha = 1.f;
		float gateAlpha = 0.f;
		float gateConnectionsAlpha = 0.5f;

		int32_t hoveredRoom = -1;
		int32_t stopGenerationAtRooms = 1000000;

		int32_t currentSeed = 2;
		bool autoExpand = false;

		int renderMode = 1;

		bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);

		bbe::Image debugBake;

		bool drawFloor = true;
		bool drawWalls = true;
		bool drawSkirtingBoard = true;
		bool drawCeiling = true;
		bool drawLights = true;

		BackgroundHum backgroundHum;
		LightBuzz lightBuzz;
	public:
		void newRooms()
		{
			rooms.clear();
			currentSeed++;
			rooms.setSeed(currentSeed);
			rooms.generateAtPoint(bbe::Vector2i(0, 0));
			expandRoom = 0;
			for (int i = 0; i < 13; i++)
			{
				expand();
			}
		}

		void expand()
		{
			rooms.connectGates(expandRoom);
			expandRoom++;
		}

		virtual void onStart() override
		{
#ifndef __EMSCRIPTEN__ // The sound is currently pretty broken on Emscripten.
			backgroundHum.play();
#endif
			rand.setSeed(17);

			newRooms();
			ccnc.constraintZPos(1.8f);
			ccnc.setSpeedBuildUp(bbe::CameraControlNoClip::SpeedBuildUp::EXPONENTIAL);
		}

		void update2D(float timeSinceLastFrame)
		{
			if (renderMode != 0) return;

			//rooms.generateAtPoint(bbe::Vector2i(WINDOW_WIDTH / 2 + cameraPos.x, WINDOW_HEIGHT / 2 + cameraPos.y));

			for (int i = 0; i < 1024 * 1; i++)
			{
				if (!autoExpand || rooms.rooms.getLength() >= stopGenerationAtRooms) break;
				rooms.connectGates(expandRoom);
				expandRoom++;
			}

			BBELOGLN("# Rooms: " << rooms.rooms.getLength());

			float cameraSpeed = 300.f;
			if (isKeyDown(bbe::Key::LEFT_SHIFT))
			{
				cameraSpeed *= 10;
			}

			if (isKeyDown(bbe::Key::W))
			{
				cameraPos.y -= timeSinceLastFrame * cameraSpeed;
			}
			if (isKeyDown(bbe::Key::S))
			{
				cameraPos.y += timeSinceLastFrame * cameraSpeed;
			}
			if (isKeyDown(bbe::Key::A))
			{
				cameraPos.x -= timeSinceLastFrame * cameraSpeed;
			}
			if (isKeyDown(bbe::Key::D))
			{
				cameraPos.x += timeSinceLastFrame * cameraSpeed;
			}

			bbe::Vector2 mouseWorldPos = getMouse() + cameraPos;
			bbe::Vector2i mouseWorldPosi((int32_t)mouseWorldPos.x, (int32_t)mouseWorldPos.y);
			hoveredRoom = rooms.getRoomIndexAtPoint(mouseWorldPosi);
		}

		void update3D(float timeSinceLastFrame)
		{
			if (renderMode != 1) return;

			{
				// Baking tests
				const Room& r = rooms.rooms[0];
				//debugBake = bakeLights(bbe::Matrix4(), r.wallsModel, nullptr, nullptr, nullptr, assetStore::Wall(), { 1, 1, 1, 1 }, { 256, 256 }, r.lights);
			}

			ccnc.update(timeSinceLastFrame * 0.2f);
			rooms.update(timeSinceLastFrame, ccnc.getCameraPos(), lightBuzz);
			//ccnc.setCameraForward(bbe::Vector3(1, 0, 0));
			float fps = (1 / timeSinceLastFrame);
			static float minFps = 100000;
			if (fps < minFps)
			{
				minFps = fps;
				BBELOGLN(fps);
			}

			setSoundListener(ccnc.getCameraPos(), ccnc.getCameraForward());
		}

		virtual void update(float timeSinceLastFrame) override
		{
			update2D(timeSinceLastFrame);
			update3D(timeSinceLastFrame);
		}

		void drawImgui()
		{
			const char* const modes[] = {"2D", "3D"};
			ImGui::Combo("Render Mode", &renderMode, modes, 2, 10);
			if (renderMode == 0)
			{
				ImGui::SliderFloat("roomAlpha: ", &roomAlpha, 0.f, 1.f);
				ImGui::SliderFloat("GateAlpha: ", &gateAlpha, 0.f, 1.f);
				ImGui::SliderFloat("gateConnectionsAlpha: ", &gateConnectionsAlpha, 0.f, 1.f);
				ImGui::LabelText("Seed: ", "%i", currentSeed);
				ImGui::LabelText("HoveredId: ", "%i", hoveredRoom);
				ImGui::LabelText("Expand: ", "%i", expandRoom);
				ImGui::Checkbox("autoExpand", &autoExpand);
				if (ImGui::Button("New Seed"))
				{
					newRooms();
					return;
				}
				if (ImGui::Button("Expand"))
				{
					expand();
					return;
				}

				ImGui::InputInt("stopGenerationAtRooms", &stopGenerationAtRooms);
			}
			else if (renderMode == 1)
			{
				ImGui::Checkbox("Draw Floor", &drawFloor);
				ImGui::Checkbox("Draw Walls", &drawWalls);
				ImGui::Checkbox("Draw Skirting Board", &drawSkirtingBoard);
				ImGui::Checkbox("Draw Ceiling", &drawCeiling);
				ImGui::Checkbox("Draw Lights", &drawLights);

				auto pos = ccnc.getCameraPos();
				ImGui::LabelText("Pos", "%f/%f/%f", pos.x, pos.y, pos.z);
				bool inWall = rooms.isPositionInWall(pos);
				ImGui::LabelText("In Wall", "%s", inWall ? "true" : "false");
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
		}

		virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
		{
			if (renderMode != 1) return;
			brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
			//brush.setCamera(bbe::Vector3(0, 0, 1.7f), bbe::Vector3(-1, 0, 1.7f));

			brush.setRenderMode(bbe::RenderMode::FORWARD_NO_LIGHTS);
			rooms.drawAt(ccnc.getCameraPos(), brush, assetStore::Floor(), assetStore::Wall(), assetStore::Ceiling(), assetStore::SkirtingBoard(), drawFloor, drawWalls, drawSkirtingBoard, drawCeiling, drawLights);
		}

		virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
		{
			drawImgui();
			//brush.drawImage(0, 0, debugBake);

			if (renderMode != 0)
			{
				bbe::String fps = "FPS: ";
				fps += 1.f / getAverageFrameTime();
				brush.fillText(25, 25, fps);

				bbe::String lowFps = "Low FPS: ";
				lowFps += 1.f / getHighestFrameTime();
				brush.fillText(25, 50, lowFps);

				bbe::String drawCalls = "Drawcalls: ";
				drawCalls += getAmountOfDrawcalls();
				brush.fillText(25, 75, drawCalls);

				static uint32_t maxDrawCalls = 0;
				if (getAmountOfDrawcalls() > maxDrawCalls)
				{
					maxDrawCalls = getAmountOfDrawcalls();
				}
				bbe::String maxDrawCallsStr = "Max Drawcalls: ";
				maxDrawCallsStr += maxDrawCalls;
				brush.fillText(25, 100, maxDrawCallsStr);
				return;
			}

			bbe::Rectanglei cameraRect((int32_t)cameraPos.x, (int32_t)cameraPos.y, WINDOW_WIDTH, WINDOW_HEIGHT);
			for (const Room& r : rooms.rooms)
			{
				if (cameraRect.intersects(r.boundingBox))
				{
					if (roomAlpha > 0)
					{
						//TODO Debug code, remove me
						if (r.id == 8 || r.id == 9)
						{
							static int hue = 0;
							brush.setColorHSV((float)hue, r.saturation, r.value, roomAlpha);
							hue++;
						}
						else
						{ 
							brush.setColorHSV(r.hue, r.saturation, r.value, roomAlpha);
						}
						brush.fillRect(r.boundingBox.x - cameraPos.x, r.boundingBox.y - cameraPos.y, r.boundingBox.width, r.boundingBox.height);
					}

					if (gateAlpha > 0)
					{
						for (const Neighbor& n : r.neighbors)
						{
							const Room& neighborRoom = rooms.rooms[n.neighborId];
							brush.setColorHSV(neighborRoom.hue, neighborRoom.saturation, neighborRoom.value, gateAlpha);
							for (const Gate& g : n.gates)
							{
								brush.fillRect(g.ownGatePos.x - cameraPos.x, g.ownGatePos.y - cameraPos.y, 1, 1);
							}
						}
					}

					if (gateConnectionsAlpha > 0)
					{
						brush.setColorRGB(1, 1, 1, gateConnectionsAlpha);
						for (int32_t x = 0; x < r.walkable.getWidth(); x++)
						{
							for (int32_t y = 0; y < r.walkable.getHeight(); y++)
							{
								if (r.walkable[x][y])
								{
									brush.fillRect(r.boundingBox.x + x - cameraPos.x, r.boundingBox.y + y - cameraPos.y, 1, 1);
								}
							}
						}
					}
				}
			}

			if (hoveredRoom >= 0)
			{
				const Room& room = rooms.rooms[hoveredRoom];
				brush.setColorRGB(1, 1, 1, 0.2f);
				brush.fillRect(room.boundingBox.x - cameraPos.x, room.boundingBox.y - cameraPos.y, room.boundingBox.width, room.boundingBox.height);

				brush.setColorRGB(0, 0, 0, 0.5f);
				for (const Neighbor& n : room.neighbors)
				{
					const Room& neighborRoom = rooms.rooms[n.neighborId];
					auto myCenter = room.boundingBox.getCenter();
					bbe::Vector2 myCenterf = bbe::Vector2(myCenter.x, myCenter.y);
					auto otherCenter = neighborRoom.boundingBox.getCenter();
					bbe::Vector2 otherCenterf = bbe::Vector2(otherCenter.x, otherCenter.y);
					brush.fillLine(myCenterf - cameraPos, otherCenterf - cameraPos);
				}
			}

		}

		virtual void onEnd() override
		{
		}
	};
}

int main()
{
	br::BackroomsGenerator *mg = new br::BackroomsGenerator();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Backrooms Generator");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
