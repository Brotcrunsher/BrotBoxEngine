#include "stdafx.h"
#include "BBE\VulkanStopWatch.h"
#include "BBE\VulkanStopWatch.h"
#include "BBE\VulkanHelper.h"

bbe::INTERNAL::vulkan::VulkanStopWatch::VulkanStopWatch()
{
	//DO NOTHING
}

void bbe::INTERNAL::vulkan::VulkanStopWatch::create(VulkanDevice &device)
{
	m_queryPool.create(device, VK_QUERY_TYPE_TIMESTAMP, 2);
}

void bbe::INTERNAL::vulkan::VulkanStopWatch::destroy()
{
	m_queryPool.destroy();
}

void bbe::INTERNAL::vulkan::VulkanStopWatch::arm(VkCommandBuffer command)
{
	m_queryPool.writeTimestamp(command, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
}

void bbe::INTERNAL::vulkan::VulkanStopWatch::end(VkCommandBuffer command)
{
	m_queryPool.writeTimestamp(command, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
}

void bbe::INTERNAL::vulkan::VulkanStopWatch::finish(VulkanCommandPool &pool, VkQueue queue)
{
	m_queryPool.getResults();
	m_timePassed = m_queryPool.getResult(1) - m_queryPool.getResult(0);
	auto command = startSingleTimeCommandBuffer(m_queryPool.getDevice(), pool.getCommandPool());
	m_queryPool.reset(pool.getCommandPool(), queue);
	endSingleTimeCommandBuffer(m_queryPool.getDevice(), queue, pool.getCommandPool(), command);
}

uint64_t bbe::INTERNAL::vulkan::VulkanStopWatch::getTimePassed()
{
	return m_timePassed;
}
