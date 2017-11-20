#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/VulkanDevice.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanQueryPool
			{
			private:
				VkDevice                      m_device    = VK_NULL_HANDLE;
				VkQueryPool                   m_queryPool = VK_NULL_HANDLE;
				VkQueryType                   m_type;
				uint32_t                      m_count;
				VkQueryPipelineStatisticFlags m_statisticFlags;

				bool m_wasCreated = false;
				uint32_t m_currentQueryIndex = 0;
				uint64_t* m_presultArray = nullptr;

				bool getResultsWasCalled = false;

			public:
				VulkanQueryPool();

				void create(VulkanDevice &device, VkQueryType type, uint32_t count, VkQueryPipelineStatisticFlags statisticFlags = 0);
				void destroy();
				void reset(VkCommandPool commandPool, VkQueue queue);

				uint32_t beginQuery(VkCommandBuffer command);
				void endQuery(VkCommandBuffer command, uint32_t query);
				void writeTimestamp(VkCommandBuffer command, VkPipelineStageFlagBits pipelineStage);

				void getResults();
				uint64_t getResult(size_t index);

				VkDevice getDevice() const;
			};
		}
	}
}