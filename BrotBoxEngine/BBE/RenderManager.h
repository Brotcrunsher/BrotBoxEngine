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
	class Circle;
	class Image;
	class Matrix4;

	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;

	class RenderManager
	{
	private:
		FillMode m_fillMode = FillMode::SOLID;
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

		void setFillMode2D(bbe::FillMode fm);
		bbe::FillMode getFillMode2D();
		virtual void setColor2D(const bbe::Color& color) = 0;
		virtual void fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader) = 0;
		virtual void fillCircle2D(const Circle& circle) = 0;
		virtual void drawImage2D(const Rectangle& rect, const Image& image, float rotation) = 0;
		virtual void fillVertexIndexList2D(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2 &scale) = 0;

		virtual void setColor3D(const bbe::Color& color) = 0;
		virtual void setCamera3D(const bbe::Matrix4& m_view, const bbe::Matrix4& m_projection) = 0;
	};
}
