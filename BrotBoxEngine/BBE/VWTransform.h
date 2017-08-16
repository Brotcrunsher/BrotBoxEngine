#pragma once

#include "../BBE/Matrix4.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Stack.h"

namespace bbe
{

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanCommandPool;
			class VulkanManager;
		}
	}

	class VWTransform : public Matrix4
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class Cube;
		friend class PrimitiveBrush3D;
	private:
		static int NUM_BUFFERS_PER_CONTAINER;
		static INTERNAL::vulkan::VulkanBuffer *s_buffers;
		static Stack<int> s_indexStack;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();

		mutable int m_index;
		mutable bool m_indexValid = false;

		int getIndex() const;

	public:
		VWTransform();
		VWTransform(const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3);
		~VWTransform();

		VWTransform& operator=(const Matrix4 &mat);
	};
}