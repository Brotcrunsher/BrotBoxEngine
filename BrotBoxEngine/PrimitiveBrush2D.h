#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

#include "Rectangle.h"
#include "Color.h"

namespace bbe
{
	class VulkanDevice;

	class PrimitiveBrush2D
	{
	private:
		VkCommandBuffer m_currentCommandBuffer;
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkPipelineLayout m_layout;
		VkDescriptorSet m_descriptorSet;
		int m_screenWidth;
		int m_screenHeight;

		void INTERNAL_fillRect(Rectangle &rect);
		void INTERNAL_setColor(float r, float g, float b, float a);

	public:

		void INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice &device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int screenWidth, int screenHeight);

		void fillRect(Rectangle &rect);
		void fillRect(float x, float y, float width, float height);

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(Color &c);
	};
}