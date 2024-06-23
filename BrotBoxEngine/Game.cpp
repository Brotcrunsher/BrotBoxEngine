#include "BBE/Game.h"
#include "BBE/Window.h"
#include "BBE/Exceptions.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Math.h"
#include "BBE/StopWatch.h"
#include "BBE/SimpleFile.h"
#include <iostream>
#include "implot.h"
#include "BBE/ImGuiExtensions.h"
#include "BBE/Logging.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <signal.h>
#if __has_include(<stacktrace>)
#include <stacktrace>
#else
#pragma warning("Stacktrace lib is not present!")
#endif

void bbe::Game::mainLoop()
{
	m_frameNumber++;
	frame(false);

	if (screenshotRenderingPath)
	{
		screenshot((bbe::String(screenshotRenderingPath) + m_frameNumber + ".png").getRaw());
	}
}

#ifndef BBE_NO_AUDIO
void bbe::Game::setSoundListener(const bbe::Vector3& pos, const bbe::Vector3& lookDirection)
{
	m_soundManager.setSoundListener(pos, lookDirection);
}
#endif

#ifndef BBE_NO_AUDIO
void bbe::Game::restartSoundSystem()
{
	m_soundManager.restart();
}
#endif

#ifdef BBE_RENDERER_OPENGL
uint32_t bbe::Game::getAmountOfDrawcalls() const
{
	return m_pwindow->getAmountOfDrawcalls();
}
#endif

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

static void crashHandler(int sig)
{
	const bbe::String time = bbe::TimePoint().toString();

	bbe::String string;
	string += "###################\n";
	string += "#                 #\n";
	string += "#   !!!CRASH!!!   #\n";
	string += "#                 #\n";
	string += "###################\n";
	string += "\n";
	string += "Time:   " + time;
	string += "Signal: " + bbe::String(sig);
	string += "\n";
	string += "Stacktrace:\n";
#if __has_include(<stacktrace>)
	string += std::to_string(std::stacktrace::current());
#else
	string += "Stacktrace lib is not present!";
#endif
	BBELOGLN(string);

	bbe::simpleFile::createDirectory("CrashLogs");
	bbe::simpleFile::writeStringToFile("CrashLogs/" + bbe::String(std::time(nullptr)) + ".txt", string);
}

