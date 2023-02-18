#include "BBE/Vulkan/VulkanDescriptorSet.h"
#include "BBE/Vulkan/VulkanDescriptorPool.h"
#include "BBE/Vulkan/VulkanDescriptorSetLayout.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanDescriptorSet.h"
#include "BBE/Vulkan/VulkanBuffer.h"
#include "BBE/Vulkan/VulkanImage.h"
#include "BBE/Image.h"

void bbe::INTERNAL::vulkan::VulkanDescriptorSet::addUniformBuffer(const VulkanBuffer & buffer, VkDeviceSize offset, uint32_t binding)
{
	if (m_descriptorSet != VK_NULL_HANDLE)
	{
		throw AlreadyCreatedException();
	}

	VkDescriptorBufferInfo dbi = {};
	dbi.buffer = buffer.getBuffer();
	dbi.offset = offset;
	dbi.range = buffer.getSize();

	m_descriptorBufferInfos.add(AdvancedDescriptorBufferInfo(dbi, binding));
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSet::addCombinedImageSampler(const Image& image, uint32_t binding)
{
	VkDescriptorImageInfo dii = {};
	VulkanImage* vi = (VulkanImage*)image.m_prendererData.get();
	if (!vi)
	{
		throw NullPointerException();
	}
	dii.sampler = vi->getSampler();
	dii.imageView = vi->getImageView();
	dii.imageLayout = vi->getImageLayout();
	

	m_descriptorImageInfos.add(AdvancedDescriptorImageInfo(dii, binding));
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSet::create(const VulkanDevice &device, const VulkanDescriptorPool & descriptorPool, const VulkanDescriptorSetLayout & setLayout)
{
	m_device = device.getDevice();
	m_descriptorPool = descriptorPool.getDescriptorPool();

	VkDescriptorSetAllocateInfo dsai = {};
	dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.pNext = nullptr;
	dsai.descriptorPool = descriptorPool.getDescriptorPool();
	dsai.descriptorSetCount = 1;
	VkDescriptorSetLayout dsl = setLayout.getDescriptorSetLayout();
	dsai.pSetLayouts = &dsl;

	vkAllocateDescriptorSets(device.getDevice(), &dsai, &m_descriptorSet);

	update(device);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSet::destroy()
{
	if (m_descriptorSet != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &m_descriptorSet);

		m_descriptorSet  = VK_NULL_HANDLE;
		m_device         = VK_NULL_HANDLE;
		m_descriptorPool = VK_NULL_HANDLE;
	}
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSet::update(const VulkanDevice & device)
{
	List<VkDescriptorImageInfo> imageInfos;
	for (std::size_t i = 0; i < m_descriptorImageInfos.getLength(); i++)
	{
		imageInfos.add(m_descriptorImageInfos[i].m_descriptorImageInfo);
	}

	List<VkWriteDescriptorSet> writeDescriptorSets;
	for (std::size_t i = 0; i < m_descriptorBufferInfos.getLength(); i++)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.pNext = nullptr;
		wds.dstSet = m_descriptorSet;
		wds.dstBinding = m_descriptorBufferInfos[i].m_binding;
		wds.dstArrayElement = 0;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wds.pImageInfo = nullptr;
		wds.pBufferInfo = &(m_descriptorBufferInfos[i].m_descriptorBufferInfo);
		wds.pTexelBufferView = nullptr;

		writeDescriptorSets.add(wds);
	}

	for (std::size_t i = 0; i < m_descriptorImageInfos.getLength(); i++)
	{
		uint32_t descriptorCount = 1;
		for (std::size_t k = i + 1; k < m_descriptorImageInfos.getLength(); k++)
		{
			if (m_descriptorImageInfos[k].m_binding == m_descriptorImageInfos[i].m_binding)
			{
				descriptorCount++;
			}
			else
			{
				break;
			}
		}

		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.pNext = nullptr;
		wds.dstSet = m_descriptorSet;
		wds.dstBinding = m_descriptorImageInfos[i].m_binding;
		wds.dstArrayElement = 0;
		wds.descriptorCount = descriptorCount;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.pImageInfo = &(imageInfos[i]);
		wds.pBufferInfo = nullptr;
		wds.pTexelBufferView = nullptr;

		writeDescriptorSets.add(wds);

		i += descriptorCount - 1;
	}

	vkUpdateDescriptorSets(device.getDevice(), static_cast<uint32_t>(writeDescriptorSets.getLength()), writeDescriptorSets.getRaw(), 0, nullptr);
}

VkDescriptorSet bbe::INTERNAL::vulkan::VulkanDescriptorSet::getDescriptorSet() const
{
	return m_descriptorSet;
}

const VkDescriptorSet * bbe::INTERNAL::vulkan::VulkanDescriptorSet::getPDescriptorSet() const
{
	return &m_descriptorSet;
}

bbe::INTERNAL::vulkan::AdvancedDescriptorBufferInfo::AdvancedDescriptorBufferInfo(const VkDescriptorBufferInfo &descriptorBufferInfo, uint32_t binding)
	: m_descriptorBufferInfo(descriptorBufferInfo)
{
	m_binding = binding;
}

bbe::INTERNAL::vulkan::AdvancedDescriptorImageInfo::AdvancedDescriptorImageInfo(const VkDescriptorImageInfo &descriptorImageInfo, uint32_t binding)
	: m_descriptorImageInfo(descriptorImageInfo)
{
	m_binding = binding;
}
