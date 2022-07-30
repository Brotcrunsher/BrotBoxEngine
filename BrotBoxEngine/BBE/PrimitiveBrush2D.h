#pragma once

#include "GLFW/glfw3.h"

#include "../BBE/Rectangle.h"
#include "../BBE/PhysRectangle.h"
#include "../BBE/PhysCircle.h"
#include "../BBE/Circle.h"
#include "../BBE/Color.h"
#include "../BBE/FillMode.h"
#include "../BBE/BezierCurve2.h"
#include "../BBE/Font.h"
#include "../BBE/Line2.h"
#include "../BBE/RenderManager.h"

namespace bbe
{
	class Image;
	class FragmentShader;
	class RectangleRotated;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanManager;
			class VulkanCommandPool;
			class VulkanDescriptorPool;
			class VulkanDescriptorSetLayout;
			class VulkanPipeline;
		}
	}

	enum class PipelineRecord2D
	{
		NONE, PRIMITIVE, IMAGE
	};

	class PrimitiveBrush2D
	{
// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
		friend class INTERNAL::vulkan::VulkanManager;
	private:

		INTERNAL::vulkan::VulkanDevice              *m_pdevice              = nullptr;
		INTERNAL::vulkan::VulkanCommandPool         *m_pcommandPool         = nullptr;
		INTERNAL::vulkan::VulkanDescriptorPool      *m_pdescriptorPool      = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayout = nullptr;
		VkCommandBuffer                              m_currentCommandBuffer = VK_NULL_HANDLE;
		VkPipelineLayout                             m_layoutPrimitive      = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelinePrimitive   = nullptr;
		VkPipelineLayout                             m_layoutImage          = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelineImage       = nullptr;
		VkDescriptorSet                              m_descriptorSet        = VK_NULL_HANDLE;
		float                                        m_windowXScale = 0;
		float                                        m_windowYScale = 0;
		int                                          m_screenWidth  = 0;
		int                                          m_screenHeight = 0;
		float                                        m_outlineWidth = 0;
		Color m_color = Color(-1000, -1000, -1000);
		Color m_outlineColor = Color(-1000, -1000, -1000);
		uint32_t m_imageIndex = 0xFFFFFFFF;
		bbe::Vector2 m_offset = {0, 0};

		bbe::RenderManager* m_prenderManager = nullptr;

		PipelineRecord2D   m_pipelineRecord = PipelineRecord2D::NONE;

		void INTERNAL_fillRect(const Rectangle &rect, float rotation, float outlineWidth, FragmentShader* shader);
		void INTERNAL_drawImage(const Rectangle &rect, const Image &image, float rotation);
		void INTERNAL_fillCircle(const Circle &circle, float outlineWidth);
		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(
			INTERNAL::vulkan::VulkanDevice &device,
			INTERNAL::vulkan::VulkanCommandPool &commandPool,
			INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout,
			VkCommandBuffer commandBuffer,
			INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive,
			INTERNAL::vulkan::VulkanPipeline &pipelineImage,
			GLFWwindow* window,
			int screenWidth, int screenHeight,
			uint32_t imageIndex,
			bbe::RenderManager *renderManager
		);

		void INTERNAL_init(const uint32_t amountOfFrames);
		void INTERNAL_destroy();

	public:
		static constexpr const char* DEFAULT_FONT_NAME = "arial.ttf";
		static constexpr unsigned DEFAULT_FONT_SIZE = 20;

		PrimitiveBrush2D();

		void fillRect(const Rectangle& rect, float rotation = 0, FragmentShader* shader = nullptr);
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
#endif
	};
}
