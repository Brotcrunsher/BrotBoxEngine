#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <Windows.h>

// TODO: Proper GUI
// TODO: Layers
// TODO: Text
// TODO: Select+Move Tool
// TODO: Drag and Drop image files into paint
// TODO: Circle tool
// TODO: Flood fill with edges of brush tool kinda bad.
// TODO: Bug: right click has weird behaviour with shadow

class MyGame : public bbe::Game
{
	bbe::Vector2 offset;
	bbe::String path;
	bbe::UndoableObject<bbe::Image> canvas;
	bbe::Image workArea;
	float zoomLevel = 1.f;

	float leftColor[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr static int32_t MODE_BRUSH      = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	constexpr static int32_t MODE_LINE       = 2;
	constexpr static int32_t MODE_RECTANGLE  = 3;
	constexpr static int32_t MODE_PIPETTE    = 4;
	int32_t mode = MODE_BRUSH;

	int32_t brushWidth = 1;
	bbe::Vector2 startMousePos;
	bool roundEdges = false;

	bool drawGridLines = true;
	bool tiled = false;

	void resetCamera()
	{
		offset = bbe::Vector2(getWindowWidth() / 2 - canvas.get().getWidth() / 2, getWindowHeight() / 2 - canvas.get().getHeight() / 2);
		zoomLevel = 1.f;
	}

	void clearWorkArea()
	{
		workArea = bbe::Image(canvas.get().getWidth(), canvas.get().getHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
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

				const bbe::Colori oldColor = canvas.get().getPixel(i, k);
				const bbe::Colori newColor = oldColor.blendTo(c);
				canvas.get().setPixel(i, k, newColor);
			}
		}
		clearWorkArea();
	}

	void setupCanvas()
	{
		canvas.get().keepAfterUpload();
		canvas.get().setFilterMode(bbe::ImageFilterMode::NEAREST);
		clearWorkArea();
		resetCamera();
		canvas.clearHistory();
	}

	void newCanvas(uint32_t width, uint32_t height)
	{
		canvas.get() = bbe::Image(width, height, bbe::Color::white());
		this->path = "";
		setupCanvas();
	}

	void newCanvas(const char* path)
	{
		canvas.get() = bbe::Image(path);
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
			pos.x = bbe::Math::mod<float>(pos.x, canvas.get().getWidth());
			pos.y = bbe::Math::mod<float>(pos.y, canvas.get().getHeight());
			return true; // If we are tiled, then any position is always within the canvas.
		}

