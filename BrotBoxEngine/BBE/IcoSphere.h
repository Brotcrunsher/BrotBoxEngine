#pragma once

#include "../BBE/VulkanBuffer.h"
#include "../BBE/Matrix4.h"
#include "../BBE/Vector3.h"

namespace bbe
{

	class VulkanDevice;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class IcoSphere
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;


		static size_t amountOfVertices;
		static size_t amountOfIndices;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();
		static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
		static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;

	public:
		IcoSphere();
		IcoSphere(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		IcoSphere(const Matrix4 &transform);

		void set(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);

		Vector3 getPos() const;
		float getX() const;
		float getY() const;
		float getZ() const;

		Vector3 getScale() const;
		float getWidth() const;
		float getHeight() const;
		float getDepth() const;

		Matrix4 getTransform() const;
	};
}