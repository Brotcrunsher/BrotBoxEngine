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
}

void bbe::PrimitiveBrush3D::INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice & device, VkCommandBuffer commandBuffer, INTERNAL::vulkan::VulkanPipeline &pipelinePrimitive, INTERNAL::vulkan::VulkanPipeline &pipelineTerrain, INTERNAL::vulkan::VulkanCommandPool &commandPool, INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout, int width, int height)
{
	m_layoutPrimitive = pipelinePrimitive.getLayout();
	m_pipelinePrimitive = pipelinePrimitive.getPipeline();
	m_layoutTerrain = pipelineTerrain.getLayout();
	m_pipelineTerrain = pipelineTerrain.getPipeline();
	m_currentCommandBuffer = commandBuffer;
	m_pdescriptorPool = &descriptorPool;
	m_pdescriptorSetLayoutTerrain = &descriptorSetLayout;
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
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelinePrimitive);
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
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelinePrimitive);
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
	terrain.init(*m_pdevice, *m_pcommandPool, *m_pdescriptorPool, *m_pdescriptorSetLayoutTerrain);

	
	if (m_pipelineRecord != PipelineRecord3D::TERRAIN)
	{
		vkCmdBindPipeline(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineTerrain);
		m_pipelineRecord = PipelineRecord3D::TERRAIN;
	}
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layoutTerrain, 3, 1, terrain.m_heightMap.m_descriptorSet.getPDescriptorSet(), 0, nullptr);

	for (int i = 0; i < terrain.m_patches.getLength(); i++)
	{
		vkCmdPushConstants(m_currentCommandBuffer, m_layoutTerrain, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, sizeof(Color), sizeof(Matrix4), &(terrain.m_patches[i].m_transform));
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = terrain.m_patches[i].m_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = terrain.m_patches[i].m_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);


		vkCmdDrawIndexed(m_currentCommandBuffer, 4, 1, 0, 0, 0);
	}

	m_lastDraw = DrawRecord::TERRAIN;
	
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
	Matrix4 view = Matrix4::createViewMatrix(cameraPos, cameraTarget, cameraUpVector);
	Matrix4 projection = Matrix4::createPerspectiveMatrix(Math::toRadians(60.0f), (float)m_screenWidth / (float)m_screenHeight, 0.001f, 10000.0f);

	void *data = m_uboMatrices.map();
	memcpy((char*)data, &view, sizeof(Matrix4));
	memcpy((char*)data + sizeof(Matrix4), &projection, sizeof(Matrix4));
	m_uboMatrices.unmap();

	m_cameraPos = cameraPos;
}
