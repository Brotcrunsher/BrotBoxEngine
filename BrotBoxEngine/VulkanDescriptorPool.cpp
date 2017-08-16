#include "stdafx.h"
#include "BBE\VulkanDescriptorPool.h"
#include "BBE\Exceptions.h"
#include "BBE\VulkanHelper.h"
#include "BBE\VulkanBuffer.h"

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::setAmountOfSets(size_t amount)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}
	amountOfSets = amount;
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addPoolSize(VkDescriptorPoolSize dps)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}
	m_descriptorPoolSizes.add(dps);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addPoolSize(VkDescriptorType type, uint32_t descriptorCount)
{
	VkDescriptorPoolSize dps = {};
	dps.type = type;
	dps.descriptorCount = descriptorCount;
	addPoolSize(dps);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addLayoutBinding(VkDescriptorSetLayoutBinding dslb)
{
	m_layoutBindings.add(dslb);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler * pImmutableSamplers)
{
	VkDescriptorSetLayoutBinding dslb = {};
	dslb.binding = binding;
	dslb.descriptorType = descriptorType;
	dslb.descriptorCount = descriptorCount;
	dslb.stageFlags = stageFlags;
	dslb.pImmutableSamplers = pImmutableSamplers;

	addLayoutBinding(dslb);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addDescriptorBufferInfo(VkDescriptorBufferInfo dbi, uint32_t binding, uint32_t setIndex)
{
	m_descriptorBufferInfo.add(AdvancedBufferInfo(dbi, binding, setIndex));
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t setIndex)
{
	VkDescriptorBufferInfo dbi = {};
	dbi.buffer = buffer;
	dbi.offset = offset;
	dbi.range = range;
	addDescriptorBufferInfo(dbi, binding, setIndex);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addDescriptorBufferInfo(const VulkanBuffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t setIndex)
{
	addDescriptorBufferInfo(buffer.getBuffer(), offset, range, binding, setIndex);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::create(VkDevice device)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}

	m_device = device;

	VkDescriptorSetLayoutCreateInfo dslci = {};
	dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslci.pNext = nullptr;
	dslci.flags = 0;
	dslci.bindingCount = m_layoutBindings.getLength();
	dslci.pBindings = m_layoutBindings.getRaw();

	VkResult result = vkCreateDescriptorSetLayout(m_device, &dslci, nullptr, &m_descriptorSetLayout);
	ASSERT_VULKAN(result);

	List<VkDescriptorSetLayout> layouts;
	for (int i = 0; i < amountOfSets; i++)
	{
		layouts.add(m_descriptorSetLayout);
	}


	VkDescriptorPoolCreateInfo dpci;
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.flags = 0;
	dpci.maxSets = amountOfSets;
	dpci.poolSizeCount = m_descriptorPoolSizes.getLength();
	dpci.pPoolSizes = m_descriptorPoolSizes.getRaw();

	result = vkCreateDescriptorPool(m_device, &dpci, nullptr, &m_descriptorPool);
	ASSERT_VULKAN(result);


	VkDescriptorSetAllocateInfo dsai = {};
	dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.pNext = nullptr;
	dsai.descriptorPool = m_descriptorPool;
	dsai.descriptorSetCount = amountOfSets;
	dsai.pSetLayouts = layouts.getRaw();
	m_descriptorSets = new VkDescriptorSet[amountOfSets];

	result = vkAllocateDescriptorSets(m_device, &dsai, m_descriptorSets);
	ASSERT_VULKAN(result);

	List<VkWriteDescriptorSet> writeDescriptorSetsUniforms;

	for (int i = 0; i < m_descriptorBufferInfo.getLength(); i++)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.pNext = nullptr;
		wds.dstSet = m_descriptorSets[m_descriptorBufferInfo[i].m_setIndex];
		wds.dstBinding = m_descriptorBufferInfo[i].m_binding;
		wds.dstArrayElement = 0;
		wds.descriptorCount = 1;
		wds.descriptorType = m_layoutBindings[i].descriptorType;
		wds.pImageInfo = nullptr;
		wds.pBufferInfo = &(m_descriptorBufferInfo[i].m_descriptorBufferInfo);
		wds.pTexelBufferView = nullptr;

		writeDescriptorSetsUniforms.add(wds);
	}

	vkUpdateDescriptorSets(m_device, writeDescriptorSetsUniforms.getLength(), writeDescriptorSetsUniforms.getRaw(), 0, nullptr);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::destroy()
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
		m_descriptorPool = VK_NULL_HANDLE;
		m_device = VK_NULL_HANDLE;
		delete[] m_descriptorSets;
		m_descriptorSets = nullptr;
	}
}

VkDescriptorSetLayout bbe::INTERNAL::vulkan::VulkanDescriptorPool::getLayout() const
{
	return m_descriptorSetLayout;
}

VkDescriptorSet* bbe::INTERNAL::vulkan::VulkanDescriptorPool::getPSet(size_t setNumber)
{
	return &m_descriptorSets[setNumber];
}

bbe::INTERNAL::vulkan::VulkanDescriptorPool::AdvancedBufferInfo::AdvancedBufferInfo(VkDescriptorBufferInfo dbi, uint32_t binding, uint32_t setIndex)
	: m_descriptorBufferInfo(dbi), m_binding(binding), m_setIndex(setIndex)
{
}
