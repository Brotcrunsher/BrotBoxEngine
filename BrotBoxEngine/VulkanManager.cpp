#include "BBE/Vector2.h"
#include "BBE/Vector3.h"
#include "BBE/VulkanManager.h"
#include "BBE/Color.h"
#include "BBE/Exceptions.h"
#include "BBE/Rectangle.h"
#include "BBE/EngineSettings.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Terrain.h"
#include "BBE/PointLight.h"
#include "BBE/Profiler.h"

#include <iostream>

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
	m_presentFence.init(m_device);

	std::cout << "Vulkan Manager: creating 3DBrush" << std::endl;
	m_primitiveBrush3D.create(m_device);
	bbe::PointLight::s_init(m_device.getDevice(), m_device.getPhysicalDevice());
	bbe::Rectangle::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::Circle::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::Cube::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::Terrain::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::TerrainMesh::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());
	bbe::IcoSphere::s_init(m_device.getDevice(), m_device.getPhysicalDevice(), m_commandPool, m_device.getQueue());


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

	m_setVertexLight              .addUniformBuffer(PointLight::s_bufferVertexData  , 0, 0);
	m_setFragmentLight            .addUniformBuffer(PointLight::s_bufferFragmentData, 0, 0);
	m_setViewProjectionMatrixLight.addUniformBuffer(m_primitiveBrush3D.m_uboMatrices, 0, 0);
	m_setVertexLight              .create(m_device, m_descriptorPool, m_setLayoutVertexLight);
	m_setFragmentLight            .create(m_device, m_descriptorPool, m_setLayoutFragmentLight);
	m_setViewProjectionMatrixLight.create(m_device, m_descriptorPool, m_setLayoutViewProjectionMatrix);


	std::cout << "Vulkan Manager: Loading Shaders" << std::endl;
	m_vertexShader2DPrimitive           .init(m_device, "vert2DPrimitive.spv");
	m_fragmentShader2DPrimitive         .init(m_device, "frag2DPrimitive.spv");
	m_vertexShader2DImage               .init(m_device, "vert2DImage.spv");
	m_fragmentShader2DImage             .init(m_device, "frag2DImage.spv");
	m_vertexShader3DPrimitive           .init(m_device, "vert3DPrimitive.spv");
	m_fragmentShader3DPrimitive         .init(m_device, "frag3DPrimitive.spv");
	m_vertexShader3DTerrain             .init(m_device, "vert3DTerrain.spv");
	m_fragmentShader3DTerrain           .init(m_device, "frag3DTerrain.spv");
	m_teseShader3DTerrain               .init(m_device, "tese3DTerrain.spv");
	m_tescShader3DTerrain               .init(m_device, "tesc3DTerrain.spv");
	m_vertexShader3DTerrainMesh         .init(m_device, "vert3DTerrainMesh.spv");
	m_vertexShader3DTerrainTransformed  .init(m_device, "vert3DTerrainTransformed.spv");
	m_fragmentShader3DTerrainTransformed.init(m_device, "frag3DTerrainTransformed.spv");

	std::cout << "Vulkan Manager: creating pipeline" << std::endl;
	createPipelines();

	m_uboMatrixViewProjection.create(m_device, sizeof(Matrix4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	m_uboMatrixModel.create(m_device, sizeof(Matrix4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	m_renderPassStopWatch.create(m_device);
}

void bbe::INTERNAL::vulkan::VulkanManager::destroy()
{
	vkDeviceWaitIdle(m_device.getDevice());
	s_pinstance = nullptr;
	
	m_renderPassStopWatch.destroy();
	
	bbe::Cube::s_destroy();
	bbe::Circle::s_destroy();
	bbe::Rectangle::s_destroy();
	bbe::PointLight::s_destroy();
	bbe::IcoSphere::s_destroy();

	bbe::TerrainMesh::s_destroy();
	bbe::Terrain::s_destroy();

	destroyPendingBuffers();
	m_presentFence.destroy();
	m_semaphoreRenderingDone.destroy();
	m_semaphoreImageAvailable.destroy();
	m_depthImage.destroy();
	m_commandPool.destroy();

	m_uboMatrixViewProjection.destroy();
	m_uboMatrixModel.destroy();
	m_pipeline3DPrimitive.destroy();
	m_pipeline3DTerrainSingle.destroy();
	m_pipeline3DTerrain.destroy();
	m_pipeline3DTerrainMesh.destroy();
	m_pipeline3DTerrainTransformed.destroy();

	m_fragmentShader3DPrimitive.destroy();
	m_vertexShader3DPrimitive.destroy();
	m_vertexShader3DTerrain.destroy();
	m_fragmentShader3DTerrain.destroy();
	m_teseShader3DTerrain.destroy();
	m_tescShader3DTerrain.destroy();
	m_vertexShader3DTerrainMesh.destroy();
	m_vertexShader3DTerrainTransformed.destroy();
	m_fragmentShader3DTerrainTransformed.destroy();

	m_pipeline2DPrimitive.destroy();
	m_pipeline2DImage.destroy();
	m_fragmentShader2DPrimitive.destroy();
	m_vertexShader2DPrimitive.destroy();
	m_vertexShader2DImage.destroy();
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
	m_primitiveBrush3D.destroy();
	m_renderPass.destroy();
	m_swapchain.destroy();
	m_device.destroy();
	m_surface.destroy();
	m_instance.destroy();
}

void bbe::INTERNAL::vulkan::VulkanManager::preDraw2D()
{
}

void bbe::INTERNAL::vulkan::VulkanManager::preDraw3D()
{
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DPrimitive.getLayout(), 1, 1, m_setViewProjectionMatrixLight.getPDescriptorSet(), 0, nullptr);

	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrain.getLayout(), 1, 1, m_setViewProjectionMatrixLight.getPDescriptorSet(), 0, nullptr);

	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrainSingle.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrainSingle.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrainSingle.getLayout(), 1, 1, m_setViewProjectionMatrixLight.getPDescriptorSet(), 0, nullptr);

	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrainMesh.getLayout(), 0, 1, m_setVertexLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrainMesh.getLayout(), 2, 1, m_setFragmentLight.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentFrameDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline3DTerrainMesh.getLayout(), 1, 1, m_setViewProjectionMatrixLight.getPDescriptorSet(), 0, nullptr);
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

	m_renderPassStopWatch.arm(m_currentFrameDrawCommandBuffer);

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

	renderPassBeginInfo.clearValueCount = clearValues.getLength();
	renderPassBeginInfo.pClearValues = clearValues.getRaw();


	vkCmdBeginRenderPass(m_currentFrameDrawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_screenWidth;
	viewport.height = (float)m_screenHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_currentFrameDrawCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = { m_screenWidth, m_screenHeight };
	vkCmdSetScissor(m_currentFrameDrawCommandBuffer, 0, 1, &scissor);

	m_primitiveBrush2D.INTERNAL_beginDraw(
		m_device, 
		m_commandPool, 
		m_descriptorPool, 
		m_setLayoutSampler, 
		m_currentFrameDrawCommandBuffer, 
		m_pipeline2DPrimitive, 
		m_pipeline2DImage, 
		m_screenWidth, m_screenHeight);
	
	m_primitiveBrush3D.INTERNAL_beginDraw(
		m_device, 
		m_currentFrameDrawCommandBuffer, 
		m_pipeline3DPrimitive, 
		m_pipeline3DTerrain, 
		m_pipeline3DTerrainSingle,
		m_pipeline3DTerrainMesh,
		m_pipeline3DTerrainTransformed,
		m_commandPool, 
		m_descriptorPool, 
		m_setLayoutTerrainHeightMap, 
		m_setLayoutSampler, 
		m_setLayoutTerrainAdditionalTexture,
		m_setLayoutTerrainAdditionalTextureWeight,
		m_setLayoutTerrainViewFrustum,
		m_screenWidth, m_screenHeight);
}

