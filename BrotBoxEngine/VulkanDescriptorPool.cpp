#include "stdafx.h"
#include "BBE/HashMap.h"
#include "BBE\VulkanDescriptorPool.h"
#include "BBE\Exceptions.h"
#include "BBE\VulkanHelper.h"
#include "BBE\VulkanBuffer.h"


void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addDescriptor(VkDescriptorType type, VkShaderStageFlags shaderStage, const VulkanBuffer & buffer, VkDeviceSize offset, uint32_t binding, uint32_t setIndex)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}
	m_descriptorBufferInfos.add(
		AdvancedBufferInfo(
			type,
			shaderStage,
			buffer,
			offset,
			binding,
			setIndex
		)
	);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::create(VkDevice device)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}

	m_device = device;

	HashMap<int, List<VkDescriptorSetLayoutBinding>> layoutBindings;
	List<int> usedSetIndexes;
	for (int i = 0; i < m_descriptorBufferInfos.getLength(); i++)
	{
		VkDescriptorSetLayoutBinding dslb = {};
		dslb.binding = m_descriptorBufferInfos[i].m_binding;
		dslb.descriptorType = m_descriptorBufferInfos[i].m_type;
		dslb.descriptorCount = 1;
		dslb.stageFlags = m_descriptorBufferInfos[i].m_shaderStage;
		dslb.pImmutableSamplers = nullptr;

		if(!layoutBindings.contains(m_descriptorBufferInfos[i].m_setIndex))
		{
			layoutBindings.add(m_descriptorBufferInfos[i].m_setIndex, List<VkDescriptorSetLayoutBinding>());
			usedSetIndexes.add(m_descriptorBufferInfos[i].m_setIndex);
		}

		layoutBindings.get(m_descriptorBufferInfos[i].m_setIndex)->add(dslb);
	}

	for (int i = 0; i < usedSetIndexes.getLength(); i++)
	{
		VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
		List<VkDescriptorSetLayoutBinding> *list = layoutBindings.get(usedSetIndexes[i]);

		VkDescriptorSetLayoutCreateInfo dslci = {};
		dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslci.pNext = nullptr;
		dslci.flags = 0;
		dslci.bindingCount = list->getLength();
		dslci.pBindings = list->getRaw();

		VkResult result = vkCreateDescriptorSetLayout(m_device, &dslci, nullptr, &dsl);
		ASSERT_VULKAN(result);

		m_descriptorSetLayout.add(dsl);
	}
	

	List<VkDescriptorPoolSize> poolSizes;
	for (int i = 0; i < m_descriptorBufferInfos.getLength(); i++)
	{
		for (int k = 0; k < poolSizes.getLength(); k++)
		{
			if (poolSizes[k].type == m_descriptorBufferInfos[i].m_type)
			{
				poolSizes[k].descriptorCount += 1;
				goto continueLoop;	//Skip adding new PoolSize Entry
			}
		}

		{
			VkDescriptorPoolSize dps = {};
			dps.type = m_descriptorBufferInfos[i].m_type;
			dps.descriptorCount = 1;
			poolSizes.add(dps);
		}
		

	continueLoop:;
	}

	VkDescriptorPoolCreateInfo dpci;
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.flags = 0;
	dpci.maxSets = usedSetIndexes.getLength();
	dpci.poolSizeCount = poolSizes.getLength();
	dpci.pPoolSizes = poolSizes.getRaw();

	VkResult result = vkCreateDescriptorPool(m_device, &dpci, nullptr, &m_descriptorPool);
	ASSERT_VULKAN(result);


	VkDescriptorSetAllocateInfo dsai = {};
	dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.pNext = nullptr;
	dsai.descriptorPool = m_descriptorPool;
	dsai.descriptorSetCount = m_descriptorSetLayout.getLength();
	dsai.pSetLayouts = m_descriptorSetLayout.getRaw();
	m_descriptorSets.resizeCapacityAndLength(m_descriptorSetLayout.getLength());

	result = vkAllocateDescriptorSets(m_device, &dsai, m_descriptorSets.getRaw());
	ASSERT_VULKAN(result);

	List<VkWriteDescriptorSet> writeDescriptorSetsUniforms;
	List<VkDescriptorBufferInfo> descriptorBufferInfo;

	for (int i = 0; i < m_descriptorBufferInfos.getLength(); i++)
	{
		VkDescriptorBufferInfo dbi = {};
		dbi.buffer = m_descriptorBufferInfos[i].m_buffer;
		dbi.offset = m_descriptorBufferInfos[i].m_offset;
		dbi.range  = m_descriptorBufferInfos[i].m_bufferSize;

		descriptorBufferInfo.add(dbi);
	}

	for (int i = 0; i < m_descriptorBufferInfos.getLength(); i++)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.pNext = nullptr;
		wds.dstSet = m_descriptorSets[m_descriptorBufferInfos[i].m_setIndex];
		wds.dstBinding = m_descriptorBufferInfos[i].m_binding;
		wds.dstArrayElement = 0;
		wds.descriptorCount = 1;
		wds.descriptorType = m_descriptorBufferInfos[i].m_type;
		wds.pImageInfo = nullptr;
		wds.pBufferInfo = &(descriptorBufferInfo[i]);
		wds.pTexelBufferView = nullptr;

		writeDescriptorSetsUniforms.add(wds);
	}

	vkUpdateDescriptorSets(m_device, writeDescriptorSetsUniforms.getLength(), writeDescriptorSetsUniforms.getRaw(), 0, nullptr);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::destroy()
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		for (int i = 0; i < m_descriptorSetLayout.getLength(); i++)
		{
			vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout[i], nullptr);
		}
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
		m_descriptorPool = VK_NULL_HANDLE;
		m_device = VK_NULL_HANDLE;
		m_descriptorSets.clear();
	}
}

VkDescriptorSetLayout bbe::INTERNAL::vulkan::VulkanDescriptorPool::getLayout(size_t setNumber) const
{
	return m_descriptorSetLayout[setNumber];
}

VkDescriptorSet* bbe::INTERNAL::vulkan::VulkanDescriptorPool::getPSet(size_t setNumber)
{
	return &m_descriptorSets[setNumber];
}

bbe::INTERNAL::vulkan::VulkanDescriptorPool::AdvancedBufferInfo::AdvancedBufferInfo(VkDescriptorType type, VkShaderStageFlags shaderStage, const VulkanBuffer & buffer, VkDeviceSize offset, uint32_t binding, uint32_t setIndex)
{
	m_type = type;
	m_shaderStage = shaderStage;
	m_buffer = buffer.getBuffer();
	m_offset = offset;
	m_bufferSize = buffer.getSize();
	m_binding = binding;
	m_setIndex = setIndex;
}
