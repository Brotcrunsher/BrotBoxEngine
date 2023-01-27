#include "BBE/Vector2.h"
#include "BBE/Vector3.h"
#include "BBE/Vulkan/VulkanManager.h"
#include "BBE/Color.h"
#include "BBE/Exceptions.h"
#include "BBE/Rectangle.h"
#include "BBE/EngineSettings.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/PointLight.h"
#include "BBE/Profiler.h"
#include "BBE/FragmentShader.h"
#include "BBE/Circle.h"
#include "BBE/Rectangle.h"
#include "BBE/IcoSphere.h"
#include "BBE/Vulkan/VulkanRectangle.h"
#include "BBE/Vulkan/VulkanCircle.h"
#include "BBE/Vulkan/VulkanCube.h"
#include "BBE/Vulkan/VulkanSphere.h"
#include "BBE/Vulkan/VulkanLight.h"
#include "EmbedOutput.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Third-Party/stb/stb_image_write.h"

#include "../Third-Party/jo/jo_mpeg.cpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui.h"

#include <future>
#include <mutex>
#include <fstream>
#include <iostream>

bbe::INTERNAL::vulkan::VulkanManager *bbe::INTERNAL::vulkan::VulkanManager::s_pinstance = nullptr;

bbe::INTERNAL::vulkan::VulkanManager::VulkanManager()
{
	m_screenWidth  = -1;
	m_screenHeight = -1;
	m_imageIndex   = -1;
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

	m_pwindow = window;
	std::cout << "Vulkan Manager: init instance" << std::endl;
	m_instance.init(appName, major, minor, patch);
	std::cout << "Vulkan Manager: init surface" << std::endl;
	m_surface.init(m_instance, m_pwindow);
	std::cout << "Vulkan Manager: init physicalDeviceContainer" << std::endl;
	m_physicalDeviceContainer.init(m_instance, m_surface);
	std::cout << "Vulkan Manager: init device" << std::endl;
	m_device.init(m_physicalDeviceContainer, m_surface);
	std::cout << "Vulkan Manager: init swapchain" << std::endl;
	m_swapchain.init(m_surface, m_device, initialWindowWidth, initialWindowHeight, nullptr);
	std::cout << "Vulkan Manager: init renderPass" << std::endl;
	m_renderPass.init(m_device);

	std::cout << "Vulkan Manager: init commandPool" << std::endl;
	m_commandPool.init(m_device);
	std::cout << "Vulkan Manager: creating depthImage" << std::endl;
	m_depthImage.create(m_device, m_commandPool, initialWindowWidth, initialWindowHeight);
	std::cout << "Vulkan Manager: creating Framebuffers" << std::endl;
	m_swapchain.createFramebuffers(m_depthImage, m_renderPass);
	std::cout << "Vulkan Manager: init semaphoreImageAvailable" << std::endl;
	m_semaphoreImageAvailable.init(m_device);
	std::cout << "Vulkan Manager: init semaphoreRenderingDone" << std::endl;
	m_semaphoreRenderingDone.init(m_device);
	std::cout << "Vulkan Manager: init presentFece" << std::endl;
	m_presentFence1.init(m_device);
	m_presentFence2.init(m_device);

	std::cout << "Vulkan Manager: creating 3DBrush" << std::endl;
	m_primitiveBrushes3D.resizeCapacityAndLength(m_swapchain.getAmountOfImages());
	m_uboMatrices       .resizeCapacityAndLength(m_swapchain.getAmountOfImages());
	for (uint32_t i = 0; i < m_swapchain.getAmountOfImages(); i++)
	{
		Matrix4 mat;
		m_uboMatrices[i].create(m_device, sizeof(Matrix4) * 2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		{
			void* data = m_uboMatrices[i].map();
			memcpy(data, &mat, sizeof(Matrix4));
			memcpy((char*)data + sizeof(Matrix4), &mat, sizeof(Matrix4));
			m_uboMatrices[i].unmap();
		}
	}
	bbe::INTERNAL::vulkan::VulkanLight::s_init(m_device.getDevice(), m_device.getPhysicalDevice());
	bbe::INTERNAL::vulkan::VulkanRectangle::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::INTERNAL::vulkan::VulkanCircle   ::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::INTERNAL::vulkan::VulkanCube     ::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::INTERNAL::vulkan::VulkanSphere   ::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());


	std::cout << "Vulkan Manager: Setting Bindings" << std::endl;
	m_setLayoutVertexLight.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
	m_setLayoutVertexLight.create(m_device);

	m_setLayoutFragmentLight.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	m_setLayoutFragmentLight.create(m_device);

	m_setLayoutViewProjectionMatrix.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
	m_setLayoutViewProjectionMatrix.create(m_device);

	m_setLayoutSampler.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	m_setLayoutSampler.create(m_device);

	m_setLayoutTerrainHeightMap.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
	m_setLayoutTerrainHeightMap.create(m_device);

	m_setLayoutTerrainAdditionalTexture.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Settings::getTerrainAdditionalTextures(), VK_SHADER_STAGE_FRAGMENT_BIT);
	m_setLayoutTerrainAdditionalTexture.create(m_device);

	m_setLayoutTerrainAdditionalTextureWeight.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Settings::getTerrainAdditionalTextures(), VK_SHADER_STAGE_FRAGMENT_BIT);
	m_setLayoutTerrainAdditionalTextureWeight.create(m_device);

	m_setLayoutTerrainViewFrustum.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
	m_setLayoutTerrainViewFrustum.create(m_device);

	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutVertexLight                   , 1);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutTerrainViewFrustum            , 16);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutFragmentLight                 , 1);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutViewProjectionMatrix          , 4);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutSampler                       , 1024);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutTerrainHeightMap              , 16);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutTerrainAdditionalTexture      , 1);
	m_descriptorPool.addVulkanDescriptorSetLayout(m_setLayoutTerrainAdditionalTextureWeight, 1);
	m_descriptorPool.create(m_device);

	m_setViewProjectionMatrixLights.resizeCapacityAndLength(m_swapchain.getAmountOfImages());
	for (uint32_t i = 0; i < m_setViewProjectionMatrixLights.getLength(); i++)
	{
		m_setViewProjectionMatrixLights[i].addUniformBuffer(m_uboMatrices[i], 0, 0);
		m_setViewProjectionMatrixLights[i].create(m_device, m_descriptorPool, m_setLayoutViewProjectionMatrix);
	}
	m_setVertexLight              .addUniformBuffer(bbe::INTERNAL::vulkan::VulkanLight::getVertexBuffer()  , 0, 0);
	m_setFragmentLight            .addUniformBuffer(bbe::INTERNAL::vulkan::VulkanLight::getFragmentBuffer(), 0, 0);
	m_setVertexLight              .create(m_device, m_descriptorPool, m_setLayoutVertexLight);
	m_setFragmentLight            .create(m_device, m_descriptorPool, m_setLayoutFragmentLight);

	std::cout << "Vulkan Manager: Loading Shaders" << std::endl;
	m_vertexShader2DPrimitive           .init(m_device, vert2DPrimitive);
	m_fragmentShader2DPrimitive         .init(m_device, frag2DPrimitive);
	m_fragmentShader2DImage             .init(m_device, frag2DImage);
	m_vertexShader3DPrimitive           .init(m_device, vert3DPrimitive);
	m_fragmentShader3DPrimitive         .init(m_device, frag3DPrimitive);
	m_vertexShader3DTerrain             .init(m_device, vert3DTerrain);
	m_fragmentShader3DTerrain           .init(m_device, frag3DTerrain);
	m_teseShader3DTerrain               .init(m_device, tese3DTerrain);
	m_tescShader3DTerrain               .init(m_device, tesc3DTerrain);

	std::cout << "Vulkan Manager: creating pipeline" << std::endl;
	createPipelines();

	//m_renderPassStopWatch.create(m_device);

	m_presentFence = &m_presentFence1;
	m_currentFrameDrawCommandBuffer = &m_currentFrameDrawCommandBuffer1;

	std::cout << "Vulkan Manager: Starting Dear ImGui" << std::endl;
	imguiStart();

	if (imageDatas.getLength() < m_swapchain.getAmountOfImages())
	{
		imageDatas.resizeCapacityAndLength(m_swapchain.getAmountOfImages());
	}
	if (m_delayedBufferDeletes.getLength() < m_swapchain.getAmountOfImages())
	{
		m_delayedBufferDeletes.resizeCapacityAndLength(m_swapchain.getAmountOfImages());
	}
}

