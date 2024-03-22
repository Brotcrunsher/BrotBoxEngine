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

class MyGame : public bbe::Game
{
	bbe::Vector2 offset;
	bbe::String path;
	bbe::Image canvas;
	float zoomLevel = 1.f;

	float leftColor[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr static int32_t MODE_BRUSH      = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	int32_t mode = MODE_BRUSH;

	bool drawGridLines = true;
	bool tiled = false;

	void resetCamera()
	{
		offset = bbe::Vector2(getWindowWidth() / 2 - canvas.getWidth() / 2, getWindowHeight() / 2 - canvas.getHeight() / 2);
		zoomLevel = 1.f;
	}

	void setupCanvas()
	{
		canvas.keepAfterUpload();
		canvas.setFilterMode(bbe::ImageFilterMode::NEAREST);
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
					bbe::Vector2 coord = screenToCanvas(gi.next().as<float>());
					if (tiled)
					{
						coord.x = bbe::Math::mod<float>(coord.x, canvas.getWidth());
						coord.y = bbe::Math::mod<float>(coord.y, canvas.getHeight());
					}
					if (coord.x >= 0 && coord.y >= 0 && coord.x < canvas.getWidth() && coord.y < canvas.getHeight())
					{
						canvas.setPixel(coord.x, coord.y, getMouseColor());
					}
				}
			}
			else if (mode == MODE_FLOOD_FILL)
			{
				canvas.floodFill(screenToCanvas(getMouse()).as<int32_t>(), getMouseColor());
			}
			else
			{
				throw bbe::IllegalStateException();
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