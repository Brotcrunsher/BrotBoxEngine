#include "BBE/Game.h"
#include "BBE/Window.h"
#include "BBE/Exceptions.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Math.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void bbe::Game::mainLoop()
{
	m_frameNumber++;
	frame();

	if (screenshotRenderingPath)
	{
		screenshot((bbe::String(screenshotRenderingPath) + m_frameNumber + ".png").getRaw());
	}
}

static void staticMainLoop(void* gamePtr)
{
	((bbe::Game*)gamePtr)->mainLoop();
}

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
	m_started = true;

	std::cout << "Starting math" << std::endl;
	bbe::Math::INTERNAL::startMath();

	std::cout << "Creating window" << std::endl;
	m_pwindow = new Window(windowWidth, windowHeight, title);

	std::cout << "Reseting game time" << std::endl;
	m_gameTime.reset();

#ifndef BBE_NO_AUDIO
	std::cout << "Initializing SoundManager" << std::endl;
	m_soundManager.init();
#endif

	std::cout << "Calling onStart()" << std::endl;
	onStart();

	if (videoRenderingPath)
	{
		m_pwindow->setVideoRenderingMode(videoRenderingPath);
	}

	if (!isExternallyManaged())
	{
#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop_arg(staticMainLoop, this, 0, true);
#else
		while (keepAlive() && (m_maxFrameNumber == 0 || m_frameNumber < m_maxFrameNumber))
		{
			mainLoop();
		}
#endif

		shutdown();
	}
}

bool bbe::Game::keepAlive()
{
#ifdef BBE_RENDERER_NULL
	// The null renderer keeps games alive for 128 frames and then closes them.
	static int callCount = 0;
	if (callCount >= 128)
	{
		return false;
	}
	callCount++;
#endif
	return m_pwindow->keepAlive();
}

void bbe::Game::frame()
{
	frameUpdate();
	frameDraw();
}

void bbe::Game::frameUpdate()
{
	m_pwindow->executeFrameStartListeneres();
	m_pwindow->INTERNAL_keyboard.update();
	const bbe::Vector2 globalMousePos = m_pwindow->getGlobalMousePos();
	m_pwindow->INTERNAL_mouse.update(globalMousePos.x, globalMousePos.y);
	float timeSinceLastFrame = m_gameTime.tick();
	if (m_fixedFrameTime != 0.f) timeSinceLastFrame = m_fixedFrameTime;
	
	if (m_frameNumber < 100) m_frameTimeRunningAverage = timeSinceLastFrame;
	else m_frameTimeRunningAverage = 0.99 * m_frameTimeRunningAverage + 0.01 * timeSinceLastFrame;

	m_physWorld.update(timeSinceLastFrame);
#ifndef BBE_NO_AUDIO
	m_soundManager.update();
#endif
	update(timeSinceLastFrame);
}

void bbe::Game::frameDraw()
{
	if (!m_pwindow->isReadyToDraw())
	{
		return;
	}

	m_pwindow->preDraw();
	m_pwindow->preDraw3D();
	draw3D(m_pwindow->getBrush3D());
	m_pwindow->preDraw2D();
	draw2D(m_pwindow->getBrush2D());
	m_pwindow->postDraw();
	m_pwindow->waitEndDraw();
}

void bbe::Game::shutdown()
{
	m_pwindow->waitTillIdle();

	onEnd();

	m_pwindow->executeCloseListeners();

#ifndef BBE_NO_AUDIO
	m_soundManager.destroy();
#endif
}

void bbe::Game::setExternallyManaged(bool managed)
{
	if (m_started)
	{
		// Managed must be set before calling start!
		throw AlreadyCreatedException();
	}
	m_externallyManaged = managed;
}

bool bbe::Game::isExternallyManaged() const
{
	return m_externallyManaged;
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

float bbe::Game::getMouseX() const
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseX());
}

float bbe::Game::getMouseY() const
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseY());
}

bbe::Vector2 bbe::Game::getMouse() const
{
	return Vector2(getMouseX(), getMouseY());
}

float bbe::Game::getMouseXGlobal() const
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseXGlobal());
}

float bbe::Game::getMouseYGlobal() const
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseYGlobal());
}

bbe::Vector2 bbe::Game::getMouseGlobal() const
{
	return Vector2(getMouseXGlobal(), getMouseYGlobal());
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

int bbe::Game::getScaledWindowWidth()
{
	return m_pwindow->getScaledWidth();
}

int bbe::Game::getWindowHeight()
{
	return m_pwindow->getHeight();
}

int bbe::Game::getScaledWindowHeight()
{
	return m_pwindow->getScaledHeight();
}

uint64_t bbe::Game::getAmountOfFrames()
{
	return m_gameTime.getAmountOfTicks();
}

float bbe::Game::getAverageFrameTime()
{
	return m_frameTimeRunningAverage;
}

void bbe::Game::setCursorMode(bbe::CursorMode cm)
{
	m_pwindow->setCursorMode(cm);
}

bbe::PhysWorld* bbe::Game::getPhysWorld()
{
	return &m_physWorld;
}

void bbe::Game::screenshot(const bbe::String &path)
{
	m_pwindow->screenshot(path);
}

void bbe::Game::setVideoRenderingMode(const char* path)
{
	if (m_started)
	{
		// Video Rendering must be enabled before start()!
		throw IllegalStateException();
	}
	videoRenderingPath = path;
	setFixedFrametime(1.f / 60.f);
}

void bbe::Game::setScreenshotRecordingMode(const char* path)
{
	// If you want to make a movie out of these screenshots,
	// you can use ffmpeg with the following command:
	// 
	// ffmpeg -framerate 60 -f image2 -i 'img%d.png' out.mp4
	if (m_started)
	{
		// Screenshot Recording must be enabled before start()!
		throw IllegalStateException();
	}
	screenshotRenderingPath = path;
	setFixedFrametime(1.f / 60.f);
}

void bbe::Game::setMaxFrame(uint64_t maxFrame)
{
	m_maxFrameNumber = maxFrame;
}

void bbe::Game::setFixedFrametime(float time)
{
	m_fixedFrameTime = time;
}

bbe::String bbe::Game::getClipboard() const
{
	return bbe::String(glfwWrapper::glfwGetClipboardString(m_pwindow->m_pwindow));
}

void bbe::Game::setClipboard(const bbe::String& string)
{
	glfwWrapper::glfwSetClipboardString(m_pwindow->m_pwindow, string.getRaw());
}