void bbe::INTERNAL::vulkan::VulkanManager::destroy()
{
	for (size_t i = 0; i < screenshotFutures.getLength(); i++)
	{
		screenshotFutures[i].wait();
	}
	stopRecording();

	vkDeviceWaitIdle(m_device.getDevice());
	s_pinstance = nullptr;
	
	m_primitiveBrush2D.INTERNAL_destroy();

	for (size_t i = 0; i < m_delayedBufferDeletes.getLength(); i++)
	{
		for (size_t k = 0; k < m_delayedBufferDeletes[i].getLength(); k++)
		{
			vkDestroyBuffer(m_device.getDevice(), m_delayedBufferDeletes[i][k].m_buffer, nullptr);
			vkFreeMemory(m_device.getDevice(), m_delayedBufferDeletes[i][k].m_memory, nullptr);
		}
	}
	for (size_t i = 0; i < imageDatas.getLength(); i++)
	{
		for (size_t k = 0; k < imageDatas[i].getLength(); k++)
		{
			imageDatas[i][k]->decRef();
		}
	}

	imguiStop();

	//m_renderPassStopWatch.destroy();
	
	bbe::INTERNAL::vulkan::VulkanCube     ::s_destroy();
	bbe::INTERNAL::vulkan::VulkanCircle   ::s_destroy();
	bbe::INTERNAL::vulkan::VulkanRectangle::s_destroy();
	bbe::INTERNAL::vulkan::VulkanLight    ::s_destroy();
	bbe::INTERNAL::vulkan::VulkanSphere   ::s_destroy();

	m_presentFence1.destroy();
	m_presentFence2.destroy();
	m_semaphoreRenderingDone.destroy();
	m_semaphoreImageAvailable.destroy();
	m_depthImage.destroy();
	m_commandPool.destroy();

	m_pipeline3DPrimitive.destroy();
	m_pipeline3DTerrain.destroy();

	m_fragmentShader3DPrimitive.destroy();
	m_vertexShader3DPrimitive.destroy();
	m_vertexShader3DTerrain.destroy();
	m_fragmentShader3DTerrain.destroy();
	m_teseShader3DTerrain.destroy();
	m_tescShader3DTerrain.destroy();

	m_pipeline2DPrimitive.destroy();
	m_pipeline2DImage.destroy();
	m_fragmentShader2DPrimitive.destroy();
	m_vertexShader2DPrimitive.destroy();
	m_fragmentShader2DImage.destroy();

	m_setLayoutVertexLight.destroy();
	m_setLayoutFragmentLight.destroy();
	m_setLayoutViewProjectionMatrix.destroy();
	m_setLayoutSampler.destroy();
	m_setLayoutTerrainHeightMap.destroy();
	m_setLayoutTerrainAdditionalTexture.destroy();
	m_setLayoutTerrainViewFrustum.destroy();
	m_setLayoutTerrainAdditionalTextureWeight.destroy();
	m_descriptorPool.destroy();
	for (uint32_t i = 0; i < m_uboMatrices.getLength(); i++)
	{
		m_uboMatrices[i].destroy();
	}
	m_renderPass.destroy();
	m_swapchain.destroy();
	m_device.destroy();
	m_surface.destroy();
	m_instance.destroy();
}

