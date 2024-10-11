#include "BBE/Game.h"
#include "BBE/Window.h"
#include "BBE/Error.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Math.h"
#include "BBE/StopWatch.h"
#include "BBE/SimpleFile.h"
#include <iostream>
#include "implot.h"
#include "BBE/ImGuiExtensions.h"
#include "BBE/Logging.h"
#include "BBE/Error.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <signal.h>
#if __has_include(<stacktrace>)
#include <stacktrace>
#else
#pragma warning("Stacktrace lib is not present!")
#endif

#ifdef _WIN32
#define NOMINMAX
#include <Objbase.h>
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

static void segvHandler(int sig)
{
	bbe::Crash(bbe::Error::Segfault);
}

void bbe::Game::start(int windowWidth, int windowHeight, const char* title)
{
	BBE_TRY_RELEASE
	{
		innerStart(windowWidth, windowHeight, title);
	}
	BBE_CATCH_RELEASE(Main Thread)
}

#ifdef _WIN32
const char* vecExToString(DWORD ex)
{
	if(ex == EXCEPTION_ACCESS_VIOLATION        ) return "EXCEPTION_ACCESS_VIOLATION       ";
	if(ex == EXCEPTION_DATATYPE_MISALIGNMENT   ) return "EXCEPTION_DATATYPE_MISALIGNMENT  ";
	if(ex == EXCEPTION_BREAKPOINT              ) return "EXCEPTION_BREAKPOINT             ";
	if(ex == EXCEPTION_SINGLE_STEP             ) return "EXCEPTION_SINGLE_STEP            ";
	if(ex == EXCEPTION_ARRAY_BOUNDS_EXCEEDED   ) return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED  ";
	if(ex == EXCEPTION_FLT_DENORMAL_OPERAND    ) return "EXCEPTION_FLT_DENORMAL_OPERAND   ";
	if(ex == EXCEPTION_FLT_DIVIDE_BY_ZERO      ) return "EXCEPTION_FLT_DIVIDE_BY_ZERO     ";
	if(ex == EXCEPTION_FLT_INEXACT_RESULT      ) return "EXCEPTION_FLT_INEXACT_RESULT     ";
	if(ex == EXCEPTION_FLT_INVALID_OPERATION   ) return "EXCEPTION_FLT_INVALID_OPERATION  ";
	if(ex == EXCEPTION_FLT_OVERFLOW            ) return "EXCEPTION_FLT_OVERFLOW           ";
	if(ex == EXCEPTION_FLT_STACK_CHECK         ) return "EXCEPTION_FLT_STACK_CHECK        ";
	if(ex == EXCEPTION_FLT_UNDERFLOW           ) return "EXCEPTION_FLT_UNDERFLOW          ";
	if(ex == EXCEPTION_INT_DIVIDE_BY_ZERO      ) return "EXCEPTION_INT_DIVIDE_BY_ZERO     ";
	if(ex == EXCEPTION_INT_OVERFLOW            ) return "EXCEPTION_INT_OVERFLOW           ";
	if(ex == EXCEPTION_PRIV_INSTRUCTION        ) return "EXCEPTION_PRIV_INSTRUCTION       ";
	if(ex == EXCEPTION_IN_PAGE_ERROR           ) return "EXCEPTION_IN_PAGE_ERROR          ";
	if(ex == EXCEPTION_ILLEGAL_INSTRUCTION     ) return "EXCEPTION_ILLEGAL_INSTRUCTION    ";
	if(ex == EXCEPTION_NONCONTINUABLE_EXCEPTION) return "EXCEPTION_NONCONTINUABLE_EXCEPTIO";
	if(ex == EXCEPTION_STACK_OVERFLOW          ) return "EXCEPTION_STACK_OVERFLOW         ";
	if(ex == EXCEPTION_INVALID_DISPOSITION     ) return "EXCEPTION_INVALID_DISPOSITION    ";
	if(ex == EXCEPTION_GUARD_PAGE              ) return "EXCEPTION_GUARD_PAGE             ";
	if(ex == EXCEPTION_INVALID_HANDLE          ) return "EXCEPTION_INVALID_HANDLE         ";

	if(ex == STATUS_WAIT_0                    ) return "STATUS_WAIT_0                    ";
	if(ex == STATUS_ABANDONED_WAIT_0          ) return "STATUS_ABANDONED_WAIT_0          ";
	if(ex == STATUS_USER_APC                  ) return "STATUS_USER_APC                  ";
	if(ex == STATUS_TIMEOUT                   ) return "STATUS_TIMEOUT                   ";
	if(ex == STATUS_PENDING                   ) return "STATUS_PENDING                   ";
	if(ex == DBG_EXCEPTION_HANDLED            ) return "DBG_EXCEPTION_HANDLED            ";
	if(ex == DBG_CONTINUE                     ) return "DBG_CONTINUE                     ";
	if(ex == STATUS_SEGMENT_NOTIFICATION      ) return "STATUS_SEGMENT_NOTIFICATION      ";
	if(ex == STATUS_FATAL_APP_EXIT            ) return "STATUS_FATAL_APP_EXIT            ";
	if(ex == DBG_REPLY_LATER                  ) return "DBG_REPLY_LATER                  ";
	if(ex == DBG_TERMINATE_THREAD             ) return "DBG_TERMINATE_THREAD             ";
	if(ex == DBG_TERMINATE_PROCESS            ) return "DBG_TERMINATE_PROCESS            ";
	if(ex == DBG_CONTROL_C                    ) return "DBG_CONTROL_C                    ";
	if(ex == DBG_PRINTEXCEPTION_C             ) return "DBG_PRINTEXCEPTION_C             ";
	if(ex == DBG_RIPEXCEPTION                 ) return "DBG_RIPEXCEPTION                 ";
	if(ex == DBG_CONTROL_BREAK                ) return "DBG_CONTROL_BREAK                ";
	if(ex == DBG_COMMAND_EXCEPTION            ) return "DBG_COMMAND_EXCEPTION            ";
	if(ex == DBG_PRINTEXCEPTION_WIDE_C        ) return "DBG_PRINTEXCEPTION_WIDE_C        ";
	if(ex == STATUS_GUARD_PAGE_VIOLATION      ) return "STATUS_GUARD_PAGE_VIOLATION      ";
	if(ex == STATUS_DATATYPE_MISALIGNMENT     ) return "STATUS_DATATYPE_MISALIGNMENT     ";
	if(ex == STATUS_BREAKPOINT                ) return "STATUS_BREAKPOINT                ";
	if(ex == STATUS_SINGLE_STEP               ) return "STATUS_SINGLE_STEP               ";
	if(ex == STATUS_LONGJUMP                  ) return "STATUS_LONGJUMP                  ";
	if(ex == STATUS_UNWIND_CONSOLIDATE        ) return "STATUS_UNWIND_CONSOLIDATE        ";
	if(ex == DBG_EXCEPTION_NOT_HANDLED        ) return "DBG_EXCEPTION_NOT_HANDLED        ";
	if(ex == STATUS_ACCESS_VIOLATION          ) return "STATUS_ACCESS_VIOLATION          ";
	if(ex == STATUS_IN_PAGE_ERROR             ) return "STATUS_IN_PAGE_ERROR             ";
	if(ex == STATUS_INVALID_HANDLE            ) return "STATUS_INVALID_HANDLE            ";
	if(ex == STATUS_INVALID_PARAMETER         ) return "STATUS_INVALID_PARAMETER         ";
	if(ex == STATUS_NO_MEMORY                 ) return "STATUS_NO_MEMORY                 ";
	if(ex == STATUS_ILLEGAL_INSTRUCTION       ) return "STATUS_ILLEGAL_INSTRUCTION       ";
	if(ex == STATUS_NONCONTINUABLE_EXCEPTION  ) return "STATUS_NONCONTINUABLE_EXCEPTION  ";
	if(ex == STATUS_INVALID_DISPOSITION       ) return "STATUS_INVALID_DISPOSITION       ";
	if(ex == STATUS_ARRAY_BOUNDS_EXCEEDED     ) return "STATUS_ARRAY_BOUNDS_EXCEEDED     ";
	if(ex == STATUS_FLOAT_DENORMAL_OPERAND    ) return "STATUS_FLOAT_DENORMAL_OPERAND    ";
	if(ex == STATUS_FLOAT_DIVIDE_BY_ZERO      ) return "STATUS_FLOAT_DIVIDE_BY_ZERO      ";
	if(ex == STATUS_FLOAT_INEXACT_RESULT      ) return "STATUS_FLOAT_INEXACT_RESULT      ";
	if(ex == STATUS_FLOAT_INVALID_OPERATION   ) return "STATUS_FLOAT_INVALID_OPERATION   ";
	if(ex == STATUS_FLOAT_OVERFLOW            ) return "STATUS_FLOAT_OVERFLOW            ";
	if(ex == STATUS_FLOAT_STACK_CHECK         ) return "STATUS_FLOAT_STACK_CHECK         ";
	if(ex == STATUS_FLOAT_UNDERFLOW           ) return "STATUS_FLOAT_UNDERFLOW           ";
	if(ex == STATUS_INTEGER_DIVIDE_BY_ZERO    ) return "STATUS_INTEGER_DIVIDE_BY_ZERO    ";
	if(ex == STATUS_INTEGER_OVERFLOW          ) return "STATUS_INTEGER_OVERFLOW          ";
	if(ex == STATUS_PRIVILEGED_INSTRUCTION    ) return "STATUS_PRIVILEGED_INSTRUCTION    ";
	if(ex == STATUS_STACK_OVERFLOW            ) return "STATUS_STACK_OVERFLOW            ";
	if(ex == STATUS_DLL_NOT_FOUND             ) return "STATUS_DLL_NOT_FOUND             ";
	if(ex == STATUS_ORDINAL_NOT_FOUND         ) return "STATUS_ORDINAL_NOT_FOUND         ";
	if(ex == STATUS_ENTRYPOINT_NOT_FOUND      ) return "STATUS_ENTRYPOINT_NOT_FOUND      ";
	if(ex == STATUS_CONTROL_C_EXIT            ) return "STATUS_CONTROL_C_EXIT            ";
	if(ex == STATUS_DLL_INIT_FAILED           ) return "STATUS_DLL_INIT_FAILED           ";
	if(ex == STATUS_CONTROL_STACK_VIOLATION   ) return "STATUS_CONTROL_STACK_VIOLATION   ";
	if(ex == STATUS_FLOAT_MULTIPLE_FAULTS     ) return "STATUS_FLOAT_MULTIPLE_FAULTS     ";
	if(ex == STATUS_FLOAT_MULTIPLE_TRAPS      ) return "STATUS_FLOAT_MULTIPLE_TRAPS      ";
	if(ex == STATUS_REG_NAT_CONSUMPTION       ) return "STATUS_REG_NAT_CONSUMPTION       ";
	if(ex == STATUS_HEAP_CORRUPTION           ) return "STATUS_HEAP_CORRUPTION           ";
	if(ex == STATUS_STACK_BUFFER_OVERRUN      ) return "STATUS_STACK_BUFFER_OVERRUN      ";
	if(ex == STATUS_INVALID_CRUNTIME_PARAMETER) return "STATUS_INVALID_CRUNTIME_PARAMETER";
	if(ex == STATUS_ASSERTION_FAILURE         ) return "STATUS_ASSERTION_FAILURE         ";
	if(ex == STATUS_ENCLAVE_VIOLATION         ) return "STATUS_ENCLAVE_VIOLATION         ";
	if(ex == STATUS_INTERRUPTED               ) return "STATUS_INTERRUPTED               ";
	if(ex == STATUS_THREAD_NOT_RUNNING        ) return "STATUS_THREAD_NOT_RUNNING        ";
	if(ex == STATUS_ALREADY_REGISTERED        ) return "STATUS_ALREADY_REGISTERED        ";

	return "EXCEPTION_UNKNOWN";
}

