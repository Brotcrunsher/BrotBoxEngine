#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanBuffer.h"

namespace bbe
{
	class Color;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanManager;
		}
	}


	class PrimitiveBrush3D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		VkCommandBuffer m_currentCommandBuffer;
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkPipelineLayout m_layout;
		VkDescriptorSet m_descriptorSet;
		int m_screenWidth;
		int m_screenHeight;

		Matrix4 m_modelMatrix;
		Matrix4 m_viewProjectionMatrix;

		INTERNAL::vulkan::VulkanBuffer m_uboModel;
		INTERNAL::vulkan::VulkanBuffer m_uboViewProjection;

		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice &device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int screenWidth, int screenHeight);
		
		void create(const INTERNAL::vulkan::VulkanDevice &vulkanDevice);
		void destroy();

	public:
		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);
	};
}