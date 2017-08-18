#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanShader;

			class VulkanPipeline
			{
			private:
				VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
				VkPipeline pipeline             = VK_NULL_HANDLE;
				VkDevice device                 = VK_NULL_HANDLE;
				VkShaderModule geometryShader   = VK_NULL_HANDLE;

				VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
				VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
				VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
				VkViewport viewport;
				VkRect2D scissor;
				VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
				VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
				VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
				VkPipelineColorBlendAttachmentState colorBlendAttachment;
				//VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
				VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
				bbe::List<VkDynamicState> dynamicStates;
				VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
				bbe::List<VkPushConstantRange> pushConstantRanges;

				bbe::List<VkVertexInputBindingDescription> vertexBindingDescription;
				bbe::List<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
				bbe::List<VkDescriptorSetLayout> descriptorSetLayouts;

				bool wasInitialized = false;
				bool wasCreated = false;

				bool useDepthBuffer = false;

			public:
				VulkanPipeline();

				

				void init(VulkanShader vertexShader, VulkanShader fragmentShader, uint32_t width, uint32_t height);

				void create(VkDevice device, VkRenderPass renderPass);

				void destroy();

				VkPipeline getPipeline() const;

				VkPipelineLayout getLayout() const;

				void setPolygonMode(VkPolygonMode polygonMode);

				void setGeometryShader(VkShaderModule shaderModule);

				void addVertexBinding(VkVertexInputBindingDescription vb);
				void addVertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);

				void addVertexDescription(VkVertexInputAttributeDescription vd);
				void addVertexDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

				void addDescriptorSetLayout(VkDescriptorSetLayout dsl);

				void addPushConstantRange(VkPushConstantRange pcr);
				void addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);

				void enableDepthBuffer();
			};
		}
	}
}