LONG WINAPI UnhandledVectoredExceptionHandler(EXCEPTION_POINTERS* exceptionPointers)
{
	// 0x406d1388 is not a "real exception" but instead a hack that is commonly used to set the name of a thread so that
	// it's visible within the debugger. When such an exception is raised, we do not want to crash.
	// See also: https://stackoverflow.com/questions/478875/how-to-change-the-name-of-a-thread
	// Within the BBE, this is used by OpenAL.
	if (exceptionPointers->ExceptionRecord->ExceptionCode == 0x406d1388) return EXCEPTION_CONTINUE_SEARCH;

	// 0xE06D7363 stands for a C++ Exception. Such are handled with try/catch blocks and should not necessarily cause a crash.
	if (exceptionPointers->ExceptionRecord->ExceptionCode == 0xE06D7363) return EXCEPTION_CONTINUE_SEARCH;


	bbe::String msg = "Exception Code: ";
	msg += vecExToString(exceptionPointers->ExceptionRecord->ExceptionCode);
	msg += "(";
	msg += exceptionPointers->ExceptionRecord->ExceptionCode;
	msg += ")\n";
	msg += "       Exception Flags: " + bbe::String(exceptionPointers->ExceptionRecord->ExceptionFlags);

	bbe::Crash(bbe::Error::VectoredException, msg.getRaw());

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void bbe::Game::innerStart(int windowWidth, int windowHeight, const char* title)
{
#ifdef _WIN32
	AddVectoredExceptionHandler(1, UnhandledVectoredExceptionHandler);

	HRESULT res = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (res != S_OK)
	{
		bbe::Crash(bbe::Error::IllegalState, "CoInitializeEx Failed");
	}
	{
		WSADATA wsaData = { 0 };

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
		{
			bbe::Crash(bbe::Error::IllegalState, "WSAStartup Failed");
		}
	}
#endif
	signal(SIGSEGV, segvHandler);

	BBELOGLN("Starting Game: " << title);
	if (m_started)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
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
	else m_frameTimeRunningAverage = 0.99f * m_frameTimeRunningAverage + 0.01f * timeSinceLastFrame;

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
	endMeasure();
	update(timeSinceLastFrame);

	beginMeasure("INTERNAL - MinuteMaxMove");
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
	endMeasure();
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

	beginMeasure("INTERNAL - Pre Draw 3D");
	m_pwindow->preDraw();
	m_pwindow->preDraw3D();
	endMeasure();
	draw3D(m_pwindow->getBrush3D());
	beginMeasure("INTERNAL - Pre Draw 2D");
	m_pwindow->preDraw2D();
	endMeasure();
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
	bbe::simpleFile::backup::async::stopIoThread();
#ifdef WIN32
	WSACleanup();
#endif
}

void bbe::Game::setExternallyManaged(bool managed)
{
	if (m_started)
	{
		// Managed must be set before calling start!
		bbe::Crash(bbe::Error::AlreadyCreated);
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

bool bbe::Game::isKeyTyped(bbe::Key key)
{
	return m_pwindow->INTERNAL_keyboard.isKeyTyped(key);
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
		bbe::Crash(bbe::Error::IllegalState);
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
		bbe::Crash(bbe::Error::IllegalState);
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
		int32_t padding = maxLen - (int32_t)strlen(it->first);
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

void bbe::Game::drawMeasurement()
{
	double maxMax = 0.0;
	double maxAvg = 0.0;
	double maxNow = 0.0;
	double maxMinuteMax = 0.0;

	int32_t maxLen = 0;
	for (auto it = m_performanceMeasurements.begin(); it != m_performanceMeasurements.end(); it++)
	{
		maxLen = bbe::Math::max(maxLen, (int32_t)strlen(it->first));

		maxMax = bbe::Math::max(maxMax, it->second.max);
		maxAvg = bbe::Math::max(maxAvg, it->second.avg);
		maxNow = bbe::Math::max(maxNow, it->second.now);
		maxMinuteMax = bbe::Math::max(maxMinuteMax, it->second.minuteMax1);
		maxMinuteMax = bbe::Math::max(maxMinuteMax, it->second.minuteMax2);
	}

	bbe::String header = bbe::String(" ") * maxLen;
	header += "  MAX      AVG      NOW      MINUTEMAX\n";
	ImGui::Text(header);

	for (auto it = m_performanceMeasurements.begin(); it != m_performanceMeasurements.end(); it++)
	{
		const PerformanceMeasurement& pm = it->second;
		int32_t padding = maxLen - (int32_t)strlen(it->first);
		ImGui::Text(it->first);
		ImGui::SameLine(0.0f, 0.0f); ImGui::Text(": ");
		ImGui::SameLine(0.0f, 0.0f); ImGui::Text(bbe::String(" ") * padding);

		const double maxPercentage = pm.max / maxMax;
		ImGui::SameLine(0.0f, 0.0f); ImGui::TextColored(ImVec4(1.0f, 1.0f - maxPercentage, 1.0f - maxPercentage, 1.0f), bbe::String(pm.max));
		ImGui::SameLine(0.0f, 0.0f); ImGui::Text(" ");
		const double avgPercentage = pm.avg / maxAvg;
		ImGui::SameLine(0.0f, 0.0f); ImGui::TextColored(ImVec4(1.0f, 1.0f - avgPercentage, 1.0f - avgPercentage, 1.0f), bbe::String(pm.avg));
		ImGui::SameLine(0.0f, 0.0f); ImGui::Text(" ");
		const double nowPercentage = pm.now / maxNow;
		ImGui::SameLine(0.0f, 0.0f); ImGui::TextColored(ImVec4(1.0f, 1.0f - nowPercentage, 1.0f - nowPercentage, 1.0f), bbe::String(pm.now));
		ImGui::SameLine(0.0f, 0.0f); ImGui::Text(" ");
		const double minuteMaxPercentage = bbe::Math::max(pm.minuteMax1, pm.minuteMax2) / maxMinuteMax;
		ImGui::SameLine(0.0f, 0.0f); ImGui::TextColored(ImVec4(1.0f, 1.0f - minuteMaxPercentage, 1.0f - minuteMaxPercentage, 1.0f), bbe::String(bbe::Math::max(pm.minuteMax1, pm.minuteMax2)));
	}
}

size_t bbe::Game::getAmountOfPlayingSounds() const
{
	return m_soundManager.getAmountOfPlayingSounds();
}
