#include "stdafx.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/Color.h"
#include "BBE/Math.h"
#include "BBE/VulkanDescriptorPool.h"
#include "BBE/VulkanPipeline.h"
#include "BBE/Vector2.h"
#include "BBE/Matrix4.h"
#include "BBE/Rectangle.h"
#include "BBE/VulkanCommandPool.h"

void bbe::PrimitiveBrush3D::INTERNAL_setColor(float r, float g, float b, float a)
{
	Color c(r, g, b, a);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
	m_color = c;
}

void bbe::PrimitiveBrush3D::INTERNAL_beginDraw(
	bbe::INTERNAL::vulkan::VulkanDevice & device, 
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
	int width, int height)
{
	m_layoutPrimitive = pipelinePrimitive.getLayout();
	m_ppipelinePrimitive = &pipelinePrimitive;
	m_layoutTerrain = pipelineTerrain.getLayout();
	m_ppipelineTerrain = &pipelineTerrain;
	m_layoutTerrainSingle = pipelineTerrainSingle.getLayout();
	m_ppipelineTerrainSingle = &pipelineTerrainSingle;
	m_layoutTerrainMesh = pipelineTerrainMesh.getLayout();
	m_ppipelineTerrainMesh = &pipelineTerrainMesh;
	m_layoutTerrainTransformed = pipelineTerrainTransformed.getLayout();
	m_ppipelineTerrainTransformed = &pipelineTerrainTransformed;
	m_currentCommandBuffer = commandBuffer;
	m_pdescriptorPool = &descriptorPool;
	m_pdescriptorSetLayoutTerrainHeightMap = &descriptorSetLayoutTerrainHeightMap;
	m_pdescriptorSetLayoutTexture = &descriptorSetLayoutTexture;
	m_pdescriptorSetLayoutTerrainAdditionalTexture = &descriptorSetLayoutTerrainAdditionalTexture;
	m_pdescriptorSetLayoutTerrainAdditionalTextureWeight = &descriptorSetLayoutTerrainAdditionalTextureWeight;
	m_pdescriptorSetLayoutViewFrustum = &descriptorSetLayoutViewFrustum;
	m_pdevice = &device;
	m_pcommandPool = &commandPool;
	m_screenWidth = width;
	m_screenHeight = height;
	m_lastDraw = DrawRecord::NONE;
	m_pipelineRecord = PipelineRecord3D::NONE;

	setColor(1.0f, 1.0f, 1.0f, 1.0f);
	setCamera(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1));
}

void bbe::PrimitiveBrush3D::create(const INTERNAL::vulkan::VulkanDevice &vulkanDevice)
{
	Matrix4 mat;
	m_uboMatrices.create(vulkanDevice, sizeof(Matrix4) * 2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	{
		void* data = m_uboMatrices.map();
		memcpy(data, &mat, sizeof(Matrix4));
		memcpy((char*)data + sizeof(Matrix4), &mat, sizeof(Matrix4));
		m_uboMatrices.unmap();
	}
}

void bbe::PrimitiveBrush3D::destroy()
{
	m_uboMatrices.destroy();
}

void bbe::PrimitiveBrush3D::fillCube(const Cube & cube)
{
	if (m_pipelineRecord != PipelineRecord3D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord3D::PRIMITIVE;
	}
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 4, sizeof(Matrix4), &cube.m_transform);

	if (m_lastDraw != DrawRecord::CUBE)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = Cube::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = Cube::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

		m_lastDraw = DrawRecord::CUBE;
	}
	

	vkCmdDrawIndexed(m_currentCommandBuffer, 12 * 3, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush3D::fillIcoSphere(const IcoSphere & sphere)
{
	if (m_pipelineRecord != PipelineRecord3D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord3D::PRIMITIVE;
	}
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 4, sizeof(Matrix4), &sphere.m_transform);

	if (m_lastDraw != DrawRecord::ICOSPHERE)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = IcoSphere::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = IcoSphere::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

		m_lastDraw = DrawRecord::ICOSPHERE;
	}


	vkCmdDrawIndexed(m_currentCommandBuffer, IcoSphere::amountOfIndices, 1, 0, 0, 0);
}