void bbe::INTERNAL::vulkan::VulkanManager::postDraw()
{
	vkCmdEndRenderPass(m_currentFrameDrawCommandBuffer);

	m_renderPassStopWatch.end(m_currentFrameDrawCommandBuffer);

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
}

void bbe::INTERNAL::vulkan::VulkanManager::waitEndDraw()
{
	m_presentFence.waitForFence();
	m_commandPool.freeCommandBuffer(m_currentFrameDrawCommandBuffer);
	destroyPendingBuffers();

	m_renderPassStopWatch.finish(m_commandPool, m_device.getQueue());
	bbe::Profiler::INTERNAL::setRenderTime(m_renderPassStopWatch.getTimePassed() * m_device.m_properties.limits.timestampPeriod / 1000.f / 1000.f / 1000.f);
}

bbe::PrimitiveBrush2D * bbe::INTERNAL::vulkan::VulkanManager::getBrush2D()
{
	return &m_primitiveBrush2D;
}

bbe::PrimitiveBrush3D * bbe::INTERNAL::vulkan::VulkanManager::getBrush3D()
{
	return &m_primitiveBrush3D;
}

void bbe::INTERNAL::vulkan::VulkanManager::addPendingDestructionBuffer(VkBuffer buffer, VkDeviceMemory memory)
{
	m_pendingDestructionBuffers.push(buffer);
	m_pendingDestructionMemory.push(memory);
}

void bbe::INTERNAL::vulkan::VulkanManager::createPipelines()
{
	m_pipeline2DPrimitive.init(m_vertexShader2DPrimitive, m_fragmentShader2DPrimitive, m_screenWidth, m_screenHeight);
	m_pipeline2DPrimitive.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline2DPrimitive.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline2DPrimitive.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color));
	m_pipeline2DPrimitive.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4);
	m_pipeline2DPrimitive.enableDepthBuffer();
	m_pipeline2DPrimitive.create(m_device.getDevice(), m_renderPass.getRenderPass());

	m_pipeline2DImage.init(m_vertexShader2DImage, m_fragmentShader2DImage, m_screenWidth, m_screenHeight);
	m_pipeline2DImage.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline2DImage.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline2DImage.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color));
	m_pipeline2DImage.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(float) * 4);
	m_pipeline2DImage.enableDepthBuffer();
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

	m_pipeline3DTerrainSingle.init(m_vertexShader3DTerrain, m_fragmentShader3DTerrain, m_screenWidth, m_screenHeight);
	m_pipeline3DTerrainSingle.setTessellationShader(m_tescShader3DTerrain.getModule(), m_teseShader3DTerrain.getModule(), 4);
	m_pipeline3DTerrainSingle.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline3DTerrainSingle.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline3DTerrainSingle.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color) + sizeof(Vector4));
	m_pipeline3DTerrainSingle.addPushConstantRange(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(Color) + sizeof(Vector4), sizeof(Matrix4) + sizeof(float));
	m_pipeline3DTerrainSingle.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(float) * 5);
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutVertexLight.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutViewProjectionMatrix.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutFragmentLight.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutTerrainHeightMap.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutSampler.getDescriptorSetLayout());
	//m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutTerrainViewFrustum.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTexture.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTextureWeight.getDescriptorSetLayout());
	m_pipeline3DTerrainSingle.enableDepthBuffer();
	m_pipeline3DTerrainSingle.addSpezializationConstant(0, 0, sizeof(int32_t));
	m_pipeline3DTerrainSingle.setSpezializationData(sizeof(int32_t), &spezAmountOfLights);
	m_pipeline3DTerrainSingle.enablePrimitiveRestart(false);
	m_pipeline3DTerrainSingle.setPrimitiveTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	m_pipeline3DTerrainSingle.create(m_device.getDevice(), m_renderPass.getRenderPass());

	m_pipeline3DTerrainMesh.init(m_vertexShader3DTerrainMesh, m_fragmentShader3DTerrain, m_screenWidth, m_screenHeight);
	m_pipeline3DTerrainMesh.addVertexBinding(0, sizeof(Vector3), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline3DTerrainMesh.addVertexDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
	m_pipeline3DTerrainMesh.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color) + sizeof(Vector4));
	m_pipeline3DTerrainMesh.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(Matrix4) + sizeof(Vector4) + sizeof(Vector4) + sizeof(float));
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutVertexLight.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutViewProjectionMatrix.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutFragmentLight.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutTerrainHeightMap.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutSampler.getDescriptorSetLayout());
	//m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutSampler.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTexture.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTextureWeight.getDescriptorSetLayout());
	m_pipeline3DTerrainMesh.enableDepthBuffer();
	m_pipeline3DTerrainMesh.addSpezializationConstant(0, 0, sizeof(int32_t));
	m_pipeline3DTerrainMesh.setSpezializationData(sizeof(int32_t), &spezAmountOfLights);
	m_pipeline3DTerrainMesh.enablePrimitiveRestart(true);
	m_pipeline3DTerrainMesh.setPrimitiveTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	m_pipeline3DTerrainMesh.create(m_device.getDevice(), m_renderPass.getRenderPass());


	m_pipeline3DTerrainTransformed.init(m_vertexShader3DTerrainTransformed, m_fragmentShader3DTerrain, m_screenWidth, m_screenHeight);
	m_pipeline3DTerrainTransformed.addVertexBinding(0, sizeof(Vector2), VK_VERTEX_INPUT_RATE_VERTEX);
	m_pipeline3DTerrainTransformed.addVertexDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
	m_pipeline3DTerrainTransformed.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color) + sizeof(Vector4));
	m_pipeline3DTerrainTransformed.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color), sizeof(Matrix4) + sizeof(Vector4) + sizeof(Vector4) + sizeof(float));
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutVertexLight.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutViewProjectionMatrix.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutFragmentLight.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutTerrainHeightMap.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutSampler.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTexture.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.addDescriptorSetLayout(m_setLayoutTerrainAdditionalTextureWeight.getDescriptorSetLayout());
	m_pipeline3DTerrainTransformed.enableDepthBuffer();
	m_pipeline3DTerrainTransformed.addSpezializationConstant(0, 0, sizeof(int32_t));
	m_pipeline3DTerrainTransformed.setSpezializationData(sizeof(int32_t), &spezAmountOfLights);
	m_pipeline3DTerrainTransformed.enablePrimitiveRestart(true);
	m_pipeline3DTerrainTransformed.setPrimitiveTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	m_pipeline3DTerrainTransformed.create(m_device.getDevice(), m_renderPass.getRenderPass());


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


	m_pipeline2DPrimitive.destroy();
	m_pipeline2DImage.destroy();
	m_pipeline3DPrimitive.destroy();
	m_pipeline3DTerrainSingle.destroy();
	m_pipeline3DTerrain.destroy();
	m_pipeline3DTerrainMesh.destroy();
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
}

