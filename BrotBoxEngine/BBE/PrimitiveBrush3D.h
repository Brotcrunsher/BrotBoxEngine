#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "../BBE/Matrix4.h"
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Cube.h"
#include "../BBE/IcoSphere.h"
#include "../BBE/Terrain.h"
#include "../BBE/TerrainSingle.h"
#include "../BBE/TerrainMesh.h"
#include "../BBE/TerrainTransformed.h"
#include "../BBE/FillMode.h"
#include "../BBE/ViewFrustum.h"

namespace bbe
{
	class Color;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanManager;
			class VulkanBuffer;
			class VulkanDescriptorPool;
			class VulkanPipeline;
			class VulkanCommandPool;
			class VulkanDescriptorSetLayout;
			class VulkanDescriptorPool;
		}
	}

	enum class DrawRecord
	{
		NONE, CUBE, ICOSPHERE, TERRAIN, TERRAINMESH
	};

	enum class PipelineRecord3D
	{
		NONE, PRIMITIVE, TERRAIN, TERRAINMESH, TERRAINSINGLE, TERRAINTRANSFORMED
	};

	class PrimitiveBrush3D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		VkCommandBuffer                              m_currentCommandBuffer                               = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanDevice              *m_pdevice                                            = nullptr;
		VkPipelineLayout                             m_layoutPrimitive                                    = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelinePrimitive                                 = nullptr;
		VkPipelineLayout                             m_layoutTerrain                                      = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelineTerrain                                   = nullptr;
		VkPipelineLayout                             m_layoutTerrainSingle                                = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelineTerrainSingle                             = nullptr;
		VkPipelineLayout                             m_layoutTerrainMesh                                  = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelineTerrainMesh                               = nullptr;
		VkPipelineLayout                             m_layoutTerrainTransformed                           = VK_NULL_HANDLE;
		INTERNAL::vulkan::VulkanPipeline            *m_ppipelineTerrainTransformed                        = nullptr;
		INTERNAL::vulkan::VulkanDescriptorPool      *m_pdescriptorPool                                    = nullptr;
		INTERNAL::vulkan::VulkanCommandPool         *m_pcommandPool                                       = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayoutTerrainHeightMap               = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayoutTexture                        = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayoutTerrainAdditionalTexture       = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayoutTerrainAdditionalTextureWeight = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayoutViewFrustum                    = nullptr;
		int                                          m_screenWidth;
		int                                          m_screenHeight;

		DrawRecord m_lastDraw = DrawRecord::NONE;
		PipelineRecord3D m_pipelineRecord = PipelineRecord3D::NONE;

		Matrix4 m_modelMatrix;
		Matrix4 m_viewProjectionMatrix;

		Vector3 m_cameraPos;

		INTERNAL::vulkan::VulkanBuffer m_uboMatrices;

		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(
			bbe::INTERNAL::vulkan::VulkanDevice &device,
			VkCommandBuffer commandBuffer,
			INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive,
			INTERNAL::vulkan::VulkanPipeline &pipelineTerrain,
			INTERNAL::vulkan::VulkanPipeline &pipelineTerrainSingle,
			INTERNAL::vulkan::VulkanPipeline &pipelineTerrainMesh,
			INTERNAL::vulkan::VulkanPipeline &pipelineTerrainTransformed,
			INTERNAL::vulkan::VulkanCommandPool &commandPool,
			INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTerrainHeightMap,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTexture,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTerrainAdditionalTexture,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTerrainAdditionalTextureWeight,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutViewFrustum,
			int screenWidth, int screenHeight);
		
		void create(const INTERNAL::vulkan::VulkanDevice &vulkanDevice);
		void destroy();

		FillMode m_fillMode = FillMode::SOLID;

		Matrix4 m_view;
		Matrix4 m_projection;

		Color m_color;

	public:
		PrimitiveBrush3D();

		void fillCube(const Cube &cube);
		void fillIcoSphere(const IcoSphere &sphere);

		void drawTerrain(const Terrain &terrain);
		void drawTerrain(const TerrainMesh &terrain);
		void drawTerrain(const TerrainSingle &terrain);
		void drawTerrain(const TerrainTransformed &terrain);

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);

		void setCamera(const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUpVector = Vector3(0, 0, 1.0f));

		void setFillMode(FillMode fm);
		FillMode getFillMode();
	};
}