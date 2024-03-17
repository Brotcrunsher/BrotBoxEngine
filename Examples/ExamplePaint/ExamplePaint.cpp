#include "BBE/BrotBoxEngine.h"
#include <iostream>

// TODO: Proper GUI
// TODO: Save
// TODO: Load
// TODO: View menu that allows you to tile draw or hide the grid when zoomed in
// TODO: "New..."
// TODO: Layers
// TODO: Text
// TODO: Copy from Paint to clipboard
// TODO: Paste from Clipboard to Paint

class MyGame : public bbe::Game
{
	bbe::Vector2 offset;
	bbe::Image canvas;
	float zoomLevel = 1.f;

	float leftColor[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr static int32_t MODE_BRUSH      = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	int32_t mode = MODE_BRUSH;

	void resetCamera()
	{
		offset = bbe::Vector2(getWindowWidth() / 2 - canvas.getWidth() / 2, getWindowHeight() / 2 - canvas.getHeight() / 2);
		zoomLevel = 1.f;
	}

	void newCanvas(uint32_t width, uint32_t height)
	{
		canvas = bbe::Image(width, height, bbe::Color::white());
		canvas.keepAfterUpload();
		canvas.setFilterMode(bbe::ImageFilterMode::NEAREST);
		resetCamera();
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

	bbe::Color getMouseColor() const
	{
		return isMouseDown(bbe::MouseButton::LEFT) ? bbe::Color(leftColor) : bbe::Color(rightColor);
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
					coord.x = bbe::Math::mod<float>(coord.x, canvas.getWidth());
					coord.y = bbe::Math::mod<float>(coord.y, canvas.getHeight());
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

		constexpr int32_t repeats = 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				brush.drawImage(offset.x + i * canvas.getWidth(), offset.y + k * canvas.getHeight(), canvas.getWidth() * zoomLevel, canvas.getHeight() * zoomLevel, canvas);
			}
		}
		if (zoomLevel > 3)
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