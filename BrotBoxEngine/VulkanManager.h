#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

#include "PrimitiveBrush2D.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevices.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanCommandPool.h"
#include "VulkanSemaphore.h"
#include "VWDepthImage.h"
#include "VulkanFence.h"
#include "Stack.h"

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
				VulkanShader m_vertexShader;
				VulkanShader m_fragmentShader;
				VulkanPipeline m_pipeline;
				VulkanCommandPool m_commandPool;
				VulkanSemaphore m_semaphoreImageAvailable;
				VulkanSemaphore m_semaphoreRenderingDone;
				VWDepthImage m_depthImage;
				VkCommandBuffer m_currentFrameDrawCommandBuffer = VK_NULL_HANDLE;
				VulkanFence m_presentFence;
				GLFWwindow *m_window = nullptr;
				bbe::PrimitiveBrush2D m_primitiveBrush2D;

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

				void addPendingDestructionBuffer(VkBuffer buffer, VkDeviceMemory memory);
			};
		}
	}
}