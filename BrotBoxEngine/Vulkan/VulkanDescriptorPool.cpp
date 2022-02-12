#include "BBE/VulkanDescriptorPool.h"
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

	for (std::size_t i = 0; i < m_setLayouts.getLength(); i++)
	{
		for (std::size_t k = 0; k < m_setLayouts[i].m_pvulkanDescriptorSetLayout->m_bindings.getLength(); k++)
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

	// Although we nicely calculate the exact amount of required buffers,
	// some dependencies may choose not to do so or don't give us the
	// exact amount. For such dependencies we just add more to some types.
	constexpr uint32_t additionalSizes = 1000;

	List<VkDescriptorPoolSize> poolSizes;
	poolSizes.add({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         amountOfUniformBuffer        + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, amountOfCombinedImageSampler + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_SAMPLER,                amountOfSampler              + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 0                            + additionalSizes});
	poolSizes.add({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       0                            + additionalSizes});

	VkDescriptorPoolCreateInfo dpci = {};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	dpci.maxSets = amountOfSets + additionalSizes * poolSizes.getLength();
	dpci.poolSizeCount = static_cast<uint32_t>(poolSizes.getLength());
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
