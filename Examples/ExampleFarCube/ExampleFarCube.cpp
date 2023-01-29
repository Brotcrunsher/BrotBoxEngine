#include "BBE/BrotBoxEngine.h"
#include <iostream>

// Debugging Example!
// A cube, a camera, and a light are placed into the Scene. All three of them are moving with a very high speed in some direction.
// However, relatively they remain at the same location. This way, the rendered image should always look the same. If it doesn't, then
// the Renderer did some mess up with high coordinate values.
// NOTE: As the position offset is growing exponentially, it quickly gets impossible for the vertex shader to keep up. That is an
//       acceptable limitation of the BBE. However, color flickering must not happen.
// Current eye-measurements:
// Renderer Breaks after...
//   Vulkan Vertex: 19.000 Meter
//   Vulkan Fragment: Never?
//   OpenGL Vertex: 100.000 Meter? Hard to tell due to all the flickering. Probably earlier.
//   OpenGL Fragment: 22 Meter (<- BAAAAD!)
class MyGame : public bbe::Game
{
	float timePassed = 0;
	bbe::Vector3 currentPos;
	bool running = true;
	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyDown(bbe::Key::SPACE))
		{
			running = false;
		}
		if(running) timePassed += timeSinceLastFrame;
		currentPos.x = bbe::Math::pow(1.5, timePassed);
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		bbe::Vector3 lightOffset(1, 0.5, 3);
		bbe::Vector3 cameraOffset(1, 1, 1);

		bbe::Cube cube(currentPos);


		brush.addLight(currentPos + lightOffset, 5, bbe::Color(1, 1, 1, 1), bbe::Color(1, 1, 1, 1), bbe::LIGHT_FALLOFF_SQUARED);
		brush.setCamera(currentPos + cameraOffset, currentPos);
		brush.fillCube(cube);
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		bbe::String text = "We are now out ";
		text += currentPos.x;
		text += " meters.";
		brush.fillText(20, 20, text);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "Template!");

    return 0;
}

