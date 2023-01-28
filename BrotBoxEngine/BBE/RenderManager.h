#pragma once

#include <stdint.h>
#include "../BBE/String.h"
#include "../BBE/FillMode.h"
#include "../BBE/Vector3.h"
#include "../BBE/LightFalloffMode.h"
#include "../BBE/PointLight.h"

struct GLFWwindow;

namespace bbe
{
	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;

	class PrimitiveBrush2D;
	class PrimitiveBrush3D;
	class FragmentShader;
	class Color;
	template<typename Vec> class Rectangle_t;
	using Rectangle = Rectangle_t<bbe::Vector2>;
	class Cube;
	class IcoSphere;
	class Circle;
	class Image;
	class Matrix4;

	class RenderManager
	{
	private:
		FillMode m_fillMode2D = FillMode::SOLID;
		FillMode m_fillMode3D = FillMode::SOLID;
	public:
		virtual ~RenderManager() = default;

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

		void setFillMode3D(bbe::FillMode fm);
		bbe::FillMode getFillMode3D();
		virtual void setColor3D(const bbe::Color& color) = 0;
		virtual void setCamera3D(const Vector3& cameraPos, const bbe::Matrix4& m_view, const bbe::Matrix4& m_projection) = 0;
		virtual void fillCube3D(const Cube& cube) = 0;
		virtual void fillSphere3D(const IcoSphere& sphere) = 0;
		virtual void addLight(const bbe::Vector3& pos, float lightStrength, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode) = 0;

		virtual void imguiStart() = 0;
		virtual void imguiStop() = 0;
		virtual void imguiStartFrame() = 0;
		virtual void imguiEndFrame() = 0;
	};
}
