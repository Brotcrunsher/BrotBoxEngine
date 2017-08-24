#include "stdafx.h"
#include "BBE/Game.h"
#include "BBE/Window.h"
#include "BBE/Exceptions.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"

bbe::Game::Game()
{
	//do nothing
}

bbe::Game::~Game()
{
	if (m_pwindow != nullptr)
	{
		delete m_pwindow;
	}
}

void bbe::Game::start(int windowWidth, int windowHeight, const char* title)
{
	if (m_started)
	{
		throw AlreadyCreatedException();
	}

	m_pwindow = new Window(windowWidth, windowHeight, title);

	m_gameTime.reset();
	onStart();

	while (m_pwindow->keepAlive())
	{
		m_pwindow->INTERNAL_keyboard.update();
		m_pwindow->INTERNAL_mouse.update();
		update(m_gameTime.tick());

		m_pwindow->preDraw();
		PrimitiveBrush3D brush3D = *(m_pwindow->getBrush3D());
		m_pwindow->preDraw3D();
		draw3D(brush3D);
		PrimitiveBrush2D brush2D = *(m_pwindow->getBrush2D());
		m_pwindow->preDraw2D();
		draw2D(brush2D);
		m_pwindow->postDraw();
	}

	onEnd();
}

bool bbe::Game::isKeyDown(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.isKeyDown(key);
}

bool bbe::Game::isKeyUp(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.isKeyUp(key);
}

bool bbe::Game::wasKeyDownLastFrame(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.wasKeyDownLastFrame(key);
}

bool bbe::Game::wasKeyUpLastFrame(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.wasKeyUpLastFrame(key);
}

bool bbe::Game::isKeyPressed(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.isKeyPressed(key);
}

float bbe::Game::getMouseX()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseX());
}

float bbe::Game::getMouseY()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseY());
}

float bbe::Game::getMouseXDelta()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseXDelta());
}

float bbe::Game::getMouseYDelta()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseYDelta());
}

void bbe::Game::setCursorMode(bbe::CursorMode cm)
{
	m_pwindow->setCursorMode(cm);
}
