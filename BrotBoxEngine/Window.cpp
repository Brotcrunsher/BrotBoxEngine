#include "BBE/Window.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/ImGuiExtensions.h"
#include <iostream>
#include "BBE/MouseButtons.h"
#include "BBE/FatalErrors.h"
#include "BBE/Logging.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#ifndef BBE_RENDERER_NULL
#include "imgui_impl_glfw.h"
#endif
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif
#ifdef BBE_RENDERER_NULL
#include "BBE/NullRenderer/NullRendererManager.h"
#endif
#ifdef BBE_RENDERER_OPENGL
#include "BBE/OpenGL/OpenGLManager.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "BBE/Game.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

size_t bbe::Window::windowsAliveCounter = 0;
bbe::Window *bbe::Window::INTERNAL_firstInstance = nullptr;

namespace
{
	struct WindowRecreateState
	{
		std::string title;
		uint32_t major = 0;
		uint32_t minor = 0;
		uint32_t patch = 0;
		bbe::CursorMode cursorMode = bbe::CursorMode::NORMAL;
		bbe::Vector2i lastWindowPos = { 0, 0 };
		bbe::Vector2i lastWindowSize = { 0, 0 };
		bool lastWindowMaximized = false;
		bool hasPlacement = false;
	};

	std::unordered_map<bbe::Window *, WindowRecreateState> g_windowRecreateStates;

#if defined(__linux__) && defined(BBE_USE_WAYLAND_CLIPBOARD)
	bool shouldPreferWaylandPlatform()
	{
		const char *xdgSessionType = std::getenv("XDG_SESSION_TYPE");
		if (xdgSessionType != nullptr && std::strcmp(xdgSessionType, "wayland") == 0)
		{
			return true;
		}

		const char *waylandDisplay = std::getenv("WAYLAND_DISPLAY");
		return waylandDisplay != nullptr && waylandDisplay[0] != '\0';
	}
#endif
}

