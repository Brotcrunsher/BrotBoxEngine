#pragma once
// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN

#include "imgui_impl_vulkan.h"

struct GLFWwindow;

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanSurface;
			class VulkanPhysicalDevice;
			class VulkanInstance;
			class VulkanDevice;
			class VulkanDescriptorPool;
			class VulkanSwapchain;
			class VulkanRenderPass;
			class VulkanCommandPool;

			class ImguiManager
			{
			private:
				constexpr static uint32_t m_minImageCount = 2;
				bool m_initSuccessful = false;
				GLFWwindow* m_window = nullptr;

			public:
				ImguiManager();

				ImguiManager(const ImguiManager&)  = delete;
				ImguiManager(      ImguiManager&&) = delete;
				ImguiManager& operator=(const ImguiManager&)  = delete;
				ImguiManager& operator=(      ImguiManager&&) = delete;

				void start(const VulkanInstance& instance, const VulkanCommandPool& commandPool, const VulkanDevice& device, const VulkanSurface& surface, const VulkanPhysicalDevice& physicalDevice, const VulkanDescriptorPool& descriptorPool, class VulkanRenderPass& renderPass, GLFWwindow* window);
				void destroy();

				void startFrame();
				void endFrame(VkCommandBuffer commandBuffer);
			};
		}

		
	}
}
#endif

