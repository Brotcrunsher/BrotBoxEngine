#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::Vector2 projectionsPoint = { 4, 1 };
	bbe::RectangleRotated rr;
	bbe::RectangleRotated rrMouse;
	static constexpr float rectangleRotationSpeed = 1;

	virtual void onStart() override
	{
		rr = bbe::RectangleRotated(100, 200, 100, 140, 0);
		rrMouse = bbe::RectangleRotated(400, 400, 40, 20, 0);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		rr.setRotation(rr.getRotation() + timeSinceLastFrame * rectangleRotationSpeed);
		rrMouse.setRotation(rrMouse.getRotation() + timeSinceLastFrame * 0.4f * rectangleRotationSpeed);
		rrMouse.setX(getMouseX());
		rrMouse.setY(getMouseY());
		if (isMouseDown(bbe::MouseButton::LEFT))
		{
			projectionsPoint = getMouse();
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(rr);

		auto vertices = rr.getVertices();
		brush.setColorRGB(1, 0, 0);
		brush.fillLineStrip(vertices, true);

		const bbe::RectangleRotated::ProjectionResult pr = rr.project(projectionsPoint);
		brush.setColorRGB(0, 1, 0);
		brush.fillLine(pr.start, pr.stop);

		auto projections = bbe::Math::project(vertices, projectionsPoint);
		assert(projections.getLength() == vertices.getLength());
		for (size_t i = 0; i < projections.getLength(); i++)
		{
			brush.fillLine(vertices[i], projections[i]);
		}

		auto normals = rr.getNormals();
		brush.setColorRGB(0, 0, 1);
		for (size_t i = 0; i < normals.getLength(); i++)
		{
			brush.fillLine(vertices[i], normals[i] * 10 + vertices[i]);
		}

		if (rrMouse.intersects(rr))
		{
			brush.setColorRGB(0, 1, 1);
		}
		else
		{
			brush.setColorRGB(1, 1, 0);
		}
		brush.fillRect(rrMouse);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame* mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Rotating Rectangles!");

    return 0;
}
