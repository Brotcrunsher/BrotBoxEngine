#include "stdafx.h"
#include "BBE\VulkanDescriptorPool.h"
#include "BBE/VulkanDescriptorSetLayout.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanHelper.h"
#include <stdint.h>

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::addVulkanDescriptorSetLayout(const VulkanDescriptorSetLayout &dsl, uint32_t amountOfSets)
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}
	m_setLayouts.add(VulkanDescriptorPoolSetLayoutContainer(&dsl, amountOfSets));
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::create(const VulkanDevice & device)
{
	m_device = device.getDevice();
	uint32_t amountOfUniformBuffer = 0;
	uint32_t amountOfCombinedImageSampler = 0;
	uint32_t amountOfSampler = 0;
	uint32_t amountOfSets = 0;

	for (int i = 0; i < m_setLayouts.getLength(); i++)
	{
		for (int k = 0; k < m_setLayouts[i].m_pvulkanDescriptorSetLayout->m_bindings.getLength(); k++)
		{
			switch (m_setLayouts[i].m_pvulkanDescriptorSetLayout->m_bindings[k].descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				amountOfUniformBuffer        += m_setLayouts[i].m_pvulkanDescriptorSetLayout->m_bindings[k].descriptorCount * m_setLayouts[i].m_amountOfSets;
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				amountOfCombinedImageSampler += m_setLayouts[i].m_pvulkanDescriptorSetLayout->m_bindings[k].descriptorCount * m_setLayouts[i].m_amountOfSets;
				break;
			case VK_DESCRIPTOR_TYPE_SAMPLER:
				amountOfSampler              += m_setLayouts[i].m_pvulkanDescriptorSetLayout->m_bindings[k].descriptorCount * m_setLayouts[i].m_amountOfSets;
				break;
			default:
				throw NotImplementedException();
			}

			amountOfSets += m_setLayouts[i].m_amountOfSets;
		}
	}

	List<VkDescriptorPoolSize> poolSizes;
	if (amountOfUniformBuffer > 0)
	{
		VkDescriptorPoolSize dps = {};
		dps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dps.descriptorCount = amountOfUniformBuffer;
		poolSizes.add(dps);
	}

	if (amountOfCombinedImageSampler > 0)
	{
		VkDescriptorPoolSize dps = {};
		dps.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dps.descriptorCount = amountOfCombinedImageSampler;
		poolSizes.add(dps);
	}

	if (amountOfSampler > 0)
	{
		VkDescriptorPoolSize dps = {};
		dps.type = VK_DESCRIPTOR_TYPE_SAMPLER;
		dps.descriptorCount = amountOfSampler;
		poolSizes.add(dps);
	}

	VkDescriptorPoolCreateInfo dpci = {};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.flags = 0;
	dpci.maxSets = amountOfSets;
	dpci.poolSizeCount = poolSizes.getLength();
	dpci.pPoolSizes = poolSizes.getRaw();

	VkResult result = vkCreateDescriptorPool(m_device, &dpci, nullptr, &m_descriptorPool);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorPool::destroy()
{
	if (m_descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
		m_descriptorPool = VK_NULL_HANDLE;
		m_device         = VK_NULL_HANDLE;
		m_setLayouts.clear();
	}
}

VkDescriptorPool bbe::INTERNAL::vulkan::VulkanDescriptorPool::getDescriptorPool() const
{
	return m_descriptorPool;
}

bbe::INTERNAL::vulkan::VulkanDescriptorPoolSetLayoutContainer::VulkanDescriptorPoolSetLayoutContainer(const VulkanDescriptorSetLayout * vulkanDescriptorSetLayout, uint32_t amountOfSets)
{
	m_pvulkanDescriptorSetLayout = vulkanDescriptorSetLayout;
	m_amountOfSets = amountOfSets;
}
