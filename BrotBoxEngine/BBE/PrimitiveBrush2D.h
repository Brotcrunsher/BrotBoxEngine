#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

#include "../BBE/Rectangle.h"
#include "../BBE/Circle.h"
#include "../BBE/Color.h"

namespace bbe
{
	

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanManager;
		}
	}

	enum class PipelineRecord
	{
		NONE, PRIMITIVE, IMAGE
	};

	class PrimitiveBrush2D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		VkCommandBuffer  m_currentCommandBuffer = VK_NULL_HANDLE;
		VkDevice         m_device               = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice       = VK_NULL_HANDLE;
		VkPipelineLayout m_layoutPrimitive      = VK_NULL_HANDLE;
		VkPipeline       m_pipelinePrimitive    = VK_NULL_HANDLE;
		VkPipelineLayout m_layoutImage          = VK_NULL_HANDLE;
		VkPipeline       m_pipelineImage        = VK_NULL_HANDLE;
		VkDescriptorSet  m_descriptorSet        = VK_NULL_HANDLE;
		int              m_screenWidth;
		int              m_screenHeight;

		PipelineRecord   m_pipelineRecord = PipelineRecord::NONE;

		void INTERNAL_fillRect(const Rectangle &rect);
		void INTERNAL_fillCircle(const Circle &circle);
		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice &device, VkCommandBuffer commandBuffer, VkPipeline m_pipelinePrimitive, VkPipelineLayout m_layoutPrimitive, int screenWidth, int screenHeight);

	public:


		void fillRect(const Rectangle &rect);
		void fillRect(float x, float y, float width, float height);

		void fillCircle(const Circle &circle);
		void fillCircle(float x, float y, float width, float height);

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);
	};
}