void bbe::INTERNAL::vulkan::VulkanManager::preDraw2D()
{
	m_pipelineRecord2D = PipelineRecord2D::NONE;
	m_primitiveBrush2D.INTERNAL_beginDraw(
		m_pwindow,
		m_screenWidth, m_screenHeight,
		this);

	float pushConstants[] = { static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight) };
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 24, sizeof(float) * 2, pushConstants);

	for (size_t i = 0; i < imageDatas[m_imageIndex].getLength(); i++)
	{
		imageDatas[m_imageIndex][i]->decRef();
	}
	imageDatas[m_imageIndex].clear();

	for (size_t i = 0; i < m_delayedBufferDeletes[m_imageIndex].getLength(); i++)
	{
		vkDestroyBuffer(m_device.getDevice(), m_delayedBufferDeletes[m_imageIndex][i].m_buffer, nullptr);
		vkFreeMemory(m_device.getDevice(), m_delayedBufferDeletes[m_imageIndex][i].m_memory, nullptr);
	}
	m_delayedBufferDeletes[m_imageIndex].clear();
}

void bbe::INTERNAL::vulkan::VulkanManager::preDraw3D()
{
	m_primitiveBrushes3D[m_imageIndex].INTERNAL_beginDraw(
		m_screenWidth, m_screenHeight,
		this);

	bbe::INTERNAL::vulkan::VulkanLight::beginDraw();

	m_pipelineRecord3D = PipelineRecord3D::NONE;
	m_lastDraw3D = DrawRecord::NONE;
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 1, 1, m_setViewProjectionMatrixLights[m_imageIndex].getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
}

void bbe::INTERNAL::vulkan::VulkanManager::preDraw()
{
	if (videoFile)
	{
		// Urghs! Hacky!
		static bool first = true;
		if (!first)
		{
			saveVideoFrame();
		}
		first = false;
	}

	imguiStartFrame();

	vkAcquireNextImageKHR(m_device.getDevice(), m_swapchain.getSwapchain(), std::numeric_limits<uint64_t>::max(), m_semaphoreImageAvailable.getSemaphore(), VK_NULL_HANDLE, &m_imageIndex);

	*m_currentFrameDrawCommandBuffer = m_commandPool.getCommandBuffer();


	VkCommandBufferBeginInfo cbbi;
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.pNext = nullptr;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.pInheritanceInfo = nullptr;


	VkResult result = vkBeginCommandBuffer(*m_currentFrameDrawCommandBuffer, &cbbi);
	ASSERT_VULKAN(result);

	//m_renderPassStopWatch.arm(*m_currentFrameDrawCommandBuffer);

	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_renderPass.getRenderPass();
	renderPassBeginInfo.framebuffer = m_swapchain.getFrameBuffer(m_imageIndex);
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = { m_screenWidth, m_screenHeight };
	VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
	//VkClearValue clearValue = { 1.0f, 20.f/255.f, 147.f/255.f, 1.0f };
	VkClearValue depthClearValue = { 1.0f, 0 };

	bbe::List<VkClearValue> clearValues = { 
		clearValue,
		depthClearValue
	};

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.getLength());
	renderPassBeginInfo.pClearValues = clearValues.getRaw();


	vkCmdBeginRenderPass(*m_currentFrameDrawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_screenWidth;
	viewport.height = (float)m_screenHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(*m_currentFrameDrawCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { m_screenWidth, m_screenHeight };
	vkCmdSetScissor(*m_currentFrameDrawCommandBuffer, 0, 1, &scissor);
}

void bbe::INTERNAL::vulkan::VulkanManager::postDraw()
{
	imguiEndFrame();

	vkCmdEndRenderPass(*m_currentFrameDrawCommandBuffer);

	//m_renderPassStopWatch.end(*m_currentFrameDrawCommandBuffer);

	VkResult result = vkEndCommandBuffer(*m_currentFrameDrawCommandBuffer);
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
	si.pCommandBuffers = m_currentFrameDrawCommandBuffer;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &semReDo;

	result = vkQueueSubmit(queue, 1, &si, m_presentFence->getFence());
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
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
	}
	else
	{
		ASSERT_VULKAN(result);
	}
}

void bbe::INTERNAL::vulkan::VulkanManager::waitEndDraw()
{
	static bool firstFrame = true;
	if (m_presentFence == &m_presentFence1)
	{
		m_presentFence = &m_presentFence2;
		m_currentFrameDrawCommandBuffer = &m_currentFrameDrawCommandBuffer2;
	}
	else
	{
		m_presentFence = &m_presentFence1;
		m_currentFrameDrawCommandBuffer = &m_currentFrameDrawCommandBuffer1;
	}
	if (!firstFrame)
	{
		m_presentFence->waitForFence();
		m_commandPool.freeCommandBuffer(*m_currentFrameDrawCommandBuffer);
	}

	//m_renderPassStopWatch.finish(m_commandPool, m_device.getQueue());
	//bbe::Profiler::INTERNAL::setRenderTime(m_renderPassStopWatch.getTimePassed() * m_device.m_properties.limits.timestampPeriod / 1000.f / 1000.f / 1000.f);
	firstFrame = false;
}

void bbe::INTERNAL::vulkan::VulkanManager::waitTillIdle()
{
	m_device.waitIdle();
}

bool bbe::INTERNAL::vulkan::VulkanManager::isReadyToDraw() const
{
	return m_swapchain.isInit();
}

bbe::PrimitiveBrush2D& bbe::INTERNAL::vulkan::VulkanManager::getBrush2D()
{
	return m_primitiveBrush2D;
}

bbe::PrimitiveBrush3D& bbe::INTERNAL::vulkan::VulkanManager::getBrush3D()
{
	return m_primitiveBrushes3D[m_imageIndex];
}

