#include "stdafx.h"
#include "BBE/Vector2.h"
#include "BBE/VulkanManager.h"
#include "BBE/Color.h"
#include "BBE/Exceptions.h"

bbe::INTERNAL::vulkan::VulkanManager *bbe::INTERNAL::vulkan::VulkanManager::s_pinstance = nullptr;

void bbe::INTERNAL::vulkan::VulkanManager::destroyPendingBuffers()
{
	while (m_pendingDestructionBuffers.hasDataLeft())
	{
		vkDestroyBuffer(m_device.getDevice(), m_pendingDestructionBuffers.pop(), nullptr);
	}
	while (m_pendingDestructionMemory.hasDataLeft())
	{
		vkFreeMemory(m_device.getDevice(), m_pendingDestructionMemory.pop(), nullptr);
	}
}

bbe::INTERNAL::vulkan::VulkanManager::VulkanManager()
{
}

void bbe::INTERNAL::vulkan::VulkanManager::init(const char * appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow * window, uint32_t initialWindowWidth, uint32_t initialWindowHeight)
{
	if (s_pinstance != nullptr)
	{
		throw SingletonViolationException();
	}
	s_pinstance = this;

	m_screenWidth = initialWindowWidth;
	m_screenHeight = initialWindowHeight;

	m_window = window;
	m_instance.init(appName, major, minor, patch);
	m_surface.init(m_instance, m_window);
	m_physicalDeviceContainer.init(m_instance, m_surface);
	m_device.init(m_physicalDeviceContainer, m_surface);
	m_swapchain.init(m_surface, m_device, initialWindowWidth, initialWindowHeight, nullptr);
	m_renderPass.init(m_device);
	m_vertexShader.init(m_device, "vert.spv");
	m_fragmentShader.init(m_device, "frag.spv");
	m_pipeline.init(m_vertexShader, m_fragmentShader, initialWindowWidth, initialWindowHeight);

	VkVertexInputBindingDescription bindingDescription;
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vector2);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	m_pipeline.addVertexBinding(bindingDescription);

	VkVertexInputAttributeDescription viad;
	viad.location = 0;
	viad.binding = 0;
	viad.format = VK_FORMAT_R32G32_SFLOAT;
	viad.offset = 0;

	m_pipeline.addVertexDescription(viad);

	VkPushConstantRange pcr = {};
	pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pcr.offset = 0;
	pcr.size = sizeof(Color);

	m_pipeline.addPushConstantRange(pcr);

	m_pipeline.create(m_device.getDevice(), m_renderPass.getRenderPass());
	m_commandPool.init(m_device);
	m_depthImage.create(m_device, m_commandPool, initialWindowWidth, initialWindowHeight);
	m_swapchain.createFramebuffers(m_depthImage, m_renderPass);
	m_semaphoreImageAvailable.init(m_device);
	m_semaphoreRenderingDone.init(m_device);
	m_presentFence.init(m_device);
}

void bbe::INTERNAL::vulkan::VulkanManager::destroy()
{
	vkDeviceWaitIdle(m_device.getDevice());
	s_pinstance = nullptr;

	destroyPendingBuffers();
	m_presentFence.destroy();
	m_semaphoreRenderingDone.destroy();
	m_semaphoreImageAvailable.destroy();
	m_depthImage.destroy();
	m_commandPool.destroy();
	m_pipeline.destroy();
	m_fragmentShader.destroy();
	m_vertexShader.destroy();
	m_renderPass.destroy();
	m_swapchain.destroy();
	m_device.destroy();
	m_surface.destroy();
	m_instance.destroy();
}

void bbe::INTERNAL::vulkan::VulkanManager::preDraw()
{
	vkAcquireNextImageKHR(m_device.getDevice(), m_swapchain.getSwapchain(), std::numeric_limits<uint64_t>::max(), m_semaphoreImageAvailable.getSemaphore(), VK_NULL_HANDLE, &m_imageIndex);

	m_currentFrameDrawCommandBuffer = m_commandPool.getCommandBuffer();

	VkCommandBufferBeginInfo cbbi;
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.pNext = nullptr;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.pInheritanceInfo = nullptr;


	VkResult result = vkBeginCommandBuffer(m_currentFrameDrawCommandBuffer, &cbbi);
	ASSERT_VULKAN(result);

	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_renderPass.getRenderPass();
	renderPassBeginInfo.framebuffer = m_swapchain.getFrameBuffer(m_imageIndex);
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = { m_screenWidth, m_screenHeight };
	VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

	bbe::List<VkClearValue> clearValues = { clearValue };

	renderPassBeginInfo.clearValueCount = clearValues.getLength();
	renderPassBeginInfo.pClearValues = clearValues.getRaw();


	vkCmdBeginRenderPass(m_currentFrameDrawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.getPipeline());

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = m_screenWidth;
	viewport.height = m_screenHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_currentFrameDrawCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { m_screenWidth, m_screenHeight };
	vkCmdSetScissor(m_currentFrameDrawCommandBuffer, 0, 1, &scissor);

	m_primitiveBrush2D.INTERNAL_beginDraw(m_device, m_currentFrameDrawCommandBuffer, m_pipeline.getLayout(), m_screenWidth, m_screenHeight);
}

void bbe::INTERNAL::vulkan::VulkanManager::postDraw()
{
	vkCmdEndRenderPass(m_currentFrameDrawCommandBuffer);

	VkResult result = vkEndCommandBuffer(m_currentFrameDrawCommandBuffer);
	ASSERT_VULKAN(result);

	VkSemaphore semImAv = m_semaphoreImageAvailable.getSemaphore();
	VkSemaphore semReDo = m_semaphoreRenderingDone.getSemaphore();
	VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSwapchainKHR swapchain = m_swapchain.getSwapchain();
	VkQueue queue = m_device.getQueue();

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.pNext = nullptr;
	si.waitSemaphoreCount = 1;
	si.pWaitSemaphores = &(semImAv);
	si.pWaitDstStageMask = waitStageMask;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &m_currentFrameDrawCommandBuffer;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &semReDo;

	result = vkQueueSubmit(queue, 1, &si, m_presentFence.getFence());
	ASSERT_VULKAN(result);

	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &semReDo;
	pi.swapchainCount = 1;
	pi.pSwapchains = &swapchain;
	pi.pImageIndices = &m_imageIndex;
	pi.pResults = nullptr;

	result = vkQueuePresentKHR(m_device.getQueue(), &pi);
	ASSERT_VULKAN(result);
	m_presentFence.waitForFence();
	m_commandPool.freeCommandBuffer(m_currentFrameDrawCommandBuffer);
	destroyPendingBuffers();
}

bbe::PrimitiveBrush2D * bbe::INTERNAL::vulkan::VulkanManager::getBrush2D()
{
	return &m_primitiveBrush2D;
}

void bbe::INTERNAL::vulkan::VulkanManager::addPendingDestructionBuffer(VkBuffer buffer, VkDeviceMemory memory)
{
	m_pendingDestructionBuffers.push(buffer);
	m_pendingDestructionMemory.push(memory);
}

