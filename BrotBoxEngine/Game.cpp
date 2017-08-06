#include "stdafx.h"
#include "BBE/Game.h"
#include "BBE/Window.h"
#include "BBE/Exceptions.h"
#include "BBE/PrimitiveBrush2D.h"

bbe::Game::Game()
{
	//do nothing
}

bbe::Game::~Game()
{
	if (m_window != nullptr)
	{
		delete m_window;
	}
}

void bbe::Game::start(int windowWidth, int windowHeight, const char* title)
{
	if (m_started)
	{
		throw AlreadyCreatedException();
	}

	m_window = new Window(windowWidth, windowHeight, title);

	m_gameTime.reset();
	onStart();

	while (m_window->keepAlive())
	{
		m_window->INTERNAL_keyboard.update();
		update(m_gameTime.tick());

		m_window->preDraw();
		PrimitiveBrush2D brush2D = *(m_window->getBrush2D());
		draw2D(brush2D);
		m_window->postDraw();
	}

	onEnd();
}

bool bbe::Game::isKeyDown(int keyCode)
{
	return m_window->INTERNAL_keyboard.isKeyDown(keyCode);
}

bool bbe::Game::isKeyUp(int keyCode)
{
	return m_window->INTERNAL_keyboard.isKeyUp(keyCode);
}

bool bbe::Game::wasKeyDownLastFrame(int keyCode)
{
	return m_window->INTERNAL_keyboard.wasKeyDownLastFrame(keyCode);
}

bool bbe::Game::wasKeyUpLastFrame(int keyCode)
{
	return m_window->INTERNAL_keyboard.wasKeyUpLastFrame(keyCode);
}

bool bbe::Game::isKeyPressed(int keyCode)
{
	return m_window->INTERNAL_keyboard.isKeyPressed(keyCode);
}
