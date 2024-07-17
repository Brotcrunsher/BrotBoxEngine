#include "BBE/Vulkan/VulkanDescriptorSetLayout.h"
#include "BBE/Vulkan/VulkanHelper.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Error.h"

void bbe::INTERNAL::vulkan::VulkanDescriptorSetLayout::addBinding(const VkDescriptorSetLayoutBinding & dslb)
{
	if (m_descriptorSetLayout != VK_NULL_HANDLE)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
	}
	m_bindings.add(dslb);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSetLayout::addBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler * pImmutableSamplers)
{
	VkDescriptorSetLayoutBinding dslb = {};
	dslb.binding            = binding;
	dslb.descriptorType     = descriptorType;
	dslb.descriptorCount    = descriptorCount;
	dslb.stageFlags         = stageFlags;
	dslb.pImmutableSamplers = pImmutableSamplers;
	addBinding(dslb);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSetLayout::create(const VulkanDevice & device)
{
	m_device = device.getDevice();

	VkDescriptorSetLayoutCreateInfo dslci = {};
	dslci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslci.pNext        = nullptr;
	dslci.flags        = 0;
	dslci.bindingCount = static_cast<uint32_t>(m_bindings.getLength());
	dslci.pBindings    = m_bindings.getRaw();

	VkResult result = vkCreateDescriptorSetLayout(m_device, &dslci, nullptr, &m_descriptorSetLayout);
	ASSERT_VULKAN(result);
}

void bbe::INTERNAL::vulkan::VulkanDescriptorSetLayout::destroy()
{
	if (m_descriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
		m_device              = VK_NULL_HANDLE;
		m_descriptorSetLayout = VK_NULL_HANDLE;
		m_bindings.clear();
	}
}

VkDescriptorSetLayout bbe::INTERNAL::vulkan::VulkanDescriptorSetLayout::getDescriptorSetLayout() const
{
	return m_descriptorSetLayout;
}
