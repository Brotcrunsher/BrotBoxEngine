#pragma once

#include "../BBE/VulkanQueryPool.h"
#include "../BBE/VulkanDevice.h"
#include "../BBE/VulkanCommandPool.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanStopWatch
			{
			private:
				VulkanQueryPool m_queryPool;
				uint64_t m_timePassed = std::numeric_limits<uint64_t>::max();

			public:
				VulkanStopWatch();

				void create(VulkanDevice &device);
				void destroy();
				void arm(VkCommandBuffer command);
				void end(VkCommandBuffer command);
				void finish(VulkanCommandPool &pool, VkQueue queue);
				uint64_t getTimePassed();
			};
		}
	}
}