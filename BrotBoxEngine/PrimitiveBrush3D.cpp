// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Vulkan/VulkanDevice.h"
#include "BBE/Vulkan/VulkanBuffer.h"
#include "BBE/Color.h"
#include "BBE/Math.h"
#include "BBE/Vulkan/VulkanDescriptorPool.h"
#include "BBE/Vulkan/VulkanPipeline.h"
#include "BBE/Vector2.h"
#include "BBE/Matrix4.h"
#include "BBE/Rectangle.h"
#include "BBE/Vulkan/VulkanCommandPool.h"
#include "BBE/Vulkan/VulkanImage.h"
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif

void bbe::PrimitiveBrush3D::INTERNAL_setColor(float r, float g, float b, float a, bool force)
{
	Color c(r, g, b, a);
	if (c.r != m_color.r || c.g != m_color.g || c.b != m_color.b || c.a != m_color.a || force)
	{
		m_prenderManager->setColor3D(c);
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
	int width, int height,
	bbe::RenderManager* renderManager)
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
	m_prenderManager = renderManager;

	INTERNAL_setColor(1.0f, 1.0f, 1.0f, 1.0f, true);
	setCamera(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1));
}

bbe::PrimitiveBrush3D::PrimitiveBrush3D()
{
}

void bbe::PrimitiveBrush3D::fillCube(const Cube & cube)
{
	m_prenderManager->fillCube3D(cube);
}

void bbe::PrimitiveBrush3D::fillIcoSphere(const IcoSphere & sphere)
{
	m_prenderManager->fillSphere3D(sphere);
}

#ifdef BBE_RENDERER_VULKAN
void bbe::PrimitiveBrush3D::drawTerrain(const Terrain& terrain)
{
	((bbe::INTERNAL::vulkan::VulkanManager*)m_prenderManager)->drawTerrain(terrain, m_color);
}
#endif

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

	m_prenderManager->setCamera3D(m_view, m_projection);

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
#endif