bbe::Window::Window(int width, int height, const char *title, bbe::Game *game, uint32_t major, uint32_t minor, uint32_t patch)
	: m_width(width), m_height(height), m_pgame(game)
{
	auto &recreateState = g_windowRecreateStates[this];
	recreateState.title = title != nullptr ? title : "";
	recreateState.major = major;
	recreateState.minor = minor;
	recreateState.patch = patch;
	recreateState.lastWindowSize = { width, height };
	recreateState.hasPlacement = true;

#ifdef BBE_RENDERER_VULKAN
	BBELOGLN("Backend: VULKAN");
	m_renderManager.reset(new bbe::INTERNAL::vulkan::VulkanManager());
#endif
#ifdef BBE_RENDERER_NULL
	BBELOGLN("Backend: NULL");
	m_renderManager.reset(new bbe::INTERNAL::nullRenderer::NullRendererManager());
#endif
#ifdef BBE_RENDERER_OPENGL
	BBELOGLN("Backend: OPENGL");
	m_renderManager.reset(new bbe::INTERNAL::openGl::OpenGLManager());
#endif

	if (bbe::Window::INTERNAL_firstInstance == nullptr)
	{
		bbe::Window::INTERNAL_firstInstance = this;
	}
	if (windowsAliveCounter == 0)
	{
		BBELOGLN("GLFWInit");
#if defined(__linux__) && defined(BBE_USE_WAYLAND_CLIPBOARD)
		if (shouldPreferWaylandPlatform())
		{
			glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
		}
#endif
		if (glfwWrapper::glfwInit() == GLFW_FALSE)
		{
			bbe::INTERNAL::triggerFatalError("An error occurred while initializing GLFW.");
		}
#ifdef BBE_RENDERER_VULKAN
		if (glfwWrapper::glfwVulkanSupported() == GLFW_FALSE)
		{
			bbe::INTERNAL::triggerFatalError("Your GPU and/or driver does not support vulkan!");
		}
#endif
	}
	windowsAliveCounter++;

	BBELOGLN("Setting WindowHints");
#ifdef BBE_RENDERER_VULKAN
	glfwWrapper::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef BBE_RENDERER_OPENGL
	glfwWrapper::glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWrapper::glfwWindowHint(GLFW_SAMPLES, 4);
#endif
#ifndef __EMSCRIPTEN__
	glfwWrapper::glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif

	BBELOGLN("Creating GLFW window");
	m_pwindow = glfwWrapper::glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (m_pwindow == nullptr)
	{
		bbe::INTERNAL::triggerFatalError("Could not create window!");
	}

	BBELOGLN("Setting window user pointer");
	glfwWrapper::glfwSetWindowUserPointer(m_pwindow, this);

	BBELOGLN("Init render manager");
	float windowXScale = 0;
	float windowYScale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, &windowXScale, &windowYScale);
	int framebufferWidth = 0;
	int framebufferHeight = 0;
	glfwWrapper::glfwGetFramebufferSize(m_pwindow, &framebufferWidth, &framebufferHeight);
	if (framebufferWidth <= 0 || framebufferHeight <= 0)
	{
		framebufferWidth = static_cast<int>(width * windowXScale);
		framebufferHeight = static_cast<int>(height * windowYScale);
	}
	m_renderManager->init(title, major, minor, patch, m_pwindow, static_cast<uint32_t>(framebufferWidth), static_cast<uint32_t>(framebufferHeight));

	BBELOGLN("Setting glfw callbacks");
	glfwWrapper::glfwSetKeyCallback(m_pwindow, INTERNAL_keyCallback);
	glfwWrapper::glfwSetCharCallback(m_pwindow, INTERNAL_charCallback);
	glfwWrapper::glfwSetCursorPosCallback(m_pwindow, INTERNAL_cursorPosCallback);
	glfwWrapper::glfwSetMouseButtonCallback(m_pwindow, INTERNAL_mouseButtonCallback);
	glfwWrapper::glfwSetWindowSizeCallback(m_pwindow, INTERNAL_windowResizeCallback);
	glfwWrapper::glfwSetFramebufferSizeCallback(m_pwindow, INTERNAL_framebufferResizeCallback);
	glfwWrapper::glfwSetScrollCallback(m_pwindow, INTERNAL_mouseScrollCallback);
	glfwWrapper::glfwSetWindowCloseCallback(m_pwindow, INTERNAL_windowCloseCallback);
	glfwWrapper::glfwSetWindowRefreshCallback(m_pwindow, INTERNAL_windowRefreshCallback);
	glfwWrapper::glfwSetWindowPosCallback(m_pwindow, INTERNAL_windowPosCallback);
	double mX = 0;
	double mY = 0;
	glfwWrapper::glfwGetCursorPos(m_pwindow, &mX, &mY);
	glfwWrapper::glfwSwapInterval(1);

	BBELOGLN("Init mouse");
	INTERNAL_mouse.INTERNAL_moveMouse((float)mX, (float)mY);
}

void bbe::Window::preDraw2D()
{
	if (m_pwindow == nullptr) return;
	m_renderManager->preDraw2D();
}

void bbe::Window::preDraw3D()
{
	if (m_pwindow == nullptr) return;
	m_renderManager->preDraw3D();
}

