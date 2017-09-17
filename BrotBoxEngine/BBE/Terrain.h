#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanCommandPool.h"
#include "../BBE/List.h"
#include "../BBE/Image.h"
#include "../BBE/Vector2.h"

namespace bbe
{

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
			class VulkanCommandPool;
			class VulkanBuffer;
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
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_indexBuffer;
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_vertexBuffer;

		mutable bool m_created = false;
		mutable bool m_needsDestruction = true;
		float* m_pdata = nullptr;

		float m_patchX; 
		float m_patchY;
		float m_patchTextureWidth;
		float m_patchTextureHeight;

		
		

	public:
		TerrainPatch(float* data, int patchX, int patchY, int maxPatchX, int maxPatchY);
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
		Image m_heightMap;
		Image m_baseTexture;
		mutable bool m_wasInit = false;
		mutable INTERNAL::vulkan::VulkanBuffer m_baseTextureBiasBuffer;
		mutable INTERNAL::vulkan::VulkanDescriptorSet m_baseTextureDescriptor;

		void init(
			const INTERNAL::vulkan::VulkanDevice & device, 
			const INTERNAL::vulkan::VulkanCommandPool & commandPool, 
			const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, 
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutHeightMap, 
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutTexture,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutBaseTextureBias) const;
		void destroy();

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);

		int m_patchesWidthAmount  = 0; 
		int m_patchesHeightAmount = 0;

		class TextureBias
		{
		public:
			Vector2 m_textureMult = Vector2(1, 1);
			Vector2 m_textureOffset = Vector2(0, 0);
		} m_baseTextureBias;

		void loadTextureBias() const;

		float m_maxHeight = 100;
	public:
		Terrain(int width, int height, const char* baseTexturePath);
		~Terrain();

		Matrix4 getTransform() const;
		void setTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		void setTransform(const Matrix4 &transform);

		Vector2 getBaseTextureOffset();
		void setBaseTextureOffset(const Vector2 &offset);
		Vector2 getBaseTextureMult();
		void setBaseTextureMult(const Vector2 &mult);

		void setMaxHeight(float height);
		float getMaxHeight() const;
	};
}