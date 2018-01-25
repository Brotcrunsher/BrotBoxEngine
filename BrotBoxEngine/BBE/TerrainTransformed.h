#pragma once

#include "../BBE/Matrix4.h"
#include "../BBE/Vector2.h"
#include "../BBE/List.h"
#include "../BBE/Image.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/ValueNoise2D.h"


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


	class TerrainTransformed
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;
		Image m_baseTexture;

		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_vertexBuffer;
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_indexBuffer;

		Image m_additionalTextures[16];
		Image m_additionalTextureWeights[16];
		int m_currentAdditionalTexture = 0;

		mutable bool m_wasInit = false;

		void init(
			const INTERNAL::vulkan::VulkanDevice & device,
			const INTERNAL::vulkan::VulkanCommandPool & commandPool,
			const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutHeightMap,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutTexture,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextures,
			const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextureWeights) const;
		void destroy();
		void initIndexBuffer(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const;
		void initVertexBuffer(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const;


		Image m_heightMap;
		Vector2 m_heightmapScale;

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

		mutable uint32_t m_amountOfIndizes = 0;
		uint32_t getAmountOfIndizes() const;

		ValueNoise2D m_valueNoise;

	public:
		TerrainTransformed(int width, int height, const char* baseTexturePath);
		TerrainTransformed(int width, int height, const char* baseTexturePath, int seed);
		~TerrainTransformed();

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
	};
}