void bbe::Window::preDraw()
{
	if (m_pwindow == nullptr) return;
	m_renderManager->preDraw();
	m_renderManager->setColor2D(bbe::Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_renderManager->setColor3D(bbe::Color(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::bbe::SetColor(ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 1.0f }));
#ifdef BBE_RENDERER_OPENGL
	((bbe::INTERNAL::openGl::OpenGLManager *)m_renderManager.get())->setRenderMode(bbe::RenderMode::DEFERRED);
#endif
}

bool bbe::Window::keepAlive()
{
	if (m_pwindow == nullptr)
	{
		glfwWrapper::glfwPollEvents();
		return true;
	}

	if (glfwWrapper::glfwWindowShouldClose(m_pwindow))
	{
		return false;
	}

	glfwWrapper::glfwPollEvents();
	return true;
}

void bbe::Window::postDraw()
{
	if (m_pwindow == nullptr) return;
	m_renderManager->postDraw();
}

void bbe::Window::waitEndDraw(bool dragging)
{
	if (m_pwindow == nullptr)
	{
		glfwWrapper::glfwPollEvents();
		return;
	}
	m_renderManager->waitEndDraw();
	if (dragging) glfwWrapper::glFinish();
	else glfwWrapper::glfwPollEvents();
}

void bbe::Window::waitTillIdle()
{
	if (m_pwindow == nullptr) return;
	m_renderManager->waitTillIdle();
}

bool bbe::Window::isReadyToDraw()
{
	if (m_pwindow == nullptr) return false;
	return m_renderManager->isReadyToDraw();
}

bool bbe::Window::isFocused() const
{
	if (m_pwindow == nullptr) return false;
	return glfwWrapper::glfwGetWindowAttrib(m_pwindow, GLFW_FOCUSED);
}

void bbe::Window::setCursorMode(bbe::CursorMode cursorMode)
{
	g_windowRecreateStates[this].cursorMode = cursorMode;
	if (m_pwindow == nullptr) return;
	switch (cursorMode)
	{
	case bbe::CursorMode::DISABLED:
		glfwWrapper::glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	case bbe::CursorMode::NORMAL:
		glfwWrapper::glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case bbe::CursorMode::HIDDEN:
		glfwWrapper::glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
	default:
		bbe::Crash(bbe::Error::IllegalArgument);
	}
}

GLFWwindow *bbe::Window::getRaw()
{
	return m_pwindow;
}

bbe::Window::~Window()
{
	if (m_pwindow != nullptr)
	{
		m_renderManager->destroy();
		glfwWrapper::glfwDestroyWindow(m_pwindow);
		m_pwindow = nullptr;
	}
	if (windowsAliveCounter > 0)
	{
		windowsAliveCounter--;
		if (windowsAliveCounter == 0)
		{
			glfwWrapper::glfwTerminate();
		}
	}
	g_windowRecreateStates.erase(this);

	if (this == bbe::Window::INTERNAL_firstInstance)
	{
		bbe::Window::INTERNAL_firstInstance = nullptr;
	}
}

int bbe::Window::getWidth() const
{
	return m_width;
}

int bbe::Window::getScaledWidth() const
{
	if (m_pwindow == nullptr) return getWidth();
	int framebufferWidth = 0;
	glfwWrapper::glfwGetFramebufferSize(m_pwindow, &framebufferWidth, nullptr);
	return framebufferWidth > 0 ? framebufferWidth : getWidth();
}

int bbe::Window::getHeight() const
{
	return m_height;
}

int bbe::Window::getScaledHeight() const
{
	if (m_pwindow == nullptr) return getHeight();
	int framebufferHeight = 0;
	glfwWrapper::glfwGetFramebufferSize(m_pwindow, nullptr, &framebufferHeight);
	return framebufferHeight > 0 ? framebufferHeight : getHeight();
}

float bbe::Window::getScale() const
{
	if (m_pwindow == nullptr) return 1.0f;
	float scale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, nullptr, &scale);
	return scale;
}

bbe::Vector2i bbe::Window::getSize() const
{
	if (m_pwindow == nullptr)
	{
		auto it = g_windowRecreateStates.find(const_cast<bbe::Window *>(this));
		if (it != g_windowRecreateStates.end()) return it->second.lastWindowSize;
		return { m_width, m_height };
	}
	bbe::Vector2i retVal;
	glfwWrapper::glfwGetWindowSize(m_pwindow, &retVal.x, &retVal.y);
	return retVal;
}

void bbe::Window::setSize(const Vector2i &size)
{
	m_width = size.x;
	m_height = size.y;
	auto &recreateState = g_windowRecreateStates[this];
	recreateState.lastWindowSize = size;
	recreateState.hasPlacement = true;
	requestRender();
	if (m_pwindow != nullptr)
	{
		glfwWrapper::glfwSetWindowSize(m_pwindow, size.x, size.y);
	}
}

