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
#include "../BBE/VulkanDescriptorSet.h"
#include "../BBE/VulkanDescriptorSetLayout.h"
#include "../BBE/VWDepthImage.h"
#include "../BBE/VulkanFence.h"
#include "../BBE/Stack.h"
#include "../BBE/Image.h"

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
				VulkanInstance          m_instance;
				VulkanSurface           m_surface;
				PhysicalDeviceContainer m_physicalDeviceContainer;
				VulkanDevice            m_device;
				VulkanSwapchain         m_swapchain;
				VulkanRenderPass        m_renderPass;

				VulkanShader   m_vertexShader2DPrimitive;
				VulkanShader   m_fragmentShader2DPrimitive;
				VulkanPipeline m_pipeline2DPrimitive;

				VulkanShader   m_vertexShader2DImage;
				VulkanShader   m_fragmentShader2DImage;
				VulkanPipeline m_pipeline2DImage;

				VulkanShader   m_vertexShader3DPrimitive;
				VulkanShader   m_fragmentShader3DPrimitive;
				VulkanShader   m_vertexShader3DTerrain;
				VulkanShader   m_fragmentShader3DTerrain;
				VulkanPipeline m_pipeline3DPrimitive;
				VulkanPipeline m_pipeline3DTerrain;


				VulkanBuffer   m_uboMatrixViewProjection;
				VulkanBuffer   m_uboMatrixModel;

				VulkanCommandPool         m_commandPool;
				VulkanSemaphore           m_semaphoreImageAvailable;
				VulkanSemaphore           m_semaphoreRenderingDone;
				VWDepthImage              m_depthImage;
				VkCommandBuffer           m_currentFrameDrawCommandBuffer = VK_NULL_HANDLE;
				VulkanFence               m_presentFence;
				VulkanDescriptorSetLayout m_setLayoutVertexLight;
				VulkanDescriptorSetLayout m_setLayoutFragmentLight;
				VulkanDescriptorSetLayout m_setLayoutViewProjectionMatrix;
				VulkanDescriptorSetLayout m_setLayoutSampler;
				VulkanDescriptorPool      m_descriptorPool;
				VulkanDescriptorSet       m_setVertexLight;
				VulkanDescriptorSet       m_setFragmentLight;
				VulkanDescriptorSet       m_setViewProjectionMatrixLight;
				GLFWwindow               *m_pwindow = nullptr;
				PrimitiveBrush2D          m_primitiveBrush2D;
				PrimitiveBrush3D          m_primitiveBrush3D;

				uint32_t m_screenWidth;
				uint32_t m_screenHeight;
				uint32_t m_imageIndex;

				Stack<VkBuffer>       m_pendingDestructionBuffers;
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
				void preDraw2D();
				void preDraw3D();
				void preDraw();
				void postDraw();

				bbe::PrimitiveBrush2D *getBrush2D();
				bbe::PrimitiveBrush3D *getBrush3D();

				void addPendingDestructionBuffer(VkBuffer buffer, VkDeviceMemory memory);
				void createPipelines();
				void resize(uint32_t width, uint32_t height);
				void recreateSwapchain();
			};
		}
	}
}