void bbe::PrimitiveBrush3D::drawTerrain(const Terrain & terrain)
{
	terrain.init(
		*m_pdevice,
		*m_pcommandPool,
		*m_pdescriptorPool,
		*m_pdescriptorSetLayoutTerrainHeightMap,
		*m_pdescriptorSetLayoutTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTextureWeight,
		*m_pdescriptorSetLayoutViewFrustum
	);
	//terrain.loadViewFrustrum(m_projection * m_view * terrain.m_transform, *m_pdevice);
	if (m_pipelineRecord != PipelineRecord3D::TERRAIN)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineTerrain->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord3D::TERRAIN;
	}
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 3, 1, terrain.m_heightMap.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 4, 1, terrain.m_baseTexture.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	//vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 5, 1, terrain.m_viewFrustrumDescriptor.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 5, 1, terrain.m_additionalTextures[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 6, 1, terrain.m_additionalTextureWeights[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);


	class PushConts
	{
	public:
		Matrix4 mat;
		float height;
	}pushConts;

	class PushContsFragmentShader
	{
	public:
		Color c;
		Vector4 bias;
	} pushContsFragmenShader;

	pushContsFragmenShader.c = m_color;
	pushContsFragmenShader.bias.x = terrain.m_baseTextureBias.m_textureMult.x;
	pushContsFragmenShader.bias.y = terrain.m_baseTextureBias.m_textureMult.y;
	pushContsFragmenShader.bias.z = terrain.m_baseTextureBias.m_textureOffset.x;
	pushContsFragmenShader.bias.w = terrain.m_baseTextureBias.m_textureOffset.y;

	pushConts.height = terrain.getMaxHeight();
	pushConts.mat = terrain.m_transform;
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(PushContsFragmentShader), sizeof(PushConts), &(pushConts));
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_VERTEX_BIT, 108, sizeof(float), &terrain.m_patchSize);

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushContsFragmentShader), &(pushContsFragmenShader));

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = TerrainPatch::s_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = TerrainPatch::s_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_VERTEX_BIT, 112, sizeof(Vector2), &terrain.m_heightmapScale);
	for (std::size_t i = 0; i < terrain.m_patches.getLength(); i++)
	{
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(Vector2), &terrain.m_patches[i].m_offset);

		vkCmdDrawIndexed(m_currentCommandBuffer, 4, 1, 0, 0, 0);
	}

	m_lastDraw = DrawRecord::TERRAIN;

}

