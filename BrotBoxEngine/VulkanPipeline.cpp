#include "stdafx.h"
#include "BBE/VulkanPipeline.h"
#include "BBE/VWDepthImage.h"
#include "BBE/VulkanShader.h"

bbe::INTERNAL::vulkan::VulkanPipeline::VulkanPipeline()
{
}

void bbe::INTERNAL::vulkan::VulkanPipeline::init(VulkanShader vertexShader, VulkanShader fragmentShader, uint32_t width, uint32_t height)
{
	//depthStencilStateCreateInfo = VWDepthImage::getDepthStencilStateCreateInfoOpaque();

	shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoVert.pNext = nullptr;
	shaderStageCreateInfoVert.flags = 0;
	shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfoVert.module = vertexShader.getModule();
	shaderStageCreateInfoVert.pName = "main";
	shaderStageCreateInfoVert.pSpecializationInfo = nullptr;


	shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfoFrag.pNext = nullptr;
	shaderStageCreateInfoFrag.flags = 0;
	shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfoFrag.module = fragmentShader.getModule();
	shaderStageCreateInfoFrag.pName = "main";
	shaderStageCreateInfoFrag.pSpecializationInfo = nullptr;








	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.pNext = nullptr;
	inputAssemblyCreateInfo.flags = 0;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;


	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;


	scissor.offset = { 0, 0 };
	scissor.extent = { width, height };


	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = nullptr;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;


	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreateInfo.pNext = nullptr;
	rasterizationCreateInfo.flags = 0;
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationCreateInfo.depthBiasClamp = 0.0f;
	rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationCreateInfo.lineWidth = 1.0f;


	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.pNext = nullptr;
	multisampleCreateInfo.flags = 0;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.minSampleShading = 1.0f;
	multisampleCreateInfo.pSampleMask = nullptr;
	multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleCreateInfo.alphaToOneEnable = VK_FALSE;



	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.pNext = nullptr;
	colorBlendCreateInfo.flags = 0;
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendCreateInfo.blendConstants[0] = 0.0f;
	colorBlendCreateInfo.blendConstants[1] = 0.0f;
	colorBlendCreateInfo.blendConstants[2] = 0.0f;
	colorBlendCreateInfo.blendConstants[3] = 0.0f;




	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.flags = 0;
	dynamicStateCreateInfo.dynamicStateCount = dynamicStates.getLength();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.getRaw();


	



	wasInitialized = true;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::create(VkDevice device, VkRenderPass renderPass)
{
	if (!wasInitialized)
	{
		throw bbe::NotInitializedException("Call init first!");
	}

	if (wasCreated)
	{
		throw bbe::AlreadyCreatedException("VulkanPipeline was already created!");
	}

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.pNext = nullptr;
	vertexInputCreateInfo.flags = 0;
	vertexInputCreateInfo.vertexBindingDescriptionCount = vertexBindingDescription.getLength();
	vertexInputCreateInfo.pVertexBindingDescriptions = vertexBindingDescription.getRaw();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.getLength();
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.getRaw();

	this->device = device;

	bbe::List<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.add(shaderStageCreateInfoVert);
	shaderStages.add(shaderStageCreateInfoFrag);

	if (geometryShader != VK_NULL_HANDLE)
	{
		VkPipelineShaderStageCreateInfo geometryShaderStageCreateInfo;
		geometryShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geometryShaderStageCreateInfo.pNext = nullptr;
		geometryShaderStageCreateInfo.flags = 0;
		geometryShaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geometryShaderStageCreateInfo.module = geometryShader;
		geometryShaderStageCreateInfo.pName = "main";
		geometryShaderStageCreateInfo.pSpecializationInfo = nullptr;

		shaderStages.add(geometryShaderStageCreateInfo);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.getLength();
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.getRaw();
	pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.getLength();
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.getRaw();

	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	ASSERT_VULKAN(result);



	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.stageCount = shaderStages.getLength();
	pipelineCreateInfo.pStages = shaderStages.getRaw();
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	ASSERT_VULKAN(result);

	wasCreated = true;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::destroy()
{
	if (wasCreated)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		wasCreated = false;
	}
}

VkPipeline bbe::INTERNAL::vulkan::VulkanPipeline::getPipeline() const
{
	if (!wasCreated)
	{
		throw std::logic_error("Pipeline was not created!");
	}
	return pipeline;
}

VkPipelineLayout bbe::INTERNAL::vulkan::VulkanPipeline::getLayout() const
{
	if (!wasCreated)
	{
		throw std::logic_error("Pipeline was not created!");
	}
	return pipelineLayout;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::setPolygonMode(VkPolygonMode polygonMode)
{
	rasterizationCreateInfo.polygonMode = polygonMode;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::setGeometryShader(VkShaderModule shaderModule)
{
	geometryShader = shaderModule;
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexBinding(VkVertexInputBindingDescription vb)
{
	vertexBindingDescription.add(vb);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	VkVertexInputBindingDescription vb = {};
	vb.binding = binding;
	vb.stride = stride;
	vb.inputRate = inputRate;
	addVertexBinding(vb);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexDescription(VkVertexInputAttributeDescription vd)
{
	vertexAttributeDescriptions.add(vd);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addVertexDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
{
	VkVertexInputAttributeDescription vd = {};
	vd.location = location;
	vd.binding = binding;
	vd.format = format;
	vd.offset = offset;
	addVertexDescription(vd);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addDescriptorSetLayout(VkDescriptorSetLayout dsl)
{
	descriptorSetLayouts.add(dsl);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addPushConstantRange(VkPushConstantRange pcr)
{
	pushConstantRanges.add(pcr);
}

void bbe::INTERNAL::vulkan::VulkanPipeline::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
{
	VkPushConstantRange pcr = {};
	pcr.stageFlags = stageFlags;
	pcr.offset = offset;
	pcr.size = size;
	addPushConstantRange(pcr);
}