void bbe::INTERNAL::vulkan::VulkanManager::createPipelines()
{
	m_pipeline2DPrimitive.init(m_vertexShader2DPrimitive, m_fragmentShader2DPrimitive, m_screenWidth, m_screenHeight);
	m_pipeline2DPrimitive.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline2DPrimitive.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline2DPrimitive.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	m_pipeline2DPrimitive.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64);
	m_pipeline2DPrimitive.create(m_device.getDevice(), m_renderPass.getRenderPass());

	m_pipeline2DImage.init(m_vertexShader2DPrimitive, m_fragmentShader2DImage, m_screenWidth, m_screenHeight);
	m_pipeline2DImage.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline2DImage.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline2DImage.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	m_pipeline2DImage.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 64);
	m_pipeline2DImage.addDescriptorSetLayout(m_setLayoutSampler.getDescriptorSetLayout());
	m_pipeline2DImage.create(m_device.getDevice(), m_renderPass.getRenderPass());


	m_pipeline3DPrimitive.init(m_vertexShader3DPrimitive, m_fragmentShader3DPrimitive, m_screenWidth, m_screenHeight);
	m_pipeline3DPrimitive.addVertexBinding(0, sizeof(VertexWithNormal), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline3DPrimitive.addVertexDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexWithNormal, m_pos));
	m_pipeline3DPrimitive.addVertexDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexWithNormal, m_normal));
	m_pipeline3DPrimitive.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color));
	m_pipeline3DPrimitive.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(Matrix4));
	m_pipeline3DPrimitive.addDescriptorSetLayout(m_setLayoutVertexLight.getDescriptorSetLayout());
	m_pipeline3DPrimitive.addDescriptorSetLayout(m_setLayoutViewProjectionMatrix.getDescriptorSetLayout());
	m_pipeline3DPrimitive.addDescriptorSetLayout(m_setLayoutFragmentLight.getDescriptorSetLayout());
	m_pipeline3DPrimitive.enableDepthBuffer();
	m_pipeline3DPrimitive.addSpezializationConstant(0, 0, sizeof(int32_t));
	int32_t spezAmountOfLights = Settings::getAmountOfLightSources();
	m_pipeline3DPrimitive.setSpezializationData(sizeof(int32_t), &spezAmountOfLights);
	m_pipeline3DPrimitive.create(m_device.getDevice(), m_renderPass.getRenderPass());

	m_pipeline3DTerrain.init(m_vertexShader3DTerrain, m_fragmentShader3DTerrain, m_screenWidth, m_screenHeight);
	m_pipeline3DTerrain.setTessellationShader(m_tescShader3DTerrain.getModule(), m_teseShader3DTerrain.getModule(), 4);
	m_pipeline3DTerrain.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline3DTerrain.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline3DTerrain.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color) + sizeof(Vector4));
	m_pipeline3DTerrain.addPushConstantRange(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(Color) + sizeof(Vector4), sizeof(Matrix4) + sizeof(float));
	m_pipeline3DTerrain.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(float) * 5);
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutVertexLight.getDescriptorSetLayout());
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutViewProjectionMatrix.getDescriptorSetLayout());
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutFragmentLight.getDescriptorSetLayout());
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutTerrainHeightMap.getDescriptorSetLayout());
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutSampler.getDescriptorSetLayout());
	//m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutTerrainViewFrustum.getDescriptorSetLayout());
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTexture.getDescriptorSetLayout());
	m_pipeline3DTerrain.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTextureWeight.getDescriptorSetLayout());
	m_pipeline3DTerrain.enableDepthBuffer();
	m_pipeline3DTerrain.addSpezializationConstant(0, 0, sizeof(int32_t));
	m_pipeline3DTerrain.setSpezializationData(sizeof(int32_t), &spezAmountOfLights);
	m_pipeline3DTerrain.enablePrimitiveRestart(false);
	m_pipeline3DTerrain.setPrimitiveTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	m_pipeline3DTerrain.create(m_device.getDevice(), m_renderPass.getRenderPass());
}

void bbe::INTERNAL::vulkan::VulkanManager::resize(uint32_t width, uint32_t height)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.getPhysicalDevice(), m_surface.getSurface(), &surfaceCapabilities);

	if (width > surfaceCapabilities.maxImageExtent.width) width = surfaceCapabilities.maxImageExtent.width;
	if (height > surfaceCapabilities.maxImageExtent.height) height = surfaceCapabilities.maxImageExtent.height;

	if (width == 0 || height == 0) return; //Do nothing!


	m_screenWidth = width;
	m_screenHeight = height;

	recreateSwapchain();
}

void bbe::INTERNAL::vulkan::VulkanManager::recreateSwapchain()
{
	m_device.waitIdle();
//	if (useIconifyRestoreWorkaround)
//	{
//		//This is very hacky. It can happen sometimes that some commands like vkQueuePresentKHR return
//		//VK_ERROR_OUT_OF_DATE_KHR. Such behavior was observed when the window that is created is bigger
//		//than the resolution of the monitor on which the window is displayed. In such a case it is not
//		//sufficient to just recreate the swap chain but we also have to quickly iconify and restore the
//		//window or else it is just completely white. It would be much better if another solution were
//		//found to avoid this issue. Unfortunately, I don't know if this is a bug in GLFW or BBE (probably
//		//the later).
//		glfwIconifyWindow(m_pwindow);
//		glfwRestoreWindow(m_pwindow);
//	}

	m_pipeline2DPrimitive.destroy();
	m_pipeline2DImage.destroy();
	m_pipeline3DPrimitive.destroy();
	m_pipeline3DTerrain.destroy();
	m_renderPass.destroy();
	m_depthImage.destroy();
	//m_swapchain.destroy();

	VulkanSwapchain newChain;
	newChain.init(m_surface, m_device, m_screenWidth, m_screenHeight, &m_swapchain);
	m_renderPass.init(m_device);
	m_depthImage.create(m_device, m_commandPool, m_screenWidth, m_screenHeight);
	newChain.createFramebuffers(m_depthImage, m_renderPass);
	createPipelines();

	m_swapchain.destroy();
	m_swapchain = newChain;

	if (imageDatas.getLength() < m_swapchain.getAmountOfImages())
	{
		imageDatas.resizeCapacityAndLength(m_swapchain.getAmountOfImages());
	}
}