bool bbe::Window::isMaximized() const
{
	if (m_pwindow == nullptr)
	{
		auto it = g_windowRecreateStates.find(const_cast<bbe::Window *>(this));
		return it != g_windowRecreateStates.end() && it->second.lastWindowMaximized;
	}
	return glfwWrapper::glfwGetWindowAttrib(m_pwindow, GLFW_MAXIMIZED) != 0;
}

void bbe::Window::maximize()
{
	auto &recreateState = g_windowRecreateStates[this];
	recreateState.lastWindowMaximized = true;
	requestRender();
	if (m_pwindow != nullptr)
	{
		glfwWrapper::glfwMaximizeWindow(m_pwindow);
	}
}

bbe::Vector2i bbe::Window::getPos() const
{
	if (m_pwindow == nullptr)
	{
		auto it = g_windowRecreateStates.find(const_cast<bbe::Window *>(this));
		if (it != g_windowRecreateStates.end()) return it->second.lastWindowPos;
		return { 0, 0 };
	}
	bbe::Vector2i retVal;
	glfwWrapper::glfwGetWindowPos(m_pwindow, &retVal.x, &retVal.y);
	return retVal;
}

void bbe::Window::setPos(const Vector2i &pos)
{
	auto &recreateState = g_windowRecreateStates[this];
	recreateState.lastWindowPos = pos;
	recreateState.hasPlacement = true;
	requestRender();
	if (m_pwindow != nullptr)
	{
		glfwWrapper::glfwSetWindowPos(m_pwindow, pos.x, pos.y);
	}
}

bbe::Vector2 bbe::Window::getGlobalMousePos() const
{
	if (m_pwindow == nullptr) return bbe::Vector2();
#ifdef _WIN32
	POINT pos;
	if (!GetCursorPos(&pos)) return bbe::Vector2();
	return bbe::Vector2((float)pos.x, (float)pos.y);
#else
	int windowPosX;
	int windowPosY;
	glfwWrapper::glfwGetWindowPos(m_pwindow, &windowPosX, &windowPosY);

	double mousePosX;
	double mousePosY;
	glfwWrapper::glfwGetCursorPos(m_pwindow, &mousePosX, &mousePosY);

	return Vector2(static_cast<float>(mousePosX + windowPosX), static_cast<float>(mousePosY + windowPosY));
#endif
}

void bbe::Window::setWindowCloseMode(bbe::WindowCloseMode wcm)
{
	m_windowCloseMode = wcm;
}

bbe::WindowCloseMode bbe::Window::getWindowCloseMode() const
{
	return m_windowCloseMode;
}