		// If we are not tiled, then we have to check if the pos is actually part of the canvas.
		return pos.x >= 0 && pos.y >= 0 && pos.x < canvas.get().getWidth() && pos.y < canvas.get().getHeight();
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
		if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT)) return bbe::Color(leftColor).asByteColor();
		return isMouseDown(bbe::MouseButton::LEFT) ? bbe::Color(leftColor).asByteColor() : bbe::Color(rightColor).asByteColor();
	}

	bool touch(const bbe::Vector2& touchPos, bool rectangleShape = false)
	{
		bool changeRegistered = false;
		for (int32_t i = -brushWidth; i <= brushWidth; i++)
		{
			for (int32_t k = -brushWidth; k <= brushWidth; k++)
			{
				float pencilStrength = bbe::Math::clamp01(brushWidth - bbe::Math::sqrt(i * i + k * k));
				if (rectangleShape)
				{
					if (i != -brushWidth && i != brushWidth && k != -brushWidth && k != brushWidth)
					{
						pencilStrength = 1.0f;
					}
					else
					{
						pencilStrength = 0.0f;
					}
				}
				if (pencilStrength <= 0.f) continue;

				bbe::Vector2 coord = touchPos + bbe::Vector2(i, k);
				if (toTiledPos(coord))
				{
					bbe::Colori newColor = getMouseColor();
					newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
					bbe::Colori oldWorkColor = workArea.getPixel(coord.x, coord.y);
					if (newColor.a > oldWorkColor.a)
					{
						workArea.setPixel(coord.x, coord.y, newColor);
						changeRegistered = true;
					}
				}
			}
		}
		return changeRegistered;
	}

	bool touchLine(const bbe::Vector2& pos1, const bbe::Vector2& pos2, bool rectangleShape = false)
	{
		bool changeRegistered = false;
		bbe::GridIterator gi(pos1, pos2);
		while (gi.hasNext())
		{
			const bbe::Vector2 coordBase = gi.next().as<float>();
			changeRegistered |= touch(coordBase, rectangleShape);
		}
		return changeRegistered;
	}

	virtual void onStart() override
	{
		newCanvas(400, 300);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		const bbe::Vector2 prevMousePos = screenToCanvas(getMousePrevious());
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

		if (isMouseDown(bbe::MouseButton::MIDDLE))
		{
			offset += getMouseDelta();
			if (tiled)
			{
				if (offset.x < 0) offset.x += canvas.get().getWidth() * zoomLevel;
				if (offset.y < 0) offset.y += canvas.get().getHeight() * zoomLevel;
				if (offset.x > canvas.get().getWidth() * zoomLevel) offset.x -= canvas.get().getWidth() * zoomLevel;
				if (offset.y > canvas.get().getHeight() * zoomLevel) offset.y -= canvas.get().getHeight() * zoomLevel;
			}
		}

		if (getMouseScrollY() < 0)
		{
			changeZoom(1.0f / 1.1f);
		}
		else if (getMouseScrollY() > 0)
		{
			changeZoom(1.1f);
		}
		const bbe::Vector2 currMousePos = screenToCanvas(getMouse());

		if (isKeyDown(bbe::Key::LEFT_CONTROL))
		{
			if (isKeyTyped(bbe::Key::Z) && canvas.isUndoable()) canvas.undo();
			if (isKeyTyped(bbe::Key::Y) && canvas.isRedoable()) canvas.redo();
		}

		static bool changeRegistered = false;
		if (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT))
		{
			startMousePos = screenToCanvas(getMouse());
		}

		// TODO: Would be nice if we had a constexpr list
		const bbe::List<decltype(mode)> shadowDrawModes = { MODE_BRUSH };
		const bool drawMode = isMouseDown(bbe::MouseButton::LEFT) || isMouseDown(bbe::MouseButton::RIGHT);

		if (drawMode || shadowDrawModes.contains(mode))
		{
			// This counter is a bit of a hack. Problem is if we wouldn't have it and immediately call clearWorkArea then the workArea would never get applied.
			// Applying the work area could be moved before this if, but then we might lose a frame of "work" which might feel odd.
			static uint32_t counter = 0;
			if (!drawMode)
			{
				counter++;
				if(counter > 1) clearWorkArea();
			}
			else
			{
				counter = 0;
			}

			if (mode == MODE_BRUSH)
			{
				changeRegistered |= touchLine(currMousePos, prevMousePos);
			}
			else if (mode == MODE_FLOOD_FILL)
			{
				bbe::Vector2 pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					canvas.get().floodFill(pos.as<int32_t>(), getMouseColor(), false, tiled);
					changeRegistered = true;
				}
			}
			else if (mode == MODE_LINE)
			{
				clearWorkArea();
				changeRegistered |= touchLine(currMousePos, startMousePos);
			}
			else if (mode == MODE_RECTANGLE)
			{
				clearWorkArea();
				const bbe::Vector2 topLeft = startMousePos;
				const bbe::Vector2 topRight = bbe::Vector2(currMousePos.x, startMousePos.y);
				const bbe::Vector2 bottomLeft = bbe::Vector2(startMousePos.x, currMousePos.y);
				const bbe::Vector2 bottomRight = currMousePos;
				changeRegistered |= touchLine(topLeft, topRight, !roundEdges);
				changeRegistered |= touchLine(topRight, bottomRight, !roundEdges);
				changeRegistered |= touchLine(bottomRight, bottomLeft, !roundEdges);
				changeRegistered |= touchLine(bottomLeft, topLeft, !roundEdges);
			}
			else if (mode == MODE_PIPETTE)
			{
				auto pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					const size_t x = (size_t)pos.x;
					const size_t y = (size_t)pos.y;
					const bbe::Colori color = canvas.get().getPixel(x, y);
					if (isMouseDown(bbe::MouseButton::LEFT))
					{
						leftColor[0] = color.r / 255.f;
						leftColor[1] = color.g / 255.f;
						leftColor[2] = color.b / 255.f;
						leftColor[3] = color.a / 255.f;
					}
					if (isMouseDown(bbe::MouseButton::RIGHT))
					{
						rightColor[0] = color.r / 255.f;
						rightColor[1] = color.g / 255.f;
						rightColor[2] = color.b / 255.f;
						rightColor[3] = color.a / 255.f;
					}
				}
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
		}

		if (changeRegistered)
		{
			if (isMouseReleased(bbe::MouseButton::LEFT) || isMouseReleased(bbe::MouseButton::RIGHT))
			{
				if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT))
				{
					applyWorkArea();
					canvas.submit(); // <- changeRegistered is for this
					changeRegistered = false;
				}
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		ImGui::BeginDisabled(!canvas.isUndoable());
		if (ImGui::Button("Undo"))
		{
			canvas.undo();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!canvas.isRedoable());
		if (ImGui::Button("Redo"))
		{
			canvas.redo();
		}
		ImGui::EndDisabled();


		ImGui::ColorEdit4("Left Color", leftColor);
		ImGui::ColorEdit4("Right Color", rightColor);
		ImGui::bbe::combo("Mode", { "Brush", "Flood fill", "Line" , "Rectangle", "Pipette"}, mode);
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE)
		{
			if (ImGui::InputInt("Brush Width", &brushWidth))
			{
				if (brushWidth < 1) brushWidth = 1;
			}
		}
		if (mode == MODE_RECTANGLE)
		{
			ImGui::Checkbox("Round Edges", &roundEdges);
		}
		if (ImGui::Button("Copy to Clipboard"))
		{
			canvas.get().copyToClipboard();
		}
		ImGui::Text(bbe::Image::isImageInClipbaord() ? "Yes" : "No");

		ImGui::BeginDisabled(!bbe::Image::isImageInClipbaord());
		if (ImGui::Button("Paste"))
		{
			canvas.get() = bbe::Image::getClipboardImage();
			setupCanvas();
		}
		ImGui::EndDisabled();

		const int32_t repeats = tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				brush.drawImage(offset.x + i * canvas.get().getWidth() * zoomLevel, offset.y + k * canvas.get().getHeight() * zoomLevel, canvas.get().getWidth() * zoomLevel, canvas.get().getHeight() * zoomLevel, canvas.get());
				brush.drawImage(offset.x + i * canvas.get().getWidth() * zoomLevel, offset.y + k * canvas.get().getHeight() * zoomLevel, canvas.get().getWidth() * zoomLevel, canvas.get().getHeight() * zoomLevel, workArea);
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
						canvas.get().writeToFile(path);
					}
				}
				if (ImGui::MenuItem("Save As..."))
				{
					if (bbe::simpleFile::showSaveDialog(path, "png"))
					{
						canvas.get().writeToFile(path);
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
			newWidth = canvas.get().getWidth();
			newHeight = canvas.get().getHeight();
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