#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Cube.h"

namespace bbe
{
	class Color;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanManager;
			class VulkanBuffer;
		}
	}

	enum DrawRecord
	{
		NONE, CUBE
	};

	class PrimitiveBrush3D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		VkCommandBuffer m_currentCommandBuffer = VK_NULL_HANDLE;
		VkDevice m_device                      = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice      = VK_NULL_HANDLE;
		VkPipelineLayout m_layout              = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet        = VK_NULL_HANDLE;
		int m_screenWidth;
		int m_screenHeight;

		DrawRecord m_lastDraw = NONE;

		Matrix4 m_modelMatrix;
		Matrix4 m_viewProjectionMatrix;

		INTERNAL::vulkan::VulkanBuffer m_uboMatrices;

		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice &device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int screenWidth, int screenHeight, VkDescriptorSet descriptorSet);
		
		void create(const INTERNAL::vulkan::VulkanDevice &vulkanDevice);
		void destroy();

	public:
		void fillCube(const Cube &cube);

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);

		void setCamera(const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUpVector = Vector3(0, 0, 1.0f));
	};
}