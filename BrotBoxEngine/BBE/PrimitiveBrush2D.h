#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "../BBE/Rectangle.h"
#include "../BBE/Circle.h"
#include "../BBE/Color.h"
#include "../BBE/FillMode.h"
#include "../BBE/BezierCurve2.h"

namespace bbe
{
	class Image;

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

	enum class ShapeRecord2D
	{
		NONE, RECTANGLE, CIRCLE
	};

	class PrimitiveBrush2D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		INTERNAL::vulkan::VulkanDevice              *m_pdevice              = nullptr;
		INTERNAL::vulkan::VulkanCommandPool         *m_pcommandPool         = nullptr;
		INTERNAL::vulkan::VulkanDescriptorPool      *m_pdescriptorPool      = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayout = nullptr;
		VkCommandBuffer                              m_currentCommandBuffer = VK_NULL_HANDLE;
		VkPipelineLayout                             m_layoutPrimitive      = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelinePrimitive    = nullptr;
		VkPipelineLayout                             m_layoutImage          = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelineImage        = nullptr;
		VkDescriptorSet                              m_descriptorSet        = VK_NULL_HANDLE;
		int                                          m_screenWidth;
		int                                          m_screenHeight;

		PipelineRecord2D   m_pipelineRecord = PipelineRecord2D::NONE;
		ShapeRecord2D      m_shapeRecord    = ShapeRecord2D::NONE;

		void INTERNAL_fillRect(const Rectangle &rect, float rotation);
		void INTERNAL_drawImage(const Rectangle &rect, const Image &image);
		void INTERNAL_fillCircle(const Circle &circle);
		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(
			INTERNAL::vulkan::VulkanDevice &device,
			INTERNAL::vulkan::VulkanCommandPool &commandPool,
			INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout,
			VkCommandBuffer commandBuffer,
			INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive,
			INTERNAL::vulkan::VulkanPipeline &pipelineImage,
			int screenWidth, int screenHeight);

		FillMode m_fillMode = FillMode::SOLID;

	public:
		PrimitiveBrush2D();

		void fillRect(const Rectangle &rect, float rotation = 0);
		void fillRect(float x, float y, float width, float height, float rotation = 0);

		void fillCircle(const Circle &circle);
		void fillCircle(float x, float y, float width, float height);

		void drawImage(const Rectangle &rect, const Image &image);
		void drawImage(float x, float y, float width, float height, const Image &image);

		void fillLine(float x1, float y1, float x2, float y2, float lineWidth = 1);
		void fillLine(const Vector2 &p1, float x2, float y2, float lineWidth = 1);
		void fillLine(float x1, float y1, const Vector2 &p2, float lineWidth = 1);
		void fillLine(const Vector2 &p1, const Vector2 &p2, float lineWidth = 1);

		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const bbe::List<Vector2>& controlPoints);
		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint);
		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control);
		void fillBezierCurve(const Vector2& startPoint, const Vector2& endPoint, const Vector2& control1, const Vector2& control2);
		void fillBezierCurve(const BezierCurve2& bc, float lineWidth = 1);

		void setColorRGB(float r, float g, float b, float a);
		void setColorRGB(float r, float g, float b);
		void setColorRGB(const Vector3& c);
		void setColorRGB(const Color &c);

		void setColorHSV(float h, float s, float v, float a);
		void setColorHSV(float h, float s, float v);

		void setFillMode(FillMode fm);
		FillMode getFillMode();
	};
}