void bbe::Window::showWindow()
{
	auto &recreateState = g_windowRecreateStates[this];

	if (m_pwindow == nullptr)
	{
		BBELOGLN("Recreating GLFW window after tray close");

		BBELOGLN("Setting WindowHints");
#ifdef BBE_RENDERER_VULKAN
		glfwWrapper::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef BBE_RENDERER_OPENGL
		glfwWrapper::glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWrapper::glfwWindowHint(GLFW_SAMPLES, 4);
#endif
#ifndef __EMSCRIPTEN__
		glfwWrapper::glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif
		glfwWrapper::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		const int recreateWidth = recreateState.lastWindowSize.x > 0 ? recreateState.lastWindowSize.x : m_width;
		const int recreateHeight = recreateState.lastWindowSize.y > 0 ? recreateState.lastWindowSize.y : m_height;

		m_pwindow = glfwWrapper::glfwCreateWindow(recreateWidth, recreateHeight, recreateState.title.c_str(), nullptr, nullptr);
		glfwWrapper::glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		if (m_pwindow == nullptr)
		{
			bbe::INTERNAL::triggerFatalError("Could not recreate window!");
		}

		glfwWrapper::glfwSetWindowUserPointer(m_pwindow, this);

		float windowXScale = 0;
		float windowYScale = 0;
		glfwWrapper::glfwGetWindowContentScale(m_pwindow, &windowXScale, &windowYScale);
		int framebufferWidth = 0;
		int framebufferHeight = 0;
		glfwWrapper::glfwGetFramebufferSize(m_pwindow, &framebufferWidth, &framebufferHeight);
		if (framebufferWidth <= 0 || framebufferHeight <= 0)
		{
			framebufferWidth = static_cast<int>(recreateWidth * windowXScale);
			framebufferHeight = static_cast<int>(recreateHeight * windowYScale);
		}
		m_renderManager->init(recreateState.title.c_str(), recreateState.major, recreateState.minor, recreateState.patch, m_pwindow, static_cast<uint32_t>(framebufferWidth), static_cast<uint32_t>(framebufferHeight));

		glfwWrapper::glfwSetKeyCallback(m_pwindow, INTERNAL_keyCallback);
		glfwWrapper::glfwSetCharCallback(m_pwindow, INTERNAL_charCallback);
		glfwWrapper::glfwSetCursorPosCallback(m_pwindow, INTERNAL_cursorPosCallback);
		glfwWrapper::glfwSetMouseButtonCallback(m_pwindow, INTERNAL_mouseButtonCallback);
		glfwWrapper::glfwSetWindowSizeCallback(m_pwindow, INTERNAL_windowResizeCallback);
		glfwWrapper::glfwSetFramebufferSizeCallback(m_pwindow, INTERNAL_framebufferResizeCallback);
		glfwWrapper::glfwSetScrollCallback(m_pwindow, INTERNAL_mouseScrollCallback);
		glfwWrapper::glfwSetWindowCloseCallback(m_pwindow, INTERNAL_windowCloseCallback);
		glfwWrapper::glfwSetWindowRefreshCallback(m_pwindow, INTERNAL_windowRefreshCallback);
		glfwWrapper::glfwSetWindowPosCallback(m_pwindow, INTERNAL_windowPosCallback);
		glfwWrapper::glfwSwapInterval(1);

		if (recreateState.hasPlacement)
		{
			glfwWrapper::glfwSetWindowSize(m_pwindow, recreateState.lastWindowSize.x, recreateState.lastWindowSize.y);
			glfwWrapper::glfwSetWindowPos(m_pwindow, recreateState.lastWindowPos.x, recreateState.lastWindowPos.y);
		}

		setCursorMode(recreateState.cursorMode);
		if (recreateState.lastWindowMaximized)
		{
			glfwWrapper::glfwMaximizeWindow(m_pwindow);
		}

		double mX = 0;
		double mY = 0;
		glfwWrapper::glfwGetCursorPos(m_pwindow, &mX, &mY);
		INTERNAL_mouse.INTERNAL_moveMouse((float)mX, (float)mY);
	}

#ifdef __linux__
	glfwWrapper::glfwRestoreWindow(m_pwindow);
	glfwWrapper::glfwShowWindow(m_pwindow);
	glfwWrapper::glfwPollEvents();
#else
	glfwWrapper::glfwShowWindow(m_pwindow);
	glfwWrapper::glfwFocusWindow(m_pwindow);
	glfwWrapper::glfwRestoreWindow(m_pwindow);
#endif
	requestRender();
}

void bbe::Window::hideWindow()
{
	requestRender();
	if (m_pwindow == nullptr) return;
#ifdef __linux__
	auto &recreateState = g_windowRecreateStates[this];
	recreateState.lastWindowSize = getSize();
	recreateState.lastWindowPos = getPos();
	recreateState.lastWindowMaximized = isMaximized();
	recreateState.hasPlacement = true;

	m_renderManager->waitTillIdle();
	m_renderManager->destroy();
	glfwWrapper::glfwDestroyWindow(m_pwindow);
	m_pwindow = nullptr;
#else
	glfwWrapper::glfwHideWindow(m_pwindow);
#endif
}

