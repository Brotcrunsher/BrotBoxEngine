#include "stdafx.h"
#include "BBE/VWTransform.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::VWTransform::s_buffer;
int bbe::VWTransform::NUM_BUFFERS = 1024;
bbe::Stack<int> bbe::VWTransform::s_indexStack;

void bbe::VWTransform::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_buffer.create(device, physicalDevice, sizeof(Matrix4) * NUM_BUFFERS, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	for (int i = 0; i < NUM_BUFFERS; i++)
	{
		s_indexStack.push(i);
	}

}

void bbe::VWTransform::s_destroy()
{
	s_buffer.destroy();
}

int bbe::VWTransform::getIndex() const
{
	if (!m_indexValid)
	{
		m_index = s_indexStack.pop();
		m_indexValid = true;
	}
	return m_index;
}

bbe::VWTransform::VWTransform()
{
}

bbe::VWTransform::VWTransform(const Vector4 & col0, const Vector4 & col1, const Vector4 & col2, const Vector4 & col3)
	:Matrix4(col0, col1, col2, col3)
{
}

bbe::VWTransform & bbe::VWTransform::operator=(const Matrix4 & mat)
{
	(*(static_cast<Matrix4*>(this))) = mat;

	return *this;
}
