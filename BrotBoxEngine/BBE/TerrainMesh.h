#pragma once

#include "../BBE/Matrix4.h"
#include "../BBE/Vector2.h"
#include "../BBE/List.h"
#include "../BBE/Image.h"
#include "../BBE/VulkanBuffer.h"
#include "BBE/ValueNoise2D.h"


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

	class TerrainMesh;

	class TerrainMeshPatch
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
		friend class TerrainMesh;
	private:
		static VkDevice         s_device;
		static VkPhysicalDevice s_physicalDevice;
		static VkQueue          s_queue;
		static INTERNAL::vulkan::VulkanCommandPool *s_pcommandPool;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		void init(float height, bbe::INTERNAL::vulkan::VulkanBuffer &parentBuffer, VkDeviceSize &offset, VkDeviceSize alignment) const;

		static void s_initIndexBuffer();
		void initVertexBuffer(float height, bbe::INTERNAL::vulkan::VulkanBuffer &parentBuffer, VkDeviceSize &offset, VkDeviceSize alignment) const;
		void destroy() const;
		static void s_destroy();
		static List<bbe::INTERNAL::vulkan::VulkanBuffer> s_indexBuffer;
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_vertexBuffer;
		mutable List<float*> m_lodDatas;

		float* m_pdata = nullptr;

		int m_patchX;
		int m_patchY;

		Vector2 getOffset() const;
		float getSize() const;
		int getMaxLod() const;

		static List<int> s_indexCount;

		TerrainMeshPatch *m_pneighborUp = nullptr;
		TerrainMeshPatch *m_pneighborDown = nullptr;
		TerrainMeshPatch *m_pneighborLeft = nullptr;
		TerrainMeshPatch *m_pneighborRight = nullptr;
		void setNeighbors(TerrainMeshPatch *up, TerrainMeshPatch *down, TerrainMeshPatch *left, TerrainMeshPatch *right);

		mutable int m_lodLevel;
		void calculateLodLevel(const Vector3 &cameraPos, const Vector3 &terrainPos) const;
		int getLodLevel() const;

	public:
		TerrainMeshPatch(float* data, int patchX, int patchY);
		~TerrainMeshPatch();

		TerrainMeshPatch(const TerrainMeshPatch& other) = delete;
		TerrainMeshPatch(TerrainMeshPatch&& other);
		TerrainMeshPatch& operator=(const TerrainMeshPatch& other) = delete;
		TerrainMeshPatch& operator=(TerrainMeshPatch&& other) = delete;
	};


	class TerrainMesh
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;
		List<TerrainMeshPatch> m_patches;
		Image m_baseTexture;

		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_meshBuffer;

		Image m_additionalTextures[16];
		Image m_additionalTextureWeights[16];
		int m_currentAdditionalTexture = 0;

		mutable bool m_wasInit = false;

		float m_patchSize;

		void init(
			const INTERNAL::vulkan::VulkanDevice & device,
			const INTERNAL::vulkan::VulkanCommandPool & commandPool,
			const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutHeightMap,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutTexture,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextures,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextureWeights) const;
		void destroy();

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();

		Vector2 m_heightmapScale;

		int m_patchesWidthAmount = 0;
		int m_patchesHeightAmount = 0;

		class TextureBias
		{
		public:
			Vector2 m_textureMult = Vector2(1, 1);
			Vector2 m_textureOffset = Vector2(0, 0);
		} m_baseTextureBias;


		float m_maxHeight = 100;
		int m_width = 0;
		int m_height = 0;

		void construct(int width, int height, const char* baseTexturePath, int seed);

		int getAmountOfVerticesPerPatch() const;
		int getAmountOfLoDs() const;


		ValueNoise2D m_valueNoise;

	public:
		TerrainMesh(int width, int height, const char* baseTexturePath);
		TerrainMesh(int width, int height, const char* baseTexturePath, int seed);
		~TerrainMesh();

		Matrix4 getTransform() const;
		void setTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);
		void setTransform(const Matrix4 &transform);

		Vector2 getBaseTextureOffset();
		void setBaseTextureOffset(const Vector2 &offset);
		Vector2 getBaseTextureMult();
		void setBaseTextureMult(const Vector2 &mult);


		void setMaxHeight(float height);
		float getMaxHeight() const;

		void addTexture(const char* texturePath, const float* weights);

		int getWidth() const;
		int getHeight() const;

		Vector3 projectOnTerrain(const Vector3& pos) const;

		static float s_getVerticesPerMeter();
	};
}