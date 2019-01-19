#include "BBE/WindowTest.h"
#include "BBE/KeyboardKeys.h"
#include <iostream>

void bbe::test::MyGame::onStart()
{
	rect.set(200.0f, 200.f, 100.f, 100.f);
	std::cout << "Start!" << std::endl;
}

void bbe::test::MyGame::update(float timeSinceLastFrame)
{
	std::cout << "Update! " << timeSinceLastFrame << std::endl;
	if (isKeyDown(Key::A))
	{
		std::cout << "A is down! :-)" << std::endl;
	}
}

void bbe::test::MyGame::draw2D(PrimitiveBrush2D &brush)
{
	brush.setColor(1.0f, 0.5f, 1.0f);
	brush.fillRect(rect);
	brush.fillRect(0.0f, 0.0f, 100.f, 50.f);

	brush.setColor(1.0f, 1.0f, 0.0f);
	brush.fillRect(200.f, 100.f, 100.f, 100.f);
	std::cout << "Draw! " << std::endl;
}

void bbe::test::MyGame::draw3D(PrimitiveBrush3D &brush)
{
}

void bbe::test::MyGame::onEnd()
{
	std::cout << "End!" << std::endl;
}

void bbe::test::testWindow()
{
	MyGame mg;
	mg.start(400, 300, "Test");
}