bool bbe::Window::isShown() const
{
	if (m_pwindow == nullptr) return false;
	int visible = glfwWrapper::glfwGetWindowAttrib(m_pwindow, GLFW_VISIBLE);
	return visible;
}

bool bbe::Window::isHovered() const
{
	if (m_pwindow == nullptr) return false;
	int hovered = glfwWrapper::glfwGetWindowAttrib(m_pwindow, GLFW_HOVERED);
	return hovered;
}

bbe::PrimitiveBrush2D &bbe::Window::getBrush2D()
{
	return m_renderManager->getBrush2D();
}

bbe::PrimitiveBrush3D &bbe::Window::getBrush3D()
{
	return m_renderManager->getBrush3D();
}

void bbe::Window::INTERNAL_resize(int width, int height)
{
	if (m_pwindow == nullptr) return;
	m_width = width;
	m_height = height;
	requestRender();

	int framebufferWidth = 0;
	int framebufferHeight = 0;
	glfwWrapper::glfwGetFramebufferSize(m_pwindow, &framebufferWidth, &framebufferHeight);
	if (framebufferWidth > 0 && framebufferHeight > 0)
	{
		m_renderManager->resize(static_cast<uint32_t>(framebufferWidth), static_cast<uint32_t>(framebufferHeight));
	}
}