bbe::INTERNAL::vulkan::VulkanDevice& bbe::INTERNAL::vulkan::VulkanManager::getVulkanDevice()
{
	return m_device;
}

bbe::INTERNAL::vulkan::VulkanRenderPass& bbe::INTERNAL::vulkan::VulkanManager::getVulkanRenderPass()
{
	return m_renderPass;
}

bbe::INTERNAL::vulkan::VulkanShader& bbe::INTERNAL::vulkan::VulkanManager::getVertexShader2DPrimitive()
{
	return m_vertexShader2DPrimitive;
}

static void insertImageMemoryBarrier(
	VkCommandBuffer cmdbuffer,
	VkImage image,
	VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask,
	VkImageSubresourceRange subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

static void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, VkDevice device)
{
	if (commandBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	ASSERT_VULKAN(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	ASSERT_VULKAN(vkCreateFence(device, &fenceInfo, nullptr, &fence));

	ASSERT_VULKAN(vkQueueSubmit(queue, 1, &submitInfo, fence));

	ASSERT_VULKAN(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000));
	vkDestroyFence(device, fence, nullptr);
	vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
}

bbe::INTERNAL::vulkan::VulkanManager::ScreenshotFirstStage bbe::INTERNAL::vulkan::VulkanManager::getRawScreenshot()
{
	ScreenshotFirstStage screenshotFirstStage = {};
	screenshotFirstStage.device = m_device.getDevice();
	screenshotFirstStage.subResourceLayout;
	screenshotFirstStage.format = m_device.getFormat();
	screenshotFirstStage.height = m_screenHeight;
	screenshotFirstStage.width = m_screenWidth;

	VkImage srcImage = m_swapchain.getImage(m_imageIndex);

	VkImageCreateInfo imageCreateCI = {};
	imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
	imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateCI.extent.width = m_screenWidth;
	imageCreateCI.extent.height = m_screenHeight;
	imageCreateCI.extent.depth = 1;
	imageCreateCI.arrayLayers = 1;
	imageCreateCI.mipLevels = 1;
	imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
	imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ASSERT_VULKAN(vkCreateImage(m_device.getDevice(), &imageCreateCI, nullptr, &screenshotFirstStage.dstImage));
	VkMemoryRequirements memRequirements;
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkGetImageMemoryRequirements(m_device.getDevice(), screenshotFirstStage.dstImage, &memRequirements);
	memAllocInfo.allocationSize = memRequirements.size;
	memAllocInfo.memoryTypeIndex = m_physicalDeviceContainer.findBestDevice(m_surface).getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	ASSERT_VULKAN(vkAllocateMemory(m_device.getDevice(), &memAllocInfo, nullptr, &screenshotFirstStage.dstImageMemory));
	ASSERT_VULKAN(vkBindImageMemory(m_device.getDevice(), screenshotFirstStage.dstImage, screenshotFirstStage.dstImageMemory, 0));

	VkCommandBuffer copyCmd = startSingleTimeCommandBuffer(m_device.getDevice(), m_commandPool.getCommandPool());

	insertImageMemoryBarrier(
		copyCmd,
		screenshotFirstStage.dstImage,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	insertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });


	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width = m_screenWidth;
	imageCopyRegion.extent.height = m_screenHeight;
	imageCopyRegion.extent.depth = 1;

	vkCmdCopyImage(
		copyCmd,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		screenshotFirstStage.dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);

	insertImageMemoryBarrier(
		copyCmd,
		screenshotFirstStage.dstImage,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	insertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	flushCommandBuffer(copyCmd, m_device.getQueue(), m_commandPool.getCommandPool(), m_device.getDevice());

	VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	vkGetImageSubresourceLayout(m_device.getDevice(), screenshotFirstStage.dstImage, &subResource, &screenshotFirstStage.subResourceLayout);

	return screenshotFirstStage;
}

static void writeThreadScreenshot(const bbe::String /*copy*/ path, bbe::INTERNAL::vulkan::VulkanManager::ScreenshotFirstStage screenshotFirstStage)
{
	bool requiresSwizzle = false;
	unsigned char* rawScreenshot = screenshotFirstStage.toPixelData(&requiresSwizzle);
	if (requiresSwizzle)
	{
		unsigned char* swizzleHead = rawScreenshot;
		for (size_t i = 0; i < screenshotFirstStage.width * screenshotFirstStage.height; i++)
		{
			unsigned char temp = swizzleHead[0];
			swizzleHead[0] = swizzleHead[2];
			swizzleHead[2] = temp;

			swizzleHead += 4;
		}
	}

	unsigned char* alphaHead = rawScreenshot;
	for (size_t i = 0; i < screenshotFirstStage.width * screenshotFirstStage.height; i++)
	{
		alphaHead[3] = 255;
		alphaHead += 4;
	}

	stbi_write_png(path.getRaw(), screenshotFirstStage.width, screenshotFirstStage.height, 4, rawScreenshot, 0);
	delete[] rawScreenshot;
}

void bbe::INTERNAL::vulkan::VulkanManager::screenshot(const bbe::String& path)
{
	while (screenshotFutures.getLength() > 16)
	{
		screenshotFutures[0].wait();
		screenshotFutures.removeIndex(0);
	}

	ScreenshotFirstStage screenshotFirstStage = getRawScreenshot();
	// The future has to be stored in a variable that is not destroyed (at least temporarily). If it doesn't then
	// (in some situations) the compiler has to wait in the destructor of the future until the thread ends.
	// For more info, see: https://scottmeyers.blogspot.com/2013/03/stdfutures-from-stdasync-arent-special.html
	screenshotFutures.add(std::async(std::launch::async, &writeThreadScreenshot, path, screenshotFirstStage));
}

static void writeThreadVideo(bbe::INTERNAL::vulkan::VulkanManager::ScreenshotFirstStage screenshotFirstStage, FILE *videoFile, std::shared_future<void> previousFrameFuture)
{
	bool requiresSwizzle = false;
	const unsigned char* rawScreenshot = screenshotFirstStage.toPixelData(&requiresSwizzle);

	constexpr size_t bufferSize = 1024 * 1024 * 64;
	char* buffer = new char[bufferSize];
	size_t amount = 0;
	jo_write_mpeg(buffer, bufferSize, amount, rawScreenshot, screenshotFirstStage.width, screenshotFirstStage.height, 60, requiresSwizzle);
	delete[] rawScreenshot;

	if (previousFrameFuture.valid())
	{
		previousFrameFuture.wait();
	}
	static std::mutex m;

	{
		std::lock_guard lg(m);
		fwrite(buffer, 1, amount, videoFile);
	}

	delete[] buffer;
}

void bbe::INTERNAL::vulkan::VulkanManager::saveVideoFrame()
{
	while (videoFutures.getLength() > 128)
	{
		videoFutures[0].wait();
		videoFutures.removeIndex(0);
	}

	ScreenshotFirstStage screenshotFirstStage = getRawScreenshot();
	// The future has to be stored in a variable that is not destroyed (at least temporarily). If it doesn't then
	// (in some situations) the compiler has to wait in the destructor of the future until the thread ends.
	// For more info, see: https://scottmeyers.blogspot.com/2013/03/stdfutures-from-stdasync-arent-special.html
	static std::shared_future<void> previousFrameFuture;
	std::shared_future<void> currentFrameFuture = std::async(std::launch::async, &writeThreadVideo, screenshotFirstStage, videoFile, previousFrameFuture);
	videoFutures.add(currentFrameFuture);
	previousFrameFuture = currentFrameFuture;
}

void bbe::INTERNAL::vulkan::VulkanManager::setVideoRenderingMode(const char* path)
{
	stopRecording();
	videoFile = fopen(path, "wb");
}

void bbe::INTERNAL::vulkan::VulkanManager::stopRecording()
{
	if (videoFile)
	{
		for (size_t i = 0; i < videoFutures.getLength(); i++)
		{
			videoFutures[i].wait();
		}
		fclose(videoFile);
	}
}

void bbe::INTERNAL::vulkan::VulkanManager::bindRectBuffers()
{
	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = bbe::INTERNAL::vulkan::VulkanRectangle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(*m_currentFrameDrawCommandBuffer, 0, 1, &buffer, offsets);

	buffer = INTERNAL::vulkan::VulkanRectangle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(*m_currentFrameDrawCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void bbe::INTERNAL::vulkan::VulkanManager::bindPipelinePrimitive3D()
{
	if (m_pipelineRecord3D != PipelineRecord3D::PRIMITIVE)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getPipeline(getFillMode3D()));
		vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
		vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getLayout(), 1, 1, m_setViewProjectionMatrixLights[m_imageIndex].getPDescriptorSet(), 0, nullptr);
		vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
		m_pipelineRecord3D = PipelineRecord3D::PRIMITIVE;
	}
}

