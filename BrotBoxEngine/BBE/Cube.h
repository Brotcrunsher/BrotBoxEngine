#pragma once

#include "../BBE/VulkanBuffer.h"
#include "../BBE/Matrix4.h"
#include "../BBE/Vector3.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class Cube
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();
		static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
		static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;
		static size_t amountOfIndices;

	public:
		Cube();
		Cube(const Vector3 &pos, const Vector3 &scale = Vector3(1, 1, 1), const Vector3 &rotationVector = Vector3(0, 0, 0), float radians = 0);
		explicit Cube(const Matrix4 &transform);

		void set(const Vector3 &pos, const Vector3 &scale = Vector3(1, 1, 1), const Vector3 &rotationVector = Vector3(0, 0, 0), float radians = 0);
		void setRotation(const Vector3 &rotationVector, float radians);

		Vector3 getPos() const;
		float getX() const;
		float getY() const;
		float getZ() const;

		Vector3 getScale() const;
		float getWidth() const;
		float getHeight() const;
		float getDepth() const;

		Matrix4 getTransform() const;

		bbe::List<bbe::Vector3> getNormals() const;
		bbe::List<bbe::Vector3> getVertices() const;
	};
}