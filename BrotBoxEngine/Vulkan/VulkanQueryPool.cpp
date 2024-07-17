#include "BBE/Vulkan/VulkanQueryPool.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Error.h"

bbe::INTERNAL::vulkan::VulkanQueryPool::VulkanQueryPool()
{
	m_type  = VK_QUERY_TYPE_MAX_ENUM;
	m_count = -1;
}

void bbe::INTERNAL::vulkan::VulkanQueryPool::create(VulkanDevice &device, VkQueryType type, uint32_t count, VkQueryPipelineStatisticFlags statisticFlags)
{
	if (m_wasCreated)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
	}

	m_device = device.getDevice();
	m_type = type;
	m_count = count;
	m_statisticFlags = statisticFlags;

	VkQueryPoolCreateInfo qpci = {};
	qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	qpci.queryType = type;
	qpci.queryCount = count;
	qpci.pipelineStatistics = statisticFlags;

	VkResult result = vkCreateQueryPool(m_device, &qpci, nullptr, &m_queryPool);
	ASSERT_VULKAN(result);


	m_currentQueryIndex = 0;
	m_presultArray = new uint64_t[count]; //TODO use allocator
	m_wasCreated = true;
	getResultsWasCalled = false;
}

void bbe::INTERNAL::vulkan::VulkanQueryPool::destroy()
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	vkDestroyQueryPool(m_device, m_queryPool, nullptr);

	m_queryPool = VK_NULL_HANDLE;
	delete[] m_presultArray;
	m_presultArray = nullptr;
	m_wasCreated = false;
	getResultsWasCalled = false;
}

void bbe::INTERNAL::vulkan::VulkanQueryPool::reset(VkCommandPool commandPool, VkQueue queue)
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	auto command = bbe::INTERNAL::vulkan::startSingleTimeCommandBuffer(m_device, commandPool);
	vkCmdResetQueryPool(command, m_queryPool, 0, m_count);
	bbe::INTERNAL::vulkan::endSingleTimeCommandBuffer(m_device, queue, commandPool, command);
	m_currentQueryIndex = 0;
	getResultsWasCalled = false;
}

uint32_t bbe::INTERNAL::vulkan::VulkanQueryPool::beginQuery(VkCommandBuffer command)
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_currentQueryIndex >= m_count)
	{
		bbe::Crash(bbe::Error::OutOfMemory);
	}
	uint32_t index = m_currentQueryIndex;
	m_currentQueryIndex++;
	vkCmdBeginQuery(command, m_queryPool, index, 0);
	return index;
}

void bbe::INTERNAL::vulkan::VulkanQueryPool::endQuery(VkCommandBuffer command, uint32_t query)
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (query >= m_currentQueryIndex)
	{
		bbe::Crash(bbe::Error::OutOfMemory);
	}
	vkCmdEndQuery(command, m_queryPool, query);
}

void bbe::INTERNAL::vulkan::VulkanQueryPool::writeTimestamp(VkCommandBuffer command, VkPipelineStageFlagBits pipelineStage)
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_currentQueryIndex >= m_count)
	{
		bbe::Crash(bbe::Error::OutOfMemory);
	}
	uint32_t index = m_currentQueryIndex;
	m_currentQueryIndex++;
	vkCmdWriteTimestamp(command, pipelineStage, m_queryPool, index);
}

void bbe::INTERNAL::vulkan::VulkanQueryPool::getResults()
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	vkGetQueryPoolResults(m_device, m_queryPool, 0, m_currentQueryIndex, sizeof(uint64_t) * m_count, m_presultArray, 0, VK_QUERY_RESULT_64_BIT);

	getResultsWasCalled = true;
}

uint64_t bbe::INTERNAL::vulkan::VulkanQueryPool::getResult(size_t index)
{
	if (!getResultsWasCalled)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	return m_presultArray[index];
}

VkDevice bbe::INTERNAL::vulkan::VulkanQueryPool::getDevice() const
{
	if (!m_wasCreated)
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	return m_device;
}
