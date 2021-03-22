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

void bbe::PrimitiveBrush3D::INTERNAL_setColor(float r, float g, float b, float a, bool force)
{
	Color c(r, g, b, a);
	if (c.r != m_color.r || c.g != m_color.g || c.b != m_color.b || c.a != m_color.a || force)
	{
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutPrimitive, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
		m_color = c;
	}
}

void bbe::PrimitiveBrush3D::INTERNAL_beginDraw(
	bbe::INTERNAL::vulkan::VulkanDevice & device, 
	VkCommandBuffer commandBuffer, 
	INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive, 
	INTERNAL::vulkan::VulkanPipeline &pipelineTerrain,
	INTERNAL::vulkan::VulkanCommandPool &commandPool, 
	INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, 
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTerrainHeightMap, 
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTexture, 
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTerrainAdditionalTexture,
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutTerrainAdditionalTextureWeight,
	INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayoutViewFrustum,
	VkDescriptorSet setVertexLight,
	VkDescriptorSet setViewProjectionMatrixLight,
	VkDescriptorSet setFragmentLight,
	int width, int height)
{
	m_layoutPrimitive = pipelinePrimitive.getLayout();
	m_ppipelinePrimitive = &pipelinePrimitive;
	m_layoutTerrain = pipelineTerrain.getLayout();
	m_ppipelineTerrain = &pipelineTerrain;
	m_currentCommandBuffer = commandBuffer;
	m_pdescriptorPool = &descriptorPool;
	m_pdescriptorSetLayoutTerrainHeightMap = &descriptorSetLayoutTerrainHeightMap;
	m_pdescriptorSetLayoutTexture = &descriptorSetLayoutTexture;
	m_pdescriptorSetLayoutTerrainAdditionalTexture = &descriptorSetLayoutTerrainAdditionalTexture;
	m_pdescriptorSetLayoutTerrainAdditionalTextureWeight = &descriptorSetLayoutTerrainAdditionalTextureWeight;
	m_pdescriptorSetLayoutViewFrustum = &descriptorSetLayoutViewFrustum;
	m_pdevice = &device;
	m_pcommandPool = &commandPool;
	m_setVertexLight = setVertexLight;
	m_setViewProjectionMatrixLight = setViewProjectionMatrixLight;
	m_setFragmentLight = setFragmentLight;
	m_screenWidth = width;
	m_screenHeight = height;
	m_lastDraw = DrawRecord::NONE;
	m_pipelineRecord = PipelineRecord3D::NONE;

	INTERNAL_setColor(1.0f, 1.0f, 1.0f, 1.0f, true);
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

void bbe::PrimitiveBrush3D::bindPipelinePrimitive()
{
	if (m_pipelineRecord != PipelineRecord3D::PRIMITIVE)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelinePrimitive->getPipeline(m_fillMode));
		vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutPrimitive, 0, 1, &m_setVertexLight, 0, nullptr);
		vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutPrimitive, 1, 1, &m_setViewProjectionMatrixLight, 0, nullptr);
		vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutPrimitive, 2, 1, &m_setFragmentLight, 0, nullptr);
		m_pipelineRecord = PipelineRecord3D::PRIMITIVE;
	}
}

void bbe::PrimitiveBrush3D::bindPipelineTerrain()
{
	if (m_pipelineRecord != PipelineRecord3D::TERRAIN)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ppipelineTerrain->getPipeline(m_fillMode));
		vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 0, 1, &m_setVertexLight, 0, nullptr);
		vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 1, 1, &m_setViewProjectionMatrixLight, 0, nullptr);
		vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 2, 1, &m_setFragmentLight, 0, nullptr);
		m_pipelineRecord = PipelineRecord3D::TERRAIN;
	}
}

bbe::PrimitiveBrush3D::PrimitiveBrush3D()
{
	m_screenWidth = -1;
	m_screenHeight = -1;
	m_color = Color(-1000, -1000, -1000);
}

void bbe::PrimitiveBrush3D::fillCube(const Cube & cube)
{
	bindPipelinePrimitive();
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
	

	vkCmdDrawIndexed(m_currentCommandBuffer, Cube::amountOfIndices, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush3D::fillIcoSphere(const IcoSphere & sphere)
{
	bindPipelinePrimitive();
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

void bbe::PrimitiveBrush3D::drawTerrain(const Terrain& terrain)
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

	bindPipelineTerrain();
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
	VkBuffer buffer = terrain.m_vertexBuffer.getBuffer();
	vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

	buffer = terrain.m_indexBuffer.getBuffer();
	vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_VERTEX_BIT, 112, sizeof(Vector2), &terrain.m_heightmapScale);
	Vector2 emptyOffset(0, 0);
	vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_VERTEX_BIT, 100, sizeof(Vector2), &emptyOffset);
	vkCmdDrawIndexed(m_currentCommandBuffer, terrain.getAmountOfIndizes(), 1, 0, 0, 0);

	m_lastDraw = DrawRecord::TERRAIN;

}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a, false);
}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f, false);
}

void bbe::PrimitiveBrush3D::setColor(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a, false);
}

void bbe::PrimitiveBrush3D::setCamera(const Vector3 & cameraPos, const Vector3 & cameraTarget, const Vector3 & cameraUpVector)
{
	m_view = Matrix4::createViewMatrix(cameraPos, cameraTarget, cameraUpVector);
	m_projection = Matrix4::createPerspectiveMatrix(Math::toRadians(60.0f), (float)m_screenWidth / (float)m_screenHeight, 0.01f, 20000.0f);

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
