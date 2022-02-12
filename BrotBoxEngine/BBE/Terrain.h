#pragma once

//Like Terrain but with a single mesh which is then used as a list of Patches for tessellation

#include "GLFW/glfw3.h"
#include "../BBE/Vulkan/VulkanBuffer.h"
#include "../BBE/Vulkan/VulkanCommandPool.h"
#include "../BBE/Vulkan/VulkanDevice.h"
#include "../BBE/Matrix4.h"
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

	class Terrain
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;
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

		void initIndexBuffer(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const;
		void initVertexBuffer(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const;
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_indexBuffer;
		mutable bbe::INTERNAL::vulkan::VulkanBuffer m_vertexBuffer;

		Vector2 m_heightmapScale;

		int m_patchesWidthAmount = 0;
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

		mutable uint32_t m_amountOfIndizes = 0;
		uint32_t getAmountOfIndizes() const;

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