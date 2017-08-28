#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanCommandPool.h"

namespace bbe
{

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
			class VulkanCommandPool;
		}
	}

	class Terrain
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;

		static VkDevice         s_device;
		static VkPhysicalDevice s_physicalDevice;
		static VkQueue          s_queue;
		static INTERNAL::vulkan::VulkanCommandPool *s_pcommandPool;



		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);

		void init() const;
		void initIndexBuffer() const;
		void initVertexBuffer() const;
		void destroy();
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_indexBuffer;
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_vertexBuffer;
		mutable int m_numberOfVertices = 0;

		int m_width;
		int m_height;

		mutable bool m_created = false;

	public:
		Terrain(int width, int height);
		~Terrain();

		Matrix4 getTransform() const;
		void setTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		void setTransform(const Matrix4 &transform);
	};
}