void bbe::PrimitiveBrush3D::drawTerrain(const TerrainSingle & terrain)
{
	terrain.init(
		*m_pdevice,
		*m_pcommandPool,
		*m_pdescriptorPool,
		*m_pdescriptorSetLayoutTerrainHeightMap,
		*m_pdescriptorSetLayoutTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTextureWeight,
		*m_pdescriptorSetLayoutViewFrustum
	);
	//terrain.loadViewFrustrum(m_projection * m_view * terrain.m_transform, *m_pdevice);
	if (m_pipelineRecord != PipelineRecord3D::TERRAINSINGLE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineTerrainSingle->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord3D::TERRAINSINGLE;
	}
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainSingle, 3, 1, terrain.m_heightMap.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainSingle, 4, 1, terrain.m_baseTexture.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	//vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainSingle, 5, 1, terrain.m_viewFrustrumDescriptor.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainSingle, 5, 1, terrain.m_additionalTextures[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainSingle, 6, 1, terrain.m_additionalTextureWeights[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);


	class PushConts
	{
	public:
		Matrix4 mat;
		float height;
	}pushConts;

	class PushContsFragmentShader
	{
	public:
		Color c;
		Vector4 bias;
	} pushContsFragmenShader;

	pushContsFragmenShader.c = m_color;
	pushContsFragmenShader.bias.x = terrain.m_baseTextureBias.m_textureMult.x;
	pushContsFragmenShader.bias.y = terrain.m_baseTextureBias.m_textureMult.y;
	pushContsFragmenShader.bias.z = terrain.m_baseTextureBias.m_textureOffset.x;
	pushContsFragmenShader.bias.w = terrain.m_baseTextureBias.m_textureOffset.y;

	pushConts.height = terrain.getMaxHeight();
	pushConts.mat = terrain.m_transform;
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(PushContsFragmentShader), sizeof(PushConts), &(pushConts));
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_VERTEX_BIT, 108, sizeof(float), &terrain.m_patchSize);

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushContsFragmentShader), &(pushContsFragmenShader));

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = terrain.m_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = terrain.m_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_VERTEX_BIT, 112, sizeof(Vector2), &terrain.m_heightmapScale);
	Vector2 emptyOffset(0, 0);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(Vector2), &emptyOffset);
	vkCmdDrawIndexed(m_currentCommandBuffer, terrain.getAmountOfIndizes(), 1, 0, 0, 0);

	m_lastDraw = DrawRecord::TERRAIN;

}

void bbe::PrimitiveBrush3D::drawTerrain(const TerrainTransformed & terrain)
{
	terrain.init(
		*m_pdevice,
		*m_pcommandPool,
		*m_pdescriptorPool,
		*m_pdescriptorSetLayoutTerrainHeightMap,
		*m_pdescriptorSetLayoutTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTextureWeight
	);
	//terrain.loadViewFrustrum(m_projection * m_view * terrain.m_transform, *m_pdevice);
	if (m_pipelineRecord != PipelineRecord3D::TERRAINTRANSFORMED)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineTerrainTransformed->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord3D::TERRAINTRANSFORMED;
	}
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainTransformed, 3, 1, terrain.m_heightMap.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainTransformed, 4, 1, terrain.m_baseTexture.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	//vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainTransformed, 5, 1, terrain.m_viewFrustrumDescriptor.getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainTransformed, 5, 1, terrain.m_additionalTextures[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainTransformed, 6, 1, terrain.m_additionalTextureWeights[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);


	class PushConts
	{
	public:
		Matrix4 mat;
		float height;
	}pushConts;

	class PushContsFragmentShader
	{
	public:
		Color c;
		Vector4 bias;
	} pushContsFragmenShader;

	pushContsFragmenShader.c = m_color;
	pushContsFragmenShader.bias.x = terrain.m_baseTextureBias.m_textureMult.x;
	pushContsFragmenShader.bias.y = terrain.m_baseTextureBias.m_textureMult.y;
	pushContsFragmenShader.bias.z = terrain.m_baseTextureBias.m_textureOffset.x;
	pushContsFragmenShader.bias.w = terrain.m_baseTextureBias.m_textureOffset.y;

	pushConts.height = terrain.getMaxHeight();
	pushConts.mat = terrain.m_transform;
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(PushContsFragmentShader), sizeof(PushConts), &(pushConts));

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushContsFragmentShader), &(pushContsFragmenShader));

	VkDeviceSize offsets[] = { 0 };
	VkBuffer buffer = terrain.m_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = terrain.m_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_VERTEX_BIT, 112, sizeof(Vector2), &terrain.m_heightmapScale);
	Vector2 emptyOffset(0, 0);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainSingle, VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(Vector2), &emptyOffset);
	vkCmdDrawIndexed(m_currentCommandBuffer, terrain.getAmountOfIndizes(), 1, 0, 0, 0);

	m_lastDraw = DrawRecord::TERRAIN;
}

void bbe::PrimitiveBrush3D::drawTerrain(const TerrainMesh & terrain)
{
	terrain.init(
		*m_pdevice,
		*m_pcommandPool,
		*m_pdescriptorPool,
		*m_pdescriptorSetLayoutTerrainHeightMap,
		*m_pdescriptorSetLayoutTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTexture,
		*m_pdescriptorSetLayoutTerrainAdditionalTextureWeight
	);

	if (m_pipelineRecord != PipelineRecord3D::TERRAINMESH)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineTerrainMesh->getPipeline(m_fillMode));
		m_pipelineRecord = PipelineRecord3D::TERRAINMESH;
	}
	
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainMesh, 4, 1, terrain.m_baseTexture.getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainMesh, 5, 1, terrain.m_additionalTextures[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrainMesh, 6, 1, terrain.m_additionalTextureWeights[0].getDescriptorSet().getPDescriptorSet(), 0, nullptr);

	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainMesh, VK_SHADER_STAGE_FRAGMENT_BIT, 16, sizeof(TerrainMesh::TextureBias), &(terrain.m_baseTextureBias));
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainMesh, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color) + sizeof(Vector4), sizeof(Matrix4), &(terrain.m_transform));
	Vector4 sizeAndOffset;
	sizeAndOffset.x = terrain.m_width / TerrainMesh::s_getVerticesPerMeter();
	sizeAndOffset.y = terrain.m_height / TerrainMesh::s_getVerticesPerMeter();
	Vector2 pos = terrain.getTransform().extractTranslation().xy();
	sizeAndOffset.z = pos.x;
	sizeAndOffset.w = pos.y;
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainMesh, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color) + sizeof(Vector4) + sizeof(Matrix4), sizeof(Vector4), &(sizeAndOffset));

	Vector3 terrainPos = terrain.getTransform().extractTranslation();
	for (std::size_t i = 0; i < terrain.m_patches.getLength(); i++)
	{
		terrain.m_patches[i].calculateLodLevel(m_cameraPos, terrainPos);
	}

	VkDeviceSize offsets[] = { 0 };
	for (std::size_t i = 0; i < terrain.m_patches.getLength(); i++)
	{
		int lod = terrain.m_patches[i].getLodLevel();

		VkBuffer buffer = terrain.m_patches[i].getIndexBuffer().getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
		buffer = terrain.m_patches[i].m_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		float offset = -lod * 0.1f;
		offset = 0;
		//offset = -terrain.m_patches[i].m_distanceToCamera * 0.1f;
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrainMesh, VK_SHADER_STAGE_VERTEX_BIT, sizeof(Color) + sizeof(Matrix4) + sizeof(Vector4) + sizeof(Vector4), sizeof(float), &(offset));

		int amountOfIndices = terrain.m_patches[i].getAmountOfIndices();
		vkCmdDrawIndexed(m_currentCommandBuffer, amountOfIndices, 1, 0, 0, 0);
	}

	m_lastDraw = DrawRecord::TERRAINMESH;
}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a);
}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f);
}

void bbe::PrimitiveBrush3D::setColor(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a);
}

void bbe::PrimitiveBrush3D::setCamera(const Vector3 & cameraPos, const Vector3 & cameraTarget, const Vector3 & cameraUpVector)
{
	m_view = Matrix4::createViewMatrix(cameraPos, cameraTarget, cameraUpVector);
	m_projection = Matrix4::createPerspectiveMatrix(Math::toRadians(60.0f), (float)m_screenWidth / (float)m_screenHeight, 0.05f, 20000.0f);

	void *data = m_uboMatrices.map();
	memcpy((char*)data, &m_view, sizeof(Matrix4));
	memcpy((char*)data + sizeof(Matrix4), &m_projection, sizeof(Matrix4));
	m_uboMatrices.unmap();

	m_cameraPos = cameraPos;
}

void bbe::PrimitiveBrush3D::setFillMode(FillMode fm)
{
	m_fillMode = fm;
}

bbe::FillMode bbe::PrimitiveBrush3D::getFillMode()
{
	return m_fillMode;
}