void bbe::Game::start(int windowWidth, int windowHeight, const char* title)
{
	signal(SIGSEGV, crashHandler);

	BBELOGLN("Starting Game: " << title);
	if (m_started)
	{
		throw AlreadyCreatedException();
	}
	m_started = true;

	BBELOGLN("Creating window");
	m_pwindow = new Window(windowWidth, windowHeight, title, this);

	BBELOGLN("Reseting game time");
	m_gameTime.reset();

#ifndef BBE_NO_AUDIO
	BBELOGLN("Initializing SoundManager");
	m_soundManager.init();
#endif

	BBELOGLN("Calling onStart()");
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
		while((m_maxFrameNumber == 0 || m_frameNumber < m_maxFrameNumber))
		{
			beginMeasure("INTERNAL - Keep Alive");
			bool kA = keepAlive();
			endMeasure();
			if (!kA) break;
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

void bbe::Game::frame(bool dragging)
{
	StopWatch sw;
	frameUpdate();
	frameDraw(dragging);
	if (m_targetFrameTime > 0)
	{
		std::this_thread::sleep_for(std::chrono::microseconds((int32_t)(m_targetFrameTime * 1000000.f) - sw.getTimeExpiredMicroseconds()));
	}
	beginMeasure("INTERNAL - Overhead (Between Frames)");
}

void bbe::Game::frameUpdate()
{
	beginMeasure("INTERNAL - Frame Start");
	ImGui::bbe::INTERNAL::setActiveGame(this);
	m_pwindow->update();
	float timeSinceLastFrame = m_gameTime.tick();
	if (m_fixedFrameTime != 0.f) timeSinceLastFrame = m_fixedFrameTime;
	
	if (m_frameNumber < 100) m_frameTimeRunningAverage = timeSinceLastFrame;
	else m_frameTimeRunningAverage = 0.99 * m_frameTimeRunningAverage + 0.01 * timeSinceLastFrame;

	if (m_frameNumber > 100)
	{
		m_frameTimeHistory[m_frameTimeHistoryWritePointer] = timeSinceLastFrame;
		m_frameTimeHistoryWritePointer++;
		if (m_frameTimeHistoryWritePointer >= m_frameTimeHistory.getLength()) m_frameTimeHistoryWritePointer = 0;
	}

	m_physWorld.update(timeSinceLastFrame);
#ifndef BBE_NO_AUDIO
	m_soundManager.update();
#endif
	update(timeSinceLastFrame);
	endMeasure();

	if (nextMinuteMaxMove.hasPassed())
	{
		nextMinuteMaxMove = bbe::TimePoint().plusMinutes(1);
		for (auto it = m_performanceMeasurements.begin(); it != m_performanceMeasurements.end(); it++)
		{
			PerformanceMeasurement& pm = it->second;
			pm.minuteMax2 = pm.minuteMax1;
			pm.minuteMax1 = 0.0;
		}
	}
}

void bbe::Game::frameDraw(bool dragging)
{
	if (!m_pwindow->isReadyToDraw())
	{
		return;
	}
	if (!m_pwindow->isShown())
	{
		return;
	}

	m_pwindow->preDraw();
	m_pwindow->preDraw3D();
	draw3D(m_pwindow->getBrush3D());
	endMeasure();
	m_pwindow->preDraw2D();
	draw2D(m_pwindow->getBrush2D());
	beginMeasure("INTERNAL - Overhead (wait)");
	m_pwindow->postDraw();
	m_pwindow->waitEndDraw(dragging);
	endMeasure();
}

void bbe::Game::shutdown()
{
	m_pwindow->waitTillIdle();

	onEnd();

	m_pwindow->executeCloseListeners();

#ifndef BBE_NO_AUDIO
	m_soundManager.destroy();
#endif
	INTERNAL::allocCleanup();
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

bool bbe::Game::isKeyPressed(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.isKeyPressed(key);
}

bool bbe::Game::isFocused() const
{
	return m_pwindow->isFocused();
}

bool bbe::Game::isHovered() const
{
	return m_pwindow->isHovered();
}

bool bbe::Game::isMouseDown(bbe::MouseButton button) const
{
	return m_pwindow->INTERNAL_mouse.isButtonDown(button);
}

bool bbe::Game::isMouseUp(bbe::MouseButton button) const
{
	return m_pwindow->INTERNAL_mouse.isButtonUp(button);
}

bool bbe::Game::wasMouseDownLastFrame(bbe::MouseButton button) const
{
	return m_pwindow->INTERNAL_mouse.wasButtonDownLastFrame(button);
}

bool bbe::Game::wasMouseUpLastFrame(bbe::MouseButton button) const
{
	return m_pwindow->INTERNAL_mouse.wasButtonUpLastFrame(button);
}

bool bbe::Game::isMousePressed(bbe::MouseButton button) const
{
	return m_pwindow->INTERNAL_mouse.isButtonPressed(button);
}

bool bbe::Game::isMouseReleased(bbe::MouseButton button) const
{
	return m_pwindow->INTERNAL_mouse.isButtonReleased(button);
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

float bbe::Game::getMouseXPrevious() const
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseXPrevious());
}

float bbe::Game::getMouseYPrevious() const
{
	return (float)(m_pwindow->INTERNAL_mouse.getMouseYPrevious());
}

bbe::Vector2 bbe::Game::getMousePrevious() const
{
	return Vector2(getMouseXPrevious(), getMouseYPrevious());
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

float bbe::Game::getHighestFrameTime()
{
	float retVal = 0.f;

	for (size_t i = 0; i < m_frameTimeHistory.getLength(); i++)
	{
		if (m_frameTimeHistory[i] > retVal) retVal = m_frameTimeHistory[i];
	}

	if (retVal == 0.f) retVal = m_frameTimeRunningAverage;
	return retVal;
}

void bbe::Game::setCursorMode(bbe::CursorMode cm)
{
	m_pwindow->setCursorMode(cm);
}

void bbe::Game::setWindowCloseMode(bbe::WindowCloseMode wcm)
{
	m_pwindow->setWindowCloseMode(wcm);
}

bbe::WindowCloseMode bbe::Game::getWindowCloseMode() const
{
	return m_pwindow->getWindowCloseMode();
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

void bbe::Game::setTargetFrametime(float time)
{
	m_targetFrameTime = time;
}

float bbe::Game::getTargetFrametime() const
{
	return m_targetFrameTime;
}

bbe::String bbe::Game::getClipboard() const
{
	return bbe::String(glfwWrapper::glfwGetClipboardString(m_pwindow->m_pwindow));
}

void bbe::Game::setClipboard(const bbe::String& string)
{
	glfwWrapper::glfwSetClipboardString(m_pwindow->m_pwindow, string.getRaw());
}

void bbe::Game::showWindow()
{
	m_pwindow->showWindow();
}

void bbe::Game::hideWindow()
{
	m_pwindow->hideWindow();
}

void bbe::Game::closeWindow()
{
	m_pwindow->close();
}

bool bbe::Game::isWindowShow() const
{
	return m_pwindow->isShown();
}

void bbe::Game::endMeasure()
{
	if (m_pcurrentPerformanceMeasurementTag)
	{
		auto passedTimeSeconds = m_performanceMeasurement.getTimeExpiredNanoseconds() / 1000.0 / 1000.0 / 1000.0;
		const bool firstMeasurement = !m_performanceMeasurements.count(m_pcurrentPerformanceMeasurementTag);
		PerformanceMeasurement& pm = m_performanceMeasurements[m_pcurrentPerformanceMeasurementTag];
		pm.now = passedTimeSeconds;
		pm.max = bbe::Math::max(pm.max, passedTimeSeconds);
		pm.minuteMax1 = bbe::Math::max(pm.minuteMax1, passedTimeSeconds);
		if (firstMeasurement)
		{
			pm.avg = passedTimeSeconds;
		}
		else
		{

			pm.avg = 0.999 * pm.avg + 0.001 * passedTimeSeconds;
		}
		if (m_performanceMeasurementsRequired || m_performanceMeasurementsForced)
		{
			pm.perFrame.add(passedTimeSeconds);
		}
	}
	m_pcurrentPerformanceMeasurementTag = nullptr;
}

void bbe::Game::beginMeasure(const char* tag, bool force)
{
	endMeasure();
	m_pcurrentPerformanceMeasurementTag = tag;
	m_performanceMeasurement.start();
	m_performanceMeasurementsForced = force;
}

bbe::String bbe::Game::getMeasuresString()
{

	int32_t maxLen = 0;
	for (auto it = m_performanceMeasurements.begin(); it != m_performanceMeasurements.end(); it++)
	{
		maxLen = bbe::Math::max(maxLen, (int32_t)strlen(it->first));
	}

	bbe::String retVal = bbe::String(" ") * maxLen;
	retVal += "  MAX      AVG      NOW      MINUTEMAX\n";

	for (auto it = m_performanceMeasurements.begin(); it != m_performanceMeasurements.end(); it++)
	{
		if (it != m_performanceMeasurements.begin()) retVal += "\n";

		const PerformanceMeasurement& pm = it->second;
		int32_t padding = maxLen - strlen(it->first);
		retVal += it->first;
		retVal += ": ";
		retVal += bbe::String(" ") * padding;
		retVal += pm.max;
		retVal += " ";
		retVal += pm.avg;
		retVal += " ";
		retVal += pm.now;
		retVal += " ";
		retVal += bbe::Math::max(pm.minuteMax1, pm.minuteMax2);
	}

	return retVal;
}

size_t bbe::Game::getAmountOfPlayingSounds() const
{
	return m_soundManager.getAmountOfPlayingSounds();
}
