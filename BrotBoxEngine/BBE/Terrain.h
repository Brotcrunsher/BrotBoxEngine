#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanCommandPool.h"
#include "../BBE/List.h"

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

	class Terrain;

	class TerrainPatch
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
		friend class Terrain;
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
		void destroy() const;
		mutable List<bbe::INTERNAL::vulkan::VulkanBuffer> m_indexBuffers;
		mutable List<bbe::INTERNAL::vulkan::VulkanBuffer> m_vertexBuffers;
		mutable List<int> m_numberOfVertices = 0;

		int m_width;
		int m_height;

		mutable bool m_created = false;
		mutable bool m_needsDestruction = true;
		float* m_pdata = nullptr;

	public:
		TerrainPatch(int width, int height, float* data);
		~TerrainPatch();

		TerrainPatch(const TerrainPatch& other) = delete;
		TerrainPatch(TerrainPatch&& other);
		TerrainPatch& operator=(const TerrainPatch& other) = delete;
		TerrainPatch& operator=(TerrainPatch&& other) = delete;

		Matrix4 getTransform() const;
		void setTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		void setTransform(const Matrix4 &transform);
	};

	class Terrain
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;
		List<TerrainPatch> m_patches;

		void init() const;
		void destroy() const;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);

		int m_patchesWidthAmount  = 0; 
		int m_patchesHeightAmount = 0;

	public:
		static const int AMOUNT_OF_LOD_LEVELS;
		Terrain(int width, int height);
		~Terrain();

		Matrix4 getTransform() const;
		void setTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		void setTransform(const Matrix4 &transform);
	};
}