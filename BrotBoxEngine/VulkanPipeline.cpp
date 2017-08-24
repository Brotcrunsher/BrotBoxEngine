#include "stdafx.h"
#include "BBE/VulkanPipeline.h"
#include "BBE/VWDepthImage.h"
#include "BBE/VulkanShader.h"

bbe::INTERNAL::vulkan::VulkanPipeline::VulkanPipeline()
{
}

void bbe::INTERNAL::vulkan::VulkanPipeline::init(VulkanShader vertexShader, VulkanShader fragmentShader, uint32_t width, uint32_t height)
{
	m_dynamicStates.add(VK_DYNAMIC_STATE_VIEWPORT);
	m_dynamicStates.add(VK_DYNAMIC_STATE_SCISSOR);

	m_shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_shaderStageCreateInfoVert.pNext = nullptr;
	m_shaderStageCreateInfoVert.flags = 0;
	m_shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	m_shaderStageCreateInfoVert.module = vertexShader.getModule();
	m_shaderStageCreateInfoVert.pName = "main";
	m_shaderStageCreateInfoVert.pSpecializationInfo = nullptr;


	m_shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_shaderStageCreateInfoFrag.pNext = nullptr;
	m_shaderStageCreateInfoFrag.flags = 0;
	m_shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	m_shaderStageCreateInfoFrag.module = fragmentShader.getModule();
	m_shaderStageCreateInfoFrag.pName = "main";
	m_shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

	m_inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssemblyCreateInfo.pNext = nullptr;
	m_inputAssemblyCreateInfo.flags = 0;
	m_inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	m_viewport.x = 0.0f;
	m_viewport.y = 0.0f;
	m_viewport.width = (float)width;
	m_viewport.height = (float)height;
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	m_scissor.offset = { 0, 0 };
	m_scissor.extent = { width, height };

	m_viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportStateCreateInfo.pNext = nullptr;
	m_viewportStateCreateInfo.flags = 0;
	m_viewportStateCreateInfo.viewportCount = 1;
	m_viewportStateCreateInfo.pViewports = &m_viewport;
	m_viewportStateCreateInfo.scissorCount = 1;
	m_viewportStateCreateInfo.pScissors = &m_scissor;


	m_rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizationCreateInfo.pNext = nullptr;
	m_rasterizationCreateInfo.flags = 0;
	m_rasterizationCreateInfo.depthClampEnable = VK_FALSE;
	m_rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	m_rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	m_rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	m_rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
	m_rasterizationCreateInfo.depthBiasClamp = 0.0f;
	m_rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
	m_rasterizationCreateInfo.lineWidth = 1.0f;

	m_multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampleCreateInfo.pNext = nullptr;
	m_multisampleCreateInfo.flags = 0;
	m_multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	m_multisampleCreateInfo.minSampleShading = 1.0f;
	m_multisampleCreateInfo.pSampleMask = nullptr;
	m_multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	m_multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	m_colorBlendAttachment.blendEnable = VK_TRUE;
	m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	m_colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_colorBlendCreateInfo.pNext = nullptr;
	m_colorBlendCreateInfo.flags = 0;
	m_colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	m_colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	m_colorBlendCreateInfo.attachmentCount = 1;
	m_colorBlendCreateInfo.pAttachments = &m_colorBlendAttachment;
	m_colorBlendCreateInfo.blendConstants[0] = 0.0f;
	m_colorBlendCreateInfo.blendConstants[1] = 0.0f;
	m_colorBlendCreateInfo.blendConstants[2] = 0.0f;
	m_colorBlendCreateInfo.blendConstants[3] = 0.0f;

	m_dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_dynamicStateCreateInfo.pNext = nullptr;
	m_dynamicStateCreateInfo.flags = 0;
	m_dynamicStateCreateInfo.dynamicStateCount = m_dynamicStates.getLength();
	m_dynamicStateCreateInfo.pDynamicStates = m_dynamicStates.getRaw();

	m_wasInitialized = true;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::create(VkDevice device, VkRenderPass renderPass)
{
	if (!m_wasInitialized)
	{
		throw bbe::NotInitializedException("Call init first!");
	}

	if (m_wasCreated)
	{
		throw bbe::AlreadyCreatedException("VulkanPipeline was already created!");
	}

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = m_vertexBindingDescription.getLength();
	vertexInputCreateInfo.pVertexBindingDescriptions = m_vertexBindingDescription.getRaw();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = m_vertexAttributeDescriptions.getLength();
	vertexInputCreateInfo.pVertexAttributeDescriptions = m_vertexAttributeDescriptions.getRaw();

	m_device = device;

	bbe::List<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.add(m_shaderStageCreateInfoVert);
	shaderStages.add(m_shaderStageCreateInfoFrag);

	if (m_geometryShader != VK_NULL_HANDLE)
	{
		VkPipelineShaderStageCreateInfo geometryShaderStageCreateInfo;
		geometryShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geometryShaderStageCreateInfo.pNext = nullptr;
		geometryShaderStageCreateInfo.flags = 0;
		geometryShaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geometryShaderStageCreateInfo.module = m_geometryShader;
		geometryShaderStageCreateInfo.pName = "main";
		geometryShaderStageCreateInfo.pSpecializationInfo = nullptr;

		shaderStages.add(geometryShaderStageCreateInfo);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = m_descriptorSetLayouts.getLength();
	pipelineLayoutCreateInfo.pSetLayouts = m_descriptorSetLayouts.getRaw();
	pipelineLayoutCreateInfo.pushConstantRangeCount = m_pushConstantRanges.getLength();
	pipelineLayoutCreateInfo.pPushConstantRanges = m_pushConstantRanges.getRaw();

	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
	ASSERT_VULKAN(result);

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = VWDepthImage::getDepthStencilStateCreateInfoOpaque();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = shaderStages.getLength();
	pipelineCreateInfo.pStages = shaderStages.getRaw();
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &m_inputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &m_viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &m_rasterizationCreateInfo;
	pipelineCreateInfo.pMultisampleState = &m_multisampleCreateInfo;
	if (m_useDepthBuffer)
	{
		pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	}
	else
	{
		pipelineCreateInfo.pDepthStencilState = nullptr;
	}
	pipelineCreateInfo.pColorBlendState = &m_colorBlendCreateInfo;
	pipelineCreateInfo.pDynamicState = &m_dynamicStateCreateInfo;
	pipelineCreateInfo.layout = m_pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
	ASSERT_VULKAN(result);

	m_wasCreated = true;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::destroy()
{
	if (m_wasCreated)
	{
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

		m_dynamicStates.clear();
		m_pushConstantRanges.clear();
		m_vertexBindingDescription.clear();
		m_vertexAttributeDescriptions.clear();
		m_descriptorSetLayouts.clear();

		m_wasCreated = false;
	}
}

VkPipeline bbe::INTERNAL::vulkan::VulkanPipeline::getPipeline() const
{
	if (!m_wasCreated)
	{
		throw std::logic_error("Pipeline was not created!");
	}
	return m_pipeline;
}

VkPipelineLayout bbe::INTERNAL::vulkan::VulkanPipeline::getLayout() const
{
	if (!m_wasCreated)
	{
		throw std::logic_error("Pipeline was not created!");
	}
	return m_pipelineLayout;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::setPolygonMode(VkPolygonMode polygonMode)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_rasterizationCreateInfo.polygonMode = polygonMode;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::setGeometryShader(VkShaderModule shaderModule)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_geometryShader = shaderModule;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexBinding(VkVertexInputBindingDescription vb)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_vertexBindingDescription.add(vb);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	VkVertexInputBindingDescription vb = {};
	vb.binding = binding;
	vb.stride = stride;
	vb.inputRate = inputRate;
	addVertexBinding(vb);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexDescription(VkVertexInputAttributeDescription vd)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_vertexAttributeDescriptions.add(vd);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	VkVertexInputAttributeDescription vd = {};
	vd.location = location;
	vd.binding = binding;
	vd.format = format;
	vd.offset = offset;
	addVertexDescription(vd);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addDescriptorSetLayout(VkDescriptorSetLayout dsl)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_descriptorSetLayouts.add(dsl);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addPushConstantRange(VkPushConstantRange pcr)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_pushConstantRanges.add(pcr);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	VkPushConstantRange pcr = {};
	pcr.stageFlags = stageFlags;
	pcr.offset = offset;
	pcr.size = size;
	addPushConstantRange(pcr);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::enableDepthBuffer()
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}
	m_useDepthBuffer = true;
}
