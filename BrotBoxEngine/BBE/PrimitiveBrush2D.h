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

	class PrimitiveBrush2D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		VkCommandBuffer  m_currentCommandBuffer = VK_NULL_HANDLE;
		VkDevice         m_device               = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice       = VK_NULL_HANDLE;
		VkPipelineLayout m_layout               = VK_NULL_HANDLE;
		VkDescriptorSet  m_descriptorSet        = VK_NULL_HANDLE;
		int              m_screenWidth;
		int              m_screenHeight;

		void INTERNAL_fillRect(const Rectangle &rect);
		void INTERNAL_fillCircle(const Circle &circle);
		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice &device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int screenWidth, int screenHeight);

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
