#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanCommandPool.h"
#include "../BBE/List.h"
#include "../BBE/Image.h"
#include "../BBE/Vector2.h"
#include "../BBE/EngineSettings.h"
#include "../BBE/ValueNoise2D.h"
#include "../BBE/ViewFrustum.h"

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
		static VkDevice         s_device;
		static VkPhysicalDevice s_physicalDevice;
		static VkQueue          s_queue;
		static INTERNAL::vulkan::VulkanCommandPool *s_pcommandPool;


		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);

		static void s_initIndexBuffer();
		static void s_initVertexBuffer();
		static void s_destroy();
		static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
		static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;

		Vector2 m_offset;

		float m_patchTextureWidth;
		float m_patchTextureHeight;
		float m_patchX;
		float m_patchY;

		int m_patchXInt;
		int m_patchYInt;
		

	public:
		TerrainPatch(int patchX, int patchY, int maxPatchX, int maxPatchY);

		TerrainPatch(const TerrainPatch& other) = delete;
		TerrainPatch(TerrainPatch&& other);
		TerrainPatch& operator=(const TerrainPatch& other) = delete;
		TerrainPatch& operator=(TerrainPatch&& other) = delete;
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

		Image m_additionalTextures[16];
		Image m_additionalTextureWeights[16];
		int m_currentAdditionalTexture = 0;

		mutable bool m_wasInit = false;
		mutable INTERNAL::vulkan::VulkanBuffer m_viewFrustrumBuffer;
		mutable INTERNAL::vulkan::VulkanDescriptorSet m_viewFrustrumDescriptor;

		float m_patchSize;

		void init(
			const INTERNAL::vulkan::VulkanDevice & device, 
			const INTERNAL::vulkan::VulkanCommandPool & commandPool, 
			const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, 
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutHeightMap, 
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutTexture,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextures,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextureWeights,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutViewFrustrums) const;
		void destroy();

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();

		Vector2 m_heightmapScale;

		int m_patchesWidthAmount  = 0; 
		int m_patchesHeightAmount = 0;

		class TextureBias
		{
		public:
			Vector2 m_textureMult = Vector2(1, 1);
			Vector2 m_textureOffset = Vector2(0, 0);
		} m_baseTextureBias;

		void loadViewFrustrum(const bbe::Matrix4 &mvpMat, const bbe::INTERNAL::vulkan::VulkanDevice &device) const;

		float m_maxHeight = 100;
		int m_width = 0;
		int m_height = 0;

		void construct(int width, int height, const char* baseTexturePath, int seed);

		ValueNoise2D m_valueNoise;

		mutable bbe::INTERNAL::vulkan::ViewFrustum m_viewFrustum;

	public:
		Terrain(int width, int height, const char* baseTexturePath);
		Terrain(int width, int height, const char* baseTexturePath, int seed);
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

		void addTexture(const char* texturePath, const float* weights);
		Vector3 projectOnTerrain(const Vector3& pos) const;

		int getWidth() const;
		int getHeight() const;
	};
}