#include "stdafx.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/VulkanDevice.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/Color.h"
#include "BBE/Math.h"
#include "BBE/VulkanDescriptorPool.h"

void bbe::PrimitiveBrush3D::INTERNAL_setColor(float r, float g, float b, float a)
{
	Color c(r, g, b, a);
	vkCmdPushConstants(m_currentCommandBuffer, m_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Color), &c);
}

void bbe::PrimitiveBrush3D::INTERNAL_beginDraw(bbe::INTERNAL::vulkan::VulkanDevice & device, VkCommandBuffer commandBuffer, VkPipelineLayout layout, int width, int height, INTERNAL::vulkan::VulkanDescriptorPool *descriptorPool)
{
	m_layout = layout;
	m_currentCommandBuffer = commandBuffer;
	m_device = device.getDevice();
	m_physicalDevice = device.getPhysicalDevice();
	m_screenWidth = width;
	m_screenHeight = height;
	m_descriptorPool = descriptorPool;
	m_lastDraw = NONE;

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

	int index = cube.m_transform.getIndex();
	int containerIndex = index / 1024;
	int localOffset = index % 1024;
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 1, 1, m_descriptorPool->getPSet(containerIndex + 1), 0, nullptr);

	if (cube.m_bufferDirty)
	{
		

		void *data = VWTransform::s_buffers[containerIndex].map();
		Matrix4 transform = cube.getTransform();
		memcpy((char*)data + sizeof(Matrix4) * localOffset, &transform, sizeof(Matrix4));
		VWTransform::s_buffers[containerIndex].unmap();

		cube.m_bufferDirty = false;
	}
	

	vkCmdPushConstants(m_currentCommandBuffer, m_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 4, sizeof(uint32_t) * 1, &localOffset);

	if (m_lastDraw != CUBE)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = Cube::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = Cube::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

		m_lastDraw = CUBE;
	}
	

	vkCmdDrawIndexed(m_currentCommandBuffer, 12 * 3, 1, 0, 0, 0);
}

void bbe::PrimitiveBrush3D::drawTerrain(const Terrain & terrain)
{
	int index = terrain.m_transform.getIndex();
	int containerIndex = index / 1024;
	int localOffset = index % 1024;
	vkCmdBindDescriptorSets(m_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 1, 1, m_descriptorPool->getPSet(containerIndex + 1), 0, nullptr);

	if (terrain.m_bufferDirty)
	{


		void *data = VWTransform::s_buffers[containerIndex].map();
		Matrix4 transform = terrain.getTransform();
		memcpy((char*)data + sizeof(Matrix4) * localOffset, &transform, sizeof(Matrix4));
		VWTransform::s_buffers[containerIndex].unmap();

		terrain.m_bufferDirty = false;
	}


	vkCmdPushConstants(m_currentCommandBuffer, m_layout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 4, sizeof(uint32_t) * 1, &localOffset);

	if (m_lastDraw != TERRAIN)
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffer = Terrain::s_vertexBuffer.getBuffer();
		vkCmdBindVertexBuffers(m_currentCommandBuffer, 0, 1, &buffer, offsets);

		buffer = Terrain::s_indexBuffer.getBuffer();
		vkCmdBindIndexBuffer(m_currentCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);

		m_lastDraw = TERRAIN;
	}


	vkCmdDrawIndexed(m_currentCommandBuffer, Terrain::s_numberOfVertices, 1, 0, 0, 0);
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
}
