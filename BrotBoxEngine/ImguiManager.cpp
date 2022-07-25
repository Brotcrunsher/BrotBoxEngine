// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include "BBE/ImguiManager.h"
#include "BBE/Vulkan/VulkanSurface.h"
#include "BBE/Vulkan/VulkanPhysicalDevices.h"
#include "BBE/Vulkan/VulkanInstance.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanDescriptorPool.h"
#include "BBE/Vulkan/VulkanSwapchain.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Vulkan/VulkanRenderPass.h"
#include "BBE/Vulkan/VulkanCommandPool.h"
#include "BBE/List.h"

#include "imgui_impl_glfw.h"

bbe::INTERNAL::vulkan::ImguiManager::ImguiManager()
{
}

void bbe::INTERNAL::vulkan::ImguiManager::destroy()
{
    if (m_initSuccessful)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }
}

void bbe::INTERNAL::vulkan::ImguiManager::startFrame()
{
    // TODO: Still not ideal - what if the scale is anything else than 1 or 2 (e.g. on 8k)
    float scale = 0;
    glfwGetWindowContentScale(m_window, &scale, nullptr);
    ImGuiIO& io = ImGui::GetIO();
    if (scale < 1.5f)
    {
        io.FontDefault = fontSmall;
    }
    else
    {
        io.FontDefault = fontBig;
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void bbe::INTERNAL::vulkan::ImguiManager::endFrame(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

static void CheckVkResultFn(VkResult err)
{
    ASSERT_VULKAN(err);
}

void bbe::INTERNAL::vulkan::ImguiManager::start(const VulkanInstance& instance, const VulkanCommandPool& commandPool, const VulkanDevice& device, const VulkanSurface& surface, const VulkanPhysicalDevice& physicalDevice, const VulkanDescriptorPool& descriptorPool, class VulkanRenderPass& renderPass, GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, false);

    ImGui_ImplVulkan_InitInfo implVulkanInitInfo = {};
    implVulkanInitInfo.Instance = instance.getInstance();
    implVulkanInitInfo.PhysicalDevice = physicalDevice.getDevice();
    implVulkanInitInfo.Device = device.getDevice();
    implVulkanInitInfo.QueueFamily = device.getQueueFamilyIndex();
    implVulkanInitInfo.Queue = device.getQueue();
    implVulkanInitInfo.PipelineCache = VK_NULL_HANDLE;
    implVulkanInitInfo.DescriptorPool = descriptorPool.getDescriptorPool();
    implVulkanInitInfo.Subpass = 0;
    implVulkanInitInfo.MinImageCount = m_minImageCount;
    implVulkanInitInfo.ImageCount = m_minImageCount;
    implVulkanInitInfo.Allocator = nullptr;
    implVulkanInitInfo.CheckVkResultFn = CheckVkResultFn;
    m_initSuccessful = ImGui_ImplVulkan_Init(&implVulkanInitInfo, renderPass.getRenderPass());
    if (!m_initSuccessful)
    {
        throw IllegalStateException();
    }

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig fontConfig;
    fontSmall = io.Fonts->AddFontDefault(&fontConfig);
    fontConfig.SizePixels = 26;
    fontBig   = io.Fonts->AddFontDefault(&fontConfig);

    VkCommandBuffer commandBuffer = INTERNAL::vulkan::startSingleTimeCommandBuffer(device.getDevice(), commandPool.getCommandPool());
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    INTERNAL::vulkan::endSingleTimeCommandBuffer(device.getDevice(), device.getQueue(), commandPool.getCommandPool(), commandBuffer);

    m_window = window;
}
#endif
