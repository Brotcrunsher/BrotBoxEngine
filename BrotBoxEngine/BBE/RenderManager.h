#pragma once

#include <stdint.h>
#include "../BBE/String.h"
#include "../BBE/FillMode.h"

struct GLFWwindow;

namespace bbe
{
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;
	class FragmentShader;
	class Color;
	class Rectangle;

	class RenderManager
	{
	public:
		virtual void init(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow* window, uint32_t initialWindowWidth, uint32_t initialWindowHeight) = 0;

		virtual void destroy() = 0;
		virtual void preDraw2D() = 0;
		virtual void preDraw3D() = 0;
		virtual void preDraw() = 0;
		virtual void postDraw() = 0;
		virtual void waitEndDraw() = 0;
		virtual void waitTillIdle() = 0;

		virtual bool isReadyToDraw() const = 0;

		virtual bbe::PrimitiveBrush2D& getBrush2D() = 0;
		virtual bbe::PrimitiveBrush3D& getBrush3D() = 0;
		virtual void resize(uint32_t width, uint32_t height) = 0;
		virtual void screenshot(const bbe::String& path) = 0;
		virtual void setVideoRenderingMode(const char* path) = 0;

		virtual void setColor2D(const bbe::Color& color) = 0;
		virtual void fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader) = 0;
		virtual void setFillMode2D(bbe::FillMode fm) = 0;
		virtual bbe::FillMode getFillMode2D() = 0;
	};
}