void bbe::INTERNAL::vulkan::VulkanManager::bindPipelineTerrain3D()
{
	if (m_pipelineRecord3D != PipelineRecord3D::TERRAIN)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getPipeline(getFillMode3D()));
		vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
		vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 1, 1, m_setViewProjectionMatrixLights[m_imageIndex].getPDescriptorSet(), 0, nullptr);
		vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
		m_pipelineRecord3D = PipelineRecord3D::TERRAIN;
	}
}

void bbe::INTERNAL::vulkan::VulkanManager::setColor2D(const bbe::Color& color)
{
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 64, sizeof(Color), &color);
}

void bbe::INTERNAL::vulkan::VulkanManager::fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader)
{
	if (shader != nullptr)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->INTERNAL_getPipeline().getPipeline(getFillMode2D()));
		vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 80, 48, shader->getPushConstants());
	}
	else if (m_pipelineRecord2D != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline2DPrimitive.getPipeline(getFillMode2D()));
		m_pipelineRecord2D = PipelineRecord2D::PRIMITIVE;
	}

	float pushConstants[] = {
		rect.getX(),
		rect.getY(),
		rect.getWidth(),
		rect.getHeight(),
		rotation
	};
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);

	bindRectBuffers();

	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::INTERNAL::vulkan::VulkanManager::fillCircle2D(const Circle& circle)
{
	if (m_pipelineRecord2D != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline2DPrimitive.getPipeline(getFillMode2D()));
		m_pipelineRecord2D = PipelineRecord2D::PRIMITIVE;
	}
	float pushConstants[] = {
		circle.getX(),
		circle.getY(),
		circle.getWidth(),
		circle.getHeight(),
		0
	};

	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = INTERNAL::vulkan::VulkanCircle::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(*m_currentFrameDrawCommandBuffer, 0, 1, &buffer, offsets);

	buffer = INTERNAL::vulkan::VulkanCircle::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(*m_currentFrameDrawCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, (INTERNAL::vulkan::VulkanCircle::AMOUNTOFVERTICES - 2) * 3, 1, 0, 0, 0);
}

