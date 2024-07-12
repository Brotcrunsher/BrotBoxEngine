#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <Windows.h>

// TODO: Proper GUI
// TODO: Layers
// TODO: Text
// TODO: Select+Move Tool
// TODO: Color Selector Tool
// TODO: Drag and Drop image files into paint
// TODO: Tiled view is kinda messed. Zooming out draws too few, line drawer seems to skip, etc...
// TODO: Flood Fill should fill over the edge if tiled is selected.

class MyGame : public bbe::Game
{
	bbe::Vector2 offset;
	bbe::String path;
	bbe::Image canvas;
	bbe::Image workArea;
	float zoomLevel = 1.f;

	float leftColor[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr static int32_t MODE_BRUSH      = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	int32_t mode = MODE_BRUSH;

	// MODE_BRUSH
	int32_t brushWidth = 1;

	bool drawGridLines = true;
	bool tiled = false;

	void resetCamera()
	{
		offset = bbe::Vector2(getWindowWidth() / 2 - canvas.getWidth() / 2, getWindowHeight() / 2 - canvas.getHeight() / 2);
		zoomLevel = 1.f;
	}

	void clearWorkArea()
	{
		workArea = bbe::Image(canvas.getWidth(), canvas.getHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		workArea.keepAfterUpload();
		workArea.setFilterMode(bbe::ImageFilterMode::NEAREST);
	}

	void applyWorkArea()
	{
		// TODO: This should probably be moved to the image class
		for (size_t i = 0; i < workArea.getWidth(); i++)
		{
			for (size_t k = 0; k < workArea.getHeight(); k++)
			{
				const bbe::Colori c = workArea.getPixel(i, k);
				if (c.a == 0) continue;

				const bbe::Colori oldColor = canvas.getPixel(i, k);
				const bbe::Colori newColor = oldColor.blendTo(c);
				canvas.setPixel(i, k, newColor);
			}
		}
		clearWorkArea();
	}

	void setupCanvas()
	{
		canvas.keepAfterUpload();
		canvas.setFilterMode(bbe::ImageFilterMode::NEAREST);
		clearWorkArea();
		resetCamera();
	}

	void newCanvas(uint32_t width, uint32_t height)
	{
		canvas = bbe::Image(width, height, bbe::Color::white());
		this->path = "";
		setupCanvas();
	}

	void newCanvas(const char* path)
	{
		canvas = bbe::Image(path);
		this->path = path;
		setupCanvas();
	}
	
	bbe::Vector2 screenToCanvas(const bbe::Vector2& pos)
	{
		return (pos - offset) / zoomLevel;
	}

	bool toTiledPos(bbe::Vector2& pos)
	{
		if (tiled)
		{
			pos.x = bbe::Math::mod<float>(pos.x, canvas.getWidth());
			pos.y = bbe::Math::mod<float>(pos.y, canvas.getHeight());
			return true; // If we are tiled, then any position is always within the canvas.
		}

		// If we are not tiled, then we have to check if the pos is actually part of the canvas.
		return pos.x >= 0 && pos.y >= 0 && pos.x < canvas.getWidth() && pos.y < canvas.getHeight();
	}

	void changeZoom(float val)
	{
		auto mouseBeforeZoom = screenToCanvas(getMouse());
		zoomLevel *= val;
		auto mouseAfterZoom = screenToCanvas(getMouse());
		offset += (mouseAfterZoom - mouseBeforeZoom) * zoomLevel;
	}

	bbe::Colori getMouseColor() const
	{
		return isMouseDown(bbe::MouseButton::LEFT) ? bbe::Color(leftColor).asByteColor() : bbe::Color(rightColor).asByteColor();
	}

	virtual void onStart() override
	{
		newCanvas(400, 300);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyPressed(bbe::Key::SPACE))
		{
			resetCamera();
		}

		constexpr float CAM_WASD_SPEED = 400;
		if (isKeyDown(bbe::Key::W))
		{
			offset.y += timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (isKeyDown(bbe::Key::S))
		{
			offset.y -= timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (isKeyDown(bbe::Key::A))
		{
			offset.x += timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (isKeyDown(bbe::Key::D))
		{
			offset.x -= timeSinceLastFrame * CAM_WASD_SPEED;
		}

		if (getMouseScrollY() < 0)
		{
			changeZoom(1.0f / 1.1f);
		}
		else if (getMouseScrollY() > 0)
		{
			changeZoom(1.1f);
		}

		if (isMouseDown(bbe::MouseButton::LEFT) || isMouseDown(bbe::MouseButton::RIGHT))
		{
			if (mode == MODE_BRUSH)
			{
				bbe::GridIterator gi(getMouse(), getMousePrevious());
				while (gi.hasNext())
				{
					const bbe::Vector2 coordBase = screenToCanvas(gi.next().as<float>());
					for (int32_t i = -brushWidth; i <= brushWidth; i++)
					{
						for (int32_t k = -brushWidth; k <= brushWidth; k++)
						{
							float pencilStrength = bbe::Math::clamp01(brushWidth - bbe::Math::sqrt(i * i + k * k));
							if (pencilStrength <= 0.f) continue;

							bbe::Vector2 coord = coordBase + bbe::Vector2(i, k);
							if (toTiledPos(coord))
							{
								bbe::Colori newColor = getMouseColor();
								newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
								bbe::Colori oldWorkColor = workArea.getPixel(coord.x, coord.y);
								if (newColor.a > oldWorkColor.a)
								{
									workArea.setPixel(coord.x, coord.y, newColor);
								}
							}
						}
					}
				}
			}
			else if (mode == MODE_FLOOD_FILL)
			{
				bbe::Vector2 pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					canvas.floodFill(pos.as<int32_t>(), getMouseColor());
				}
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
		}

		if (isMouseReleased(bbe::MouseButton::LEFT) || isMouseReleased(bbe::MouseButton::RIGHT))
		{
			if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT))
			{
				applyWorkArea();
			}
		}

		if (isMouseDown(bbe::MouseButton::MIDDLE))
		{
			offset += getMouseDelta();
			if (tiled)
			{
				if (offset.x < 0) offset.x += canvas.getWidth() * zoomLevel;
				if (offset.y < 0) offset.y += canvas.getHeight() * zoomLevel;
				if (offset.x > canvas.getWidth() * zoomLevel) offset.x -= canvas.getWidth() * zoomLevel;
				if (offset.y > canvas.getHeight() * zoomLevel) offset.y -= canvas.getHeight() * zoomLevel;
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		ImGui::ColorEdit4("Left Color", leftColor);
		ImGui::ColorEdit4("Right Color", rightColor);
		ImGui::bbe::combo("Mode", { "Brush", "Flood fill" }, mode);
		if (mode == MODE_BRUSH)
		{
			if (ImGui::InputInt("Brush Width", &brushWidth))
			{
				if (brushWidth < 1) brushWidth = 1;
			}
		}
		if (ImGui::Button("Copy to Clipboard"))
		{
			canvas.copyToClipboard();
		}
		ImGui::Text(bbe::Image::isImageInClipbaord() ? "Yes" : "No");

		ImGui::BeginDisabled(!bbe::Image::isImageInClipbaord());
		if (ImGui::Button("Paste"))
		{
			canvas = bbe::Image::getClipboardImage();
			setupCanvas();
		}
		ImGui::EndDisabled();

		const int32_t repeats = tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				brush.drawImage(offset.x + i * canvas.getWidth() * zoomLevel, offset.y + k * canvas.getHeight() * zoomLevel, canvas.getWidth() * zoomLevel, canvas.getHeight() * zoomLevel, canvas);
				brush.drawImage(offset.x + i * canvas.getWidth() * zoomLevel, offset.y + k * canvas.getHeight() * zoomLevel, canvas.getWidth() * zoomLevel, canvas.getHeight() * zoomLevel, workArea);
			}
		}
		if (zoomLevel > 3 && drawGridLines)
		{
			bbe::Vector2 zeroPos = screenToCanvas({ 0, 0 });
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.5f);
			for (float i = -(zeroPos.x - (int)zeroPos.x) * zoomLevel; i < getWindowWidth(); i += zoomLevel)
			{
				brush.fillLine(i, 0, i, getWindowHeight());
			}
			for (float i = -(zeroPos.y - (int)zeroPos.y) * zoomLevel; i < getWindowHeight(); i += zoomLevel)
			{
				brush.fillLine(0, i, getWindowWidth(), i);
			}
		}

		// HACK: We can only open popups if we are in the same ID Stack. See: https://github.com/ocornut/imgui/issues/331
		bool openNewCanvas = false;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("New..."))
				{
					openNewCanvas = true;
				}
				if (ImGui::MenuItem("Open..."))
				{
					if (bbe::simpleFile::showOpenDialog(path))
					{
						newCanvas(path.getRaw());
					}
				}
				if (ImGui::MenuItem("Save"))
				{
					if (path.isEmpty())
					{
						bbe::simpleFile::showSaveDialog(path, "png");
					}
					if (!path.isEmpty())
					{
						canvas.writeToFile(path);
					}
				}
				if (ImGui::MenuItem("Save As..."))
				{
					if (bbe::simpleFile::showSaveDialog(path, "png"))
					{
						canvas.writeToFile(path);
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Draw Grid Lines", nullptr, drawGridLines))
				{
					drawGridLines = !drawGridLines;
				}
				if (ImGui::MenuItem("Tiled", nullptr, tiled))
				{
					tiled = !tiled;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		static int newWidth = 0;
		static int newHeight = 0;
		if (openNewCanvas)
		{
			ImGui::OpenPopup("New Canvas");
			newWidth = canvas.getWidth();
			newHeight = canvas.getHeight();
		}
		if (ImGui::BeginPopupModal("New Canvas", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputInt("Width", &newWidth);
			ImGui::SameLine();
			ImGui::InputInt("Height", &newHeight);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				newCanvas(newWidth, newHeight);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::ShowDemoWindow();
	}

	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "ExamplePaint");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}