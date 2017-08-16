#include "stdafx.h"
#include "BBE/VWTransform.h"
#include "BBE/Exceptions.h"
#include "BBE/EngineSettings.h"

bbe::INTERNAL::vulkan::VulkanBuffer *bbe::VWTransform::s_buffers = nullptr;
int bbe::VWTransform::NUM_BUFFERS_PER_CONTAINER = 1024;
bbe::Stack<int> bbe::VWTransform::s_indexStack;

void bbe::VWTransform::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	int amountOfBuffers = Settings::getAmountOfTransformContainers();
	s_buffers = new INTERNAL::vulkan::VulkanBuffer[amountOfBuffers];
	for (int i = 0; i < amountOfBuffers; i++)
	{
		s_buffers[i].create(device, physicalDevice, sizeof(Matrix4) * NUM_BUFFERS_PER_CONTAINER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}
	for (int i = 0; i < NUM_BUFFERS_PER_CONTAINER * amountOfBuffers; i++)
	{
		s_indexStack.push(i);
	}

}

void bbe::VWTransform::s_destroy()
{
	int amountOfBuffers = Settings::getAmountOfTransformContainers();
	for (int i = 0; i < amountOfBuffers; i++)
	{
		s_buffers[i].destroy();
	}
	delete[] s_buffers;
}

int bbe::VWTransform::getIndex() const
{
	if (!m_indexValid)
	{
		if (!s_indexStack.hasDataLeft())
		{
			throw NoTransformsLeftException();
		}
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

bbe::VWTransform::~VWTransform()
{
	if (m_indexValid)
	{
		s_indexStack.push(m_index);
	}
}

bbe::VWTransform & bbe::VWTransform::operator=(const Matrix4 & mat)
{
	(*(static_cast<Matrix4*>(this))) = mat;

	return *this;
}