void bbe::INTERNAL::vulkan::VulkanManager::drawImage2D(const Rectangle& rect, const Image& image, float rotation)
{
	if (m_pipelineRecord2D != PipelineRecord2D::IMAGE)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline2DImage.getPipeline(getFillMode2D()));
		m_pipelineRecord2D = PipelineRecord2D::IMAGE;
	}

	bbe::INTERNAL::vulkan::VulkanImage* vi = nullptr;
	if (image.m_prendererData == nullptr)
	{
		// The image is cleaning up this VulkanImage
		vi = new bbe::INTERNAL::vulkan::VulkanImage(image, m_device, m_commandPool, m_descriptorPool, m_setLayoutSampler);
	}
	else
	{
		vi = (bbe::INTERNAL::vulkan::VulkanImage*)image.m_prendererData;
	}

	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline2DImage.getLayout(), 0, 1, vi->getDescriptorSet().getPDescriptorSet(), 0, nullptr);

	imageDatas[m_imageIndex].add(vi);
	vi->incRef();

	float pushConstants[] = {
		rect.getX(),
		rect.getY(),
		rect.getWidth(),
		rect.getHeight(),
		rotation
	};
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);

	bindRectBuffers();

	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, 6, 1, 0, 0, 0);
}

void bbe::INTERNAL::vulkan::VulkanManager::fillVertexIndexList2D(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale)
{
	// These two asserts make sure that we can cast the vertices array to a float array.
	static_assert(alignof(bbe::Vector2) == alignof(float));
	static_assert(sizeof(bbe::Vector2) == 2 * sizeof(float));

	bbe::INTERNAL::vulkan::VulkanBuffer indexBuffer;
	bbe::INTERNAL::vulkan::VulkanBuffer vertexBuffer;
	indexBuffer.create(m_device.getDevice(), m_device.getPhysicalDevice(), sizeof(uint32_t) * amountOfIndices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* indexDataBuf = indexBuffer.map();
	memcpy(indexDataBuf, indices, sizeof(uint32_t) * amountOfIndices);
	indexBuffer.unmap();

	vertexBuffer.create(m_device.getDevice(), m_device.getPhysicalDevice(), sizeof(Vector2) * amountOfVertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	void* vertexDataBuf = vertexBuffer.map();
	memcpy(vertexDataBuf, (float*)vertices, sizeof(Vector2) * amountOfVertices);
	vertexBuffer.unmap();

	if (m_pipelineRecord2D != PipelineRecord2D::PRIMITIVE)
	{
		vkCmdBindPipeline(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline2DPrimitive.getPipeline(getFillMode2D()));
		m_pipelineRecord2D = PipelineRecord2D::PRIMITIVE;
	}

	float pushConstants[] = {
		pos.x * scale.x,
		pos.y * scale.y,
		scale.x,
		scale.y,
		0
	};
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline2DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 5, pushConstants);

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(*m_currentFrameDrawCommandBuffer, 0, 1, &buffer, offsets);

	buffer = indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(*m_currentFrameDrawCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, amountOfIndices, 1, 0, 0, 0);

	m_delayedBufferDeletes[m_imageIndex].add({ indexBuffer.getBuffer(), indexBuffer.getMemory() });
	m_delayedBufferDeletes[m_imageIndex].add({ vertexBuffer.getBuffer(), vertexBuffer.getMemory() });
}

void bbe::INTERNAL::vulkan::VulkanManager::setColor3D(const bbe::Color& color)
{
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DPrimitive.getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &color);
}

void bbe::INTERNAL::vulkan::VulkanManager::setCamera3D(const bbe::Vector3& cameraPos, const bbe::Matrix4& m_view, const bbe::Matrix4& m_projection)
{
	void* data = m_uboMatrices[m_imageIndex].map();
	memcpy((char*)data, &m_view, sizeof(Matrix4));
	memcpy((char*)data + sizeof(Matrix4), &m_projection, sizeof(Matrix4));
	m_uboMatrices[m_imageIndex].unmap();
}

void bbe::INTERNAL::vulkan::VulkanManager::fillCube3D(const Cube& cube)
{
	bindPipelinePrimitive3D();
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 4, sizeof(Matrix4), &cube.getTransform());

	if (m_lastDraw3D != DrawRecord::CUBE)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = VulkanCube::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(*m_currentFrameDrawCommandBuffer, 0, 1, &buffer, offsets);

		buffer = VulkanCube::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(*m_currentFrameDrawCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

		m_lastDraw3D = DrawRecord::CUBE;
	}


	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, VulkanCube::amountOfIndices, 1, 0, 0, 0);
}

void bbe::INTERNAL::vulkan::VulkanManager::fillSphere3D(const bbe::IcoSphere& sphere)
{
	bindPipelinePrimitive3D();
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DPrimitive.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 4, sizeof(Matrix4), &sphere.m_transform);

	if (m_lastDraw3D != DrawRecord::ICOSPHERE)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = VulkanSphere::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(*m_currentFrameDrawCommandBuffer, 0, 1, &buffer, offsets);

		buffer = VulkanSphere::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(*m_currentFrameDrawCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

		m_lastDraw3D = DrawRecord::ICOSPHERE;
	}


	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, VulkanSphere::amountOfIndices, 1, 0, 0, 0);
}

