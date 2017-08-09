#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

#include "../BBE/PrimitiveBrush2D.h"
#include "../BBE/PrimitiveBrush3D.h"
#include "../BBE/VulkanInstance.h"
#include "../BBE/VulkanSurface.h"
#include "../BBE/VulkanPhysicalDevices.h"
#include "../BBE/VulkanDevice.h"
#include "../BBE/VulkanSwapchain.h"
#include "../BBE/VulkanRenderPass.h"
#include "../BBE/VulkanShader.h"
#include "../BBE/VulkanPipeline.h"
#include "../BBE/VulkanCommandPool.h"
#include "../BBE/VulkanSemaphore.h"
#include "../BBE/VulkanDescriptorPool.h"
#include "../BBE/VWDepthImage.h"
#include "../BBE/VulkanFence.h"
#include "../BBE/Stack.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager {
			public:
				static VulkanManager* s_pinstance;
			private:
				VulkanInstance m_instance;
				VulkanSurface m_surface;
				PhysicalDeviceContainer m_physicalDeviceContainer;
				VulkanDevice m_device;
				VulkanSwapchain m_swapchain;
				VulkanRenderPass m_renderPass;

				VulkanShader m_vertexShader2DPrimitive;
				VulkanShader m_fragmentShader2DPrimitive;
				VulkanPipeline m_pipeline2DPrimitive;

				VulkanShader m_vertexShader3DPrimitive;
				VulkanShader m_fragmentShader3DPrimitive;
				VulkanPipeline m_pipeline3DPrimitive;

				VulkanCommandPool m_commandPool;
				VulkanSemaphore m_semaphoreImageAvailable;
				VulkanSemaphore m_semaphoreRenderingDone;
				VWDepthImage m_depthImage;
				VkCommandBuffer m_currentFrameDrawCommandBuffer = VK_NULL_HANDLE;
				VulkanFence m_presentFence;
				VulkanDescriptorPool m_descriptorPool;
				GLFWwindow *m_window = nullptr;
				PrimitiveBrush2D m_primitiveBrush2D;
				PrimitiveBrush3D m_primitiveBrush3D;

				uint32_t m_screenWidth;
				uint32_t m_screenHeight;
				uint32_t m_imageIndex;

				Stack<VkBuffer> m_pendingDestructionBuffers;
				Stack<VkDeviceMemory> m_pendingDestructionMemory;
				void destroyPendingBuffers();

			public:
				VulkanManager();

				VulkanManager(const VulkanManager& other) = delete;
				VulkanManager(VulkanManager&& other) = delete;
				VulkanManager& operator=(const VulkanManager& other) = delete;
				VulkanManager& operator=(VulkanManager&& other) = delete;

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow *window, uint32_t initialWindowWidth, uint32_t initialWindowHeight);

				void destroy();
				void preDraw();
				void postDraw();

				bbe::PrimitiveBrush2D *getBrush2D();
				bbe::PrimitiveBrush3D *getBrush3D();

				void addPendingDestructionBuffer(VkBuffer buffer, VkDeviceMemory memory);
			};
		}
	}
}
