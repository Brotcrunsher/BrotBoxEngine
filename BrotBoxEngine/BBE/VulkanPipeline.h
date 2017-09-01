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
				VkPipelineLayout m_pipelineLayout               = VK_NULL_HANDLE;
				VkPipeline       m_pipeline                     = VK_NULL_HANDLE;
				VkDevice         m_device                       = VK_NULL_HANDLE;
				VkShaderModule   m_geometryShader               = VK_NULL_HANDLE;
				VkShaderModule   m_tessellationShaderControl    = VK_NULL_HANDLE;
				VkShaderModule   m_tessellationShaderEvaluation = VK_NULL_HANDLE;

				VkPipelineShaderStageCreateInfo        m_shaderStageCreateInfoVert   = {};
				VkPipelineShaderStageCreateInfo        m_shaderStageCreateInfoFrag   = {};
				VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyCreateInfo     = {};
				VkViewport                             m_viewport                    = {};
				VkRect2D                               m_scissor                     = {};
				VkPipelineViewportStateCreateInfo      m_viewportStateCreateInfo     = {};
				VkPipelineRasterizationStateCreateInfo m_rasterizationCreateInfo     = {};
				VkPipelineTessellationStateCreateInfo  m_tessellationStateCreateInfo = {};
				VkPipelineMultisampleStateCreateInfo   m_multisampleCreateInfo       = {};
				VkPipelineColorBlendAttachmentState    m_colorBlendAttachment        = {};
				VkPipelineColorBlendStateCreateInfo    m_colorBlendCreateInfo        = {};
				bbe::List<VkDynamicState>              m_dynamicStates               = {};
				VkPipelineDynamicStateCreateInfo       m_dynamicStateCreateInfo      = {};
				bbe::List<VkPushConstantRange>         m_pushConstantRanges          = {};

				size_t m_spezializationSize = 0;
				const void* m_spezializationData = nullptr;
				bbe::List<VkSpecializationMapEntry>          m_specializationEntries;
				bbe::List<VkVertexInputBindingDescription>   m_vertexBindingDescription;
				bbe::List<VkVertexInputAttributeDescription> m_vertexAttributeDescriptions;
				bbe::List<VkDescriptorSetLayout>             m_descriptorSetLayouts;

				bool m_wasInitialized  = false;
				bool m_wasCreated      = false;
				bool m_useDepthBuffer  = false;
				bool m_useTessellation = false;

			public:
				VulkanPipeline();

				

				void init(VulkanShader vertexShader, VulkanShader fragmentShader, uint32_t width, uint32_t height);

				void create(VkDevice device, VkRenderPass renderPass);

				void destroy();

				VkPipeline getPipeline() const;

				VkPipelineLayout getLayout() const;

				void setPolygonMode(VkPolygonMode polygonMode);

				void setGeometryShader(VkShaderModule shaderModule);
				void setTessellationShader(VkShaderModule control, VkShaderModule evaluation, uint32_t patchControlPoints);

				void addVertexBinding(const VkVertexInputBindingDescription &vb);
				void addVertexBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);

				void addVertexDescription(const VkVertexInputAttributeDescription &vd);
				void addVertexDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

				void addDescriptorSetLayout(const VkDescriptorSetLayout &dsl);

				void addPushConstantRange(const VkPushConstantRange &pcr);
				void addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);

				void addSpezializationConstant(const VkSpecializationMapEntry &sme);
				void addSpezializationConstant(uint32_t constantID, uint32_t offset, size_t size);
				void setSpezializationData(size_t size, const void* data);

				void setPrimitiveTopology(VkPrimitiveTopology topology);
				void enablePrimitiveRestart(bool enable);

				void enableDepthBuffer();
			};
		}
	}
}
