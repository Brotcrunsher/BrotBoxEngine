#pragma once

#include "../BBE/VulkanBuffer.h"
#include "../BBE/VWTransform.h"

namespace bbe
{

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class Terrain
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		VWTransform m_transform;
		INTERNAL::vulkan::VulkanBuffer m_transformBuffer;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();
		static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
		static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;
		static int s_numberOfVertices;

		mutable bool m_bufferDirty = true;

	public:
		Terrain();

		Matrix4 getTransform() const;
		void setTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		void setTransform(const Matrix4 &transform);
	};
}