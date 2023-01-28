#pragma once

#include "../BBE/glfwWrapper.h"
#include "../BBE/Color.h"
#include "../BBE/FillMode.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	class Image;
	class FragmentShader;
	class RectangleRotated;
	class RenderManager;
	template<typename Vec> class Line2_t;
	using Line2 = Line2_t<bbe::Vector2>;
	class Font;
	class BezierCurve2;
	class Circle;
	class PhysCircle;
	template<typename Vec> class Rectangle_t;
	using Rectangle = Rectangle_t<bbe::Vector2>;
	class PhysRectangle;

	enum class PipelineRecord2D
	{
		NONE, PRIMITIVE, IMAGE
	};

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
		namespace nullRenderer
		{
			class NullRendererManager;
		}
		namespace openGl
		{
			class OpenGLManager;
		}
	}

	class PrimitiveBrush2D
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class INTERNAL::nullRenderer::NullRendererManager;
		friend class INTERNAL::openGl::OpenGLManager;
	private:

		float                                        m_windowXScale = 0;
		float                                        m_windowYScale = 0;
		int                                          m_screenWidth  = 0;
		int                                          m_screenHeight = 0;
		float                                        m_outlineWidth = 0;
		Color m_color = Color(-1000, -1000, -1000);
		Color m_outlineColor = Color(-1000, -1000, -1000);
		bbe::Vector2 m_offset = {0, 0};

		bbe::RenderManager* m_prenderManager = nullptr;

		void INTERNAL_fillRect(const Rectangle &rect, float rotation, float outlineWidth, FragmentShader* shader);
		void INTERNAL_drawImage(const Rectangle &rect, const Image &image, float rotation);
		void INTERNAL_fillCircle(const Circle &circle, float outlineWidth);
		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(
			GLFWwindow* window,
			int screenWidth, int screenHeight,
			bbe::RenderManager *renderManager
		);

		void INTERNAL_destroy();

	public:
#ifdef _WIN32
		static constexpr const char* DEFAULT_FONT_NAME = "arial.ttf";
#elif defined(unix) || defined(linux)
		static constexpr const char* DEFAULT_FONT_NAME = "dejavu";
#else
		static constexpr const char* DEFAULT_FONT_NAME = "";