void bbe::INTERNAL::vulkan::VulkanManager::drawTerrain(const bbe::Terrain& terrain, const bbe::Color& color)
{
	terrain.init(
		m_device,
		m_commandPool,
		m_descriptorPool,
		m_setLayoutTerrainHeightMap,
		m_setLayoutSampler,
		m_setLayoutTerrainAdditionalTexture,
		m_setLayoutTerrainAdditionalTextureWeight,
		m_setLayoutTerrainViewFrustum
	);

	bindPipelineTerrain3D();
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 3, 1, ((bbe::INTERNAL::vulkan::VulkanImage*)terrain.m_heightMap.m_prendererData)->getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 4, 1, ((bbe::INTERNAL::vulkan::VulkanImage*)terrain.m_baseTexture.m_prendererData)->getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	//vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 5, 1, terrain.m_viewFrustrumDescriptor.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 5, 1, ((bbe::INTERNAL::vulkan::VulkanImage*)terrain.m_additionalTextures[0].m_prendererData)->getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(*m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 6, 1, ((bbe::INTERNAL::vulkan::VulkanImage*)terrain.m_additionalTextureWeights[0].m_prendererData)->getDescriptorSet().getPDescriptorSet(), 0, nullptr);


	class PushConts
	{
	public:
		Matrix4 mat;
		float height;
	}pushConts;

	class PushContsFragmentShader
	{
	public:
		Color c;
		Vector4 bias;
	} pushContsFragmenShader;

	pushContsFragmenShader.c = color;
	pushContsFragmenShader.bias.x = terrain.m_baseTextureBias.m_textureMult.x;
	pushContsFragmenShader.bias.y = terrain.m_baseTextureBias.m_textureMult.y;
	pushContsFragmenShader.bias.z = terrain.m_baseTextureBias.m_textureOffset.x;
	pushContsFragmenShader.bias.w = terrain.m_baseTextureBias.m_textureOffset.y;

	pushConts.height = terrain.getMaxHeight();
	pushConts.mat = terrain.m_transform;
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DTerrain.getLayout(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(PushContsFragmentShader), sizeof(PushConts), &(pushConts));
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DTerrain.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 108, sizeof(float), &terrain.m_patchSize);

	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DTerrain.getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushContsFragmentShader), &(pushContsFragmenShader));

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = terrain.m_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(*m_currentFrameDrawCommandBuffer, 0, 1, &buffer, offsets);

	buffer = terrain.m_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(*m_currentFrameDrawCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DTerrain.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 112, sizeof(Vector2), &terrain.m_heightmapScale);
	Vector2 emptyOffset(0, 0);
	vkCmdPushConstants(*m_currentFrameDrawCommandBuffer, m_pipeline3DTerrain.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(Vector2), &emptyOffset);
	vkCmdDrawIndexed(*m_currentFrameDrawCommandBuffer, terrain.getAmountOfIndizes(), 1, 0, 0, 0);

	m_lastDraw3D = DrawRecord::TERRAIN;
}

static void CheckVkResultFn(VkResult err)
{
	ASSERT_VULKAN(err);
}

void bbe::INTERNAL::vulkan::VulkanManager::imguiStart()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(m_pwindow, false);

	ImGui_ImplVulkan_InitInfo implVulkanInitInfo = {};
	implVulkanInitInfo.Instance = m_instance.getInstance();
	implVulkanInitInfo.PhysicalDevice = m_device.getPhysicalDevice();
	implVulkanInitInfo.Device = m_device.getDevice();
	implVulkanInitInfo.QueueFamily = m_device.getQueueFamilyIndex();
	implVulkanInitInfo.Queue = m_device.getQueue();
	implVulkanInitInfo.PipelineCache = VK_NULL_HANDLE;
	implVulkanInitInfo.DescriptorPool = m_descriptorPool.getDescriptorPool();
	implVulkanInitInfo.Subpass = 0;
	implVulkanInitInfo.MinImageCount = m_imguiMinImageCount;
	implVulkanInitInfo.ImageCount = m_imguiMinImageCount;
	implVulkanInitInfo.Allocator = nullptr;
	implVulkanInitInfo.CheckVkResultFn = CheckVkResultFn;
	m_imguiInitSuccessful = ImGui_ImplVulkan_Init(&implVulkanInitInfo, m_renderPass.getRenderPass());
	if (!m_imguiInitSuccessful)
	{
		throw IllegalStateException();
	}

	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig fontConfig;
	imguiFontSmall = io.Fonts->AddFontDefault(&fontConfig);
	fontConfig.SizePixels = 26;
	imguiFontBig = io.Fonts->AddFontDefault(&fontConfig);

	VkCommandBuffer commandBuffer = INTERNAL::vulkan::startSingleTimeCommandBuffer(m_device.getDevice(), m_commandPool.getCommandPool());
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	INTERNAL::vulkan::endSingleTimeCommandBuffer(m_device.getDevice(), m_device.getQueue(), m_commandPool.getCommandPool(), commandBuffer);
}

void bbe::INTERNAL::vulkan::VulkanManager::imguiStop()
{
	if (m_imguiInitSuccessful)
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}
}

void bbe::INTERNAL::vulkan::VulkanManager::imguiStartFrame()
{
	// TODO: Still not ideal - what if the scale is anything else than 1 or 2 (e.g. on 8k)
	float scale = 0;
	glfwGetWindowContentScale(m_pwindow, &scale, nullptr);
	ImGuiIO& io = ImGui::GetIO();
	if (scale < 1.5f)
	{
		io.FontDefault = imguiFontSmall;
	}
	else
	{
		io.FontDefault = imguiFontBig;
	}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void bbe::INTERNAL::vulkan::VulkanManager::imguiEndFrame()
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, *m_currentFrameDrawCommandBuffer);
}

void bbe::INTERNAL::vulkan::VulkanManager::addLight(const bbe::Vector3& pos, float lightStrenght, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode)
{
	bbe::INTERNAL::vulkan::VulkanLight::addLight(pos, lightStrenght, lightColor, specularColor, falloffMode);
}

unsigned char* bbe::INTERNAL::vulkan::VulkanManager::ScreenshotFirstStage::toPixelData(bool* outRequiresSwizzle)
{
	if (subResourceLayout.rowPitch != width * 4u)
	{
		// This assumption is not conform to the vulkan spec, but in my tests
		// always held. It makes things much, much more performant as we are
		// able to simply memcpy the memory.
		std::cout << "VIDEO PIXEL PANIC!" << std::endl;
		exit(1);
	}
	unsigned char* data;
	vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	unsigned char* retVal = new unsigned char[height * width * 4u];
	memcpy(retVal, data, height * width * 4u);

	std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
	bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), format) != formatsBGR.end());
	
	if (outRequiresSwizzle) *outRequiresSwizzle = colorSwizzle;

	vkUnmapMemory(device, dstImageMemory);
	vkFreeMemory(device, dstImageMemory, nullptr);
	vkDestroyImage(device, dstImage, nullptr);

	return retVal;
}