void bbe::Window::INTERNAL_resizeFramebuffer(int width, int height)
{
	if (m_pwindow == nullptr) return;
	glfwWrapper::glfwGetWindowSize(m_pwindow, &m_width, &m_height);
	if (width <= 0 || height <= 0)
	{
		return;
	}
	requestRender();
	m_renderManager->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

void bbe::Window::INTERNAL_onRefresh()
{
	requestRender();
	m_pgame->frame(true);
}

void bbe::Window::screenshot(const bbe::String &path)
{
	if (m_pwindow == nullptr) return;
	m_renderManager->screenshot(path);
}

void bbe::Window::setVideoRenderingMode(const char *path)
{
	if (m_pwindow == nullptr) return;
	m_renderManager->setVideoRenderingMode(path);
}

void bbe::Window::close()
{
	if (m_pwindow == nullptr) return;
	glfwWrapper::glfwSetWindowShouldClose(m_pwindow, GLFW_TRUE);
}

void bbe::Window::registerCloseListener(const std::function<void()> &listener)
{
	m_closeListeners.add(listener);
}

void bbe::Window::executeCloseListeners()
{
	for (const std::function<void()> listener : m_closeListeners)
	{
		listener();
	}
}

void bbe::Window::registerFrameStartListener(const std::function<void()> &listener)
{
	m_frameStartListeners.add(listener);
}

void bbe::Window::executeFrameStartListeneres()
{
	for (const std::function<void()> listener : m_frameStartListeners)
	{
		listener();
	}
}

void bbe::Window::update()
{
	executeFrameStartListeneres();
	INTERNAL_keyboard.update();
	if (m_pwindow == nullptr) return;
	const bbe::Vector2 globalMousePos = getGlobalMousePos();
	INTERNAL_mouse.update(globalMousePos.x, globalMousePos.y);
	if (isShown() && !isFocused() && isHovered())
	{
		// NOTE: mousePosX/Y are not globalMousePos! mousePosX/Y are relative
		//       to the window, globalMousePos is actually global.
		double mousePosX;
		double mousePosY;
		glfwWrapper::glfwGetCursorPos(m_pwindow, &mousePosX, &mousePosY);
#ifndef BBE_RENDERER_NULL
		ImGui_ImplGlfw_CursorPosCallback(m_pwindow, mousePosX, mousePosY);
#endif
	}
}

void bbe::Window::requestRender()
{
	m_hasPendingRenderRequest = true;
}

bool bbe::Window::hasPendingRenderRequest() const
{
	return m_hasPendingRenderRequest;
}

void bbe::Window::consumeRenderRequest()
{
	m_hasPendingRenderRequest = false;
}

void *bbe::Window::getNativeHandle()
{
	if (m_pwindow == nullptr) return nullptr;
#ifdef WIN32
	return (void *)glfwGetWin32Window(m_pwindow);
#else
	return nullptr;
#endif
}

#ifdef BBE_RENDERER_OPENGL
uint32_t bbe::Window::getAmountOfDrawcalls() const
{
	if (m_pwindow == nullptr) return 0;
	return ((bbe::INTERNAL::openGl::OpenGLManager *)m_renderManager.get())->getAmountOfDrawcalls();
}
#endif

void bbe::INTERNAL_keyCallback(GLFWwindow *window, int keyCode, int scanCode, int action, int mods)
{
#ifndef BBE_RENDERER_NULL
	if (keyCode == GLFW_KEY_UNKNOWN)
	{
		// This can happen for example when pressing the FN key on some platforms.
		// As we don't care about that key, we just drop the event.
		return;
	}
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->requestRender();
	keyCode = ImGui_ImplGlfw_TranslateUntranslatedKey(keyCode, scanCode);
	ImGui_ImplGlfw_KeyCallback(window, keyCode, scanCode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard) return;
	if (action == GLFW_PRESS)
	{
		((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_keyboard.INTERNAL_press((bbe::Key)keyCode);
	}
	else if (action == GLFW_RELEASE)
	{
		((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_keyboard.INTERNAL_release((bbe::Key)keyCode);
	}
#endif
}

void bbe::INTERNAL_charCallback(GLFWwindow *window, unsigned int c)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->requestRender();
#ifndef BBE_RENDERER_NULL
	ImGui_ImplGlfw_CharCallback(window, c);
#endif
}

void bbe::INTERNAL_cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->requestRender();
#ifdef BBE_RENDERER_VULKAN
	if (ImGui::GetIO().WantCaptureMouse) return;
#endif
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_mouse.INTERNAL_moveMouse(static_cast<float>(xpos), static_cast<float>(ypos));
}

void bbe::INTERNAL_windowResizeCallback(GLFWwindow *window, int width, int height)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_resize(width, height);
}

void bbe::INTERNAL_framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_resizeFramebuffer(width, height);
}

void bbe::INTERNAL_mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->requestRender();
#ifndef BBE_RENDERER_NULL
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	if (ImGui::GetIO().WantCaptureMouse) return;
#endif

	if (action == GLFW_PRESS)
	{
		((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_mouse.INTERNAL_press((bbe::MouseButton)button);
	}
	else if (action == GLFW_RELEASE)
	{
		((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_mouse.INTERNAL_release((bbe::MouseButton)button);
	}
}

void bbe::INTERNAL_mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->requestRender();
#ifndef BBE_RENDERER_NULL
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	// TODO: This lead to issues in the MOTHER console. We need to have scroll events there, even though a ImGui Window is in the foreground. Fix me.
	//if (ImGui::GetIO().WantCaptureMouse) return;
#endif
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_mouse.INTERNAL_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void bbe::INTERNAL_windowCloseCallback(GLFWwindow *window)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->requestRender();
	switch (((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->getWindowCloseMode())
	{
	case bbe::WindowCloseMode::CLOSE:
		// Do nothing
		break;
	case bbe::WindowCloseMode::HIDE:
		glfwWrapper::glfwSetWindowShouldClose(window, GLFW_FALSE);
		((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->hideWindow();
		break;
	default:
		bbe::Crash(bbe::Error::IllegalState);
	}
}

void bbe::INTERNAL_windowRefreshCallback(GLFWwindow *window)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_onRefresh();
}

void bbe::INTERNAL_windowPosCallback(GLFWwindow *window, int, int)
{
	((bbe::Window *)glfwWrapper::glfwGetWindowUserPointer(window))->INTERNAL_onRefresh();
}

template<>
uint32_t bbe::hash(const bbe::Window &t)
{
	//UNTESTED
	return t.getWidth() * 7 + t.getHeight() * 13;
}