#endif
		static constexpr unsigned DEFAULT_FONT_SIZE = 20;

		PrimitiveBrush2D();

		template<typename T>
		void fillRect(const Rectangle_t<T>& rect, float rotation = 0, FragmentShader* shader = nullptr)
		{
			fillRect(rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), rotation, shader);
		}
		void fillRect(float x, float y,   float width, float height, float rotation = 0, FragmentShader* shader = nullptr);
		void fillRect(const Vector2& pos, float width, float height, float rotation = 0, FragmentShader* shader = nullptr);
		void fillRect(float x, float y,   const Vector2& dimensions, float rotation = 0, FragmentShader* shader = nullptr);
		void fillRect(const Vector2& pos, const Vector2& dimensions, float rotation = 0, FragmentShader* shader = nullptr);
		void fillRect(const PhysRectangle& rect, FragmentShader* shader = nullptr);
		void fillRect(const RectangleRotated& rect, FragmentShader* shader = nullptr);

		void sketchRect(float x, float y, float width, float height);
		void sketchRect(const Vector2& pos, float width, float height);
		void sketchRect(float x, float y, const Vector2& dim);
		void sketchRect(const Vector2& pos, const Vector2& dim);
		void sketchRect(const Rectangle& rect);

		void fillCircle(const Circle &circle);
		void fillCircle(float x, float y, float width, float height);
		void fillCircle(const Vector2& pos, float width, float height);
		void fillCircle(float x, float y, const Vector2& dimensions);
		void fillCircle(const Vector2& pos, const Vector2& dimensions);
		void fillCircle(const PhysCircle& circle);

		void fillTriangle(const Vector2& a,   const Vector2& b,   const Vector2& c);

		void drawImage(const Rectangle &rect, const Image &image, float rotation = 0);
		void drawImage(float x, float y,   float width, float height, const Image &image, float rotation = 0);
		void drawImage(const Vector2& pos, float width, float height, const Image& image, float rotation = 0);
		void drawImage(float x, float y,   const Vector2& dimensions, const Image& image, float rotation = 0);
		void drawImage(const Vector2& pos, const Vector2& dimensions, const Image& image, float rotation = 0);
		void drawImage(float x, float y, const Image& image, float rotation = 0);
		void drawImage(const Vector2& pos, const Image& image, float rotation = 0);

		void fillLine(float x1, float y1, float x2, float y2, float lineWidth = 1);
		void fillLine(const Vector2 &p1,  float x2, float y2, float lineWidth = 1);
		void fillLine(float x1, float y1, const Vector2 &p2,  float lineWidth = 1);
		void fillLine(const Vector2 &p1,  const Vector2 &p2,  float lineWidth = 1);
		void fillLine(const Line2& line, float lineWidth = 1);
		
		void fillArrow(float x1, float y1, float x2, float y2, float tailWidth = 1, float spikeInnerLength = 20, float spikeOuterLength = 30, float spikeAngle = 0.35, bool dynamicSpikeLength = true);
		void fillArrow(const Vector2& p1, const Vector2& p2,   float tailWidth = 1, float spikeInnerLength = 20, float spikeOuterLength = 30, float spikeAngle = 0.35, bool dynamicSpikeLength = true);

		void fillLineStrip(const bbe::List<bbe::Vector2> &points, bool closed, float lineWidth = 1);

		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const bbe::List<Vector2>& controlPoints);
		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint);
		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control);
		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control1, const Vector2& control2);
		void fillBezierCurve(const BezierCurve2& bc, float lineWidth = 1);

		void fillChar(float x, float y, int32_t c, const bbe::Font& font, float rotation = 0);
		void fillChar(const Vector2& p, int32_t c, const bbe::Font& font, float rotation = 0);

		void fillChar(float x, float y, int32_t c, unsigned fontSize = DEFAULT_FONT_SIZE, const bbe::String& fontName = DEFAULT_FONT_NAME, float rotation = 0);
		void fillChar(const Vector2& p, int32_t c, unsigned fontSize = DEFAULT_FONT_SIZE, const bbe::String& fontName = DEFAULT_FONT_NAME, float rotation = 0);

		void fillText(float x, float y, const char* text, const bbe::Font& font, float rotation = 0);
		void fillText(const Vector2& p, const char* text, const bbe::Font& font, float rotation = 0);
		void fillText(float x, float y, const bbe::String &text, const bbe::Font& font, float rotation = 0);
		void fillText(const Vector2& p, const bbe::String& text, const bbe::Font& font, float rotation = 0);
		
		void fillText(float x, float y, const char* text, unsigned fontSize = DEFAULT_FONT_SIZE, const bbe::String& fontName = DEFAULT_FONT_NAME, float rotation = 0);
		void fillText(const Vector2& p, const char* text, unsigned fontSize = DEFAULT_FONT_SIZE, const bbe::String& fontName = DEFAULT_FONT_NAME, float rotation = 0);
		void fillText(float x, float y, const bbe::String& text, unsigned fontSize = DEFAULT_FONT_SIZE, const bbe::String& fontName = DEFAULT_FONT_NAME, float rotation = 0);
		void fillText(const Vector2& p, const bbe::String& text, unsigned fontSize = DEFAULT_FONT_SIZE, const bbe::String& fontName = DEFAULT_FONT_NAME, float rotation = 0);

		void setColorRGB(float r, float g, float b, float a);
		void setColorRGB(float r, float g, float b);
		void setColorRGB(const Vector3& c);
		void setColorRGB(const Vector3& c, float a);
		void setColorRGB(const Color &c);
		void setOutlineRGB(float r, float g, float b, float a);
		void setOutlineRGB(float r, float g, float b);
		void setOutlineRGB(const Vector3& c);
		void setOutlineRGB(const Vector3& c, float a);
		void setOutlineRGB(const Color& c);

		void setColorHSV(float h, float s, float v, float a);
		void setColorHSV(float h, float s, float v);
		void setOutlineHSV(float h, float s, float v, float a);
		void setOutlineHSV(float h, float s, float v);

		void setOffset(float x, float y);
		void setOffset(const bbe::Vector2& offset);
		bbe::Vector2 getOffset() const;

		void setOutlineWidth(float outlineWidht);

		void setFillMode(FillMode fm);
		FillMode getFillMode();

		void fillVertexIndexList(const bbe::List<uint32_t>& indices, const bbe::List<bbe::Vector2>& vertices);
		void fillVertexIndexList(const uint32_t *indices, uint32_t amountOfIndices, const bbe::Vector2 *vertices, size_t amountOfVertices);
	};
}
