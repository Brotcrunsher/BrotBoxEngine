#include "BBE/Game.h"
#include "BBE/Window.h"
#include "BBE/Exceptions.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Math.h"
#include "BBE/StopWatch.h"
#include "BBE/Profiler.h"
#include <iostream>

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
	std::cout << "Starting Game: " << title << std::endl;
	if (m_started)
	{
		throw AlreadyCreatedException();
	}

	std::cout << "Starting math" << std::endl;
	bbe::Math::INTERNAL::startMath();

	std::cout << "Creating window" << std::endl;
	m_pwindow = new Window(windowWidth, windowHeight, title);

	std::cout << "Reseting game time" << std::endl;
	m_gameTime.reset();

	std::cout << "Calling onStart()" << std::endl;
	onStart();

	while (m_pwindow->keepAlive())
	{
		StopWatch sw;
		m_pwindow->INTERNAL_keyboard.update();
		m_pwindow->INTERNAL_mouse.update();
		const float timeSinceLastFrame = m_gameTime.tick();
		m_physWorld.update(timeSinceLastFrame);
		update(timeSinceLastFrame);

		m_pwindow->preDraw();
		m_pwindow->preDraw3D();
		draw3D(m_pwindow->getBrush3D());
		m_pwindow->preDraw2D();
		draw2D(m_pwindow->getBrush2D());
		m_pwindow->postDraw();
		bbe::Profiler::INTERNAL::setCPUTime(sw.getTimeExpiredNanoseconds() / 1000.f / 1000.f / 1000.f);
		m_pwindow->waitEndDraw();
	}

	m_pwindow->waitTillIdle();

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

bool bbe::Game::isMouseDown(bbe::MouseButton button)
{
	return m_pwindow->INTERNAL_mouse.isButtonDown(button);
}

bool bbe::Game::isMouseUp(bbe::MouseButton button)
{
	return m_pwindow->INTERNAL_mouse.isButtonUp(button);
}

bool bbe::Game::wasMouseDownLastFrame(bbe::MouseButton button)
{
	return m_pwindow->INTERNAL_mouse.wasButtonDownLastFrame(button);
}

bool bbe::Game::wasMouseUpLastFrame(bbe::MouseButton button)
{
	return m_pwindow->INTERNAL_mouse.wasButtonUpLastFrame(button);
}

bool bbe::Game::isMousePressed(bbe::MouseButton button)
{
	return m_pwindow->INTERNAL_mouse.isButtonPressed(button);
}

float bbe::Game::getMouseX()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseX());
}

float bbe::Game::getMouseY()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseY());
}

bbe::Vector2 bbe::Game::getMouse()
{
	return Vector2(getMouseX(), getMouseY());
}

float bbe::Game::getMouseXDelta()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseXDelta());
}

float bbe::Game::getMouseYDelta()
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseYDelta());
}

bbe::Vector2 bbe::Game::getMouseDelta()
{
	return Vector2(getMouseXDelta(), getMouseYDelta());
}

float bbe::Game::getMouseScrollX()
{
	return (float)(m_pwindow->INTERNAL_mouse.getScrollX());
}

float bbe::Game::getMouseScrollY()
{
	return (float)(m_pwindow->INTERNAL_mouse.getScrollY());
}

bbe::Vector2 bbe::Game::getMouseScroll()
{
	return Vector2(getMouseScrollX(), getMouseScrollY());
}

float bbe::Game::getTimeSinceStartSeconds()
{
	return m_gameTime.timeSinceStartSeconds();
}

float bbe::Game::getTimeSinceStartMilliseconds()
{
	return m_gameTime.timeSinceStartMilliseconds();
}

int bbe::Game::getWindowWidth()
{
	return m_pwindow->getWidth();
}

int bbe::Game::getWindowHeight()
{
	return m_pwindow->getHeight();
}

uint64_t bbe::Game::getAmountOfFrames()
{
	return m_gameTime.getAmountOfTicks();
}

void bbe::Game::setCursorMode(bbe::CursorMode cm)
{
	m_pwindow->setCursorMode(cm);
}

bbe::PhysWorld* bbe::Game::getPhysWorld()
{
	return &m_physWorld;
}
