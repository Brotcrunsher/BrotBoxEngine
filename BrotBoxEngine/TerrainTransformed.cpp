#include "stdafx.h"
#include "BBE/TerrainTransformed.h"
#include "BBE/VulkanDevice.h"
#include "BBE/TimeHelper.h"
#include "BBE/Math.h"

#define VERTICES_PER_DIMENSION (512)
static const float VERTICES_PER_METER = 1;

void bbe::TerrainTransformed::init(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, const INTERNAL::vulkan::VulkanDescriptorPool & descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutHeightMap, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutTexture, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutAdditionalTextures, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutAdditionalTextureWeights) const
{
	if (!m_wasInit)
	{
		m_wasInit = true;

		m_baseTexture.createAndUpload(device, commandPool, descriptorPool, setLayoutTexture);
		m_heightMap.createAndUpload(device, commandPool, descriptorPool, setLayoutHeightMap);

		for (int i = 1; i < m_currentAdditionalTexture; i++)
		{
			m_additionalTextures[i].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures, &(m_additionalTextures[0]));
			m_additionalTextureWeights[i].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures, &(m_additionalTextureWeights[0]));
		}

		m_additionalTextures[0].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures);
		m_additionalTextureWeights[0].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures);
		initIndexBuffer(device, commandPool, device.getQueue());
		initVertexBuffer(device, commandPool, device.getQueue());
	}
}

void bbe::TerrainTransformed::destroy()
{
	m_baseTexture.destroy();
	m_indexBuffer.destroy();
	m_vertexBuffer.destroy();
}

void bbe::TerrainTransformed::initIndexBuffer(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const
{
	int w = VERTICES_PER_DIMENSION;
	int h = VERTICES_PER_DIMENSION;

	List<uint32_t> indices(w * h * 4);
	for (int i = 0; i < w; i++)
	{
		for (int k = 0; k < h; k++)
		{
			uint32_t coord = k + (w + 1) * i;
			indices.add(coord);
			indices.add(coord + w + 1);
		}
		indices.add(0xFFFFFFFF);
	}

	m_indexBuffer.create(device.getDevice(), device.getPhysicalDevice(), sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void *dataBuf = m_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	m_indexBuffer.unmap();

	m_indexBuffer.upload(commandPool, queue);
	m_amountOfIndizes = indices.getLength();
}

void bbe::TerrainTransformed::initVertexBuffer(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const
{
	List<Vector2> vertices;

	int w = VERTICES_PER_DIMENSION;
	int h = VERTICES_PER_DIMENSION;

	for (int i = 0; i <= w; i++)
	{
		for (int k = 0; k <= h; k++)
		{
			vertices.add(Vector2(m_width * (float)k / (float)VERTICES_PER_DIMENSION, m_height * (float)i / (float)VERTICES_PER_DIMENSION));
		}
	}

	m_vertexBuffer.create(device.getDevice(), device.getPhysicalDevice(), sizeof(Vector2) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = m_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(Vector2) * vertices.getLength());
	m_vertexBuffer.unmap();

	m_vertexBuffer.upload(commandPool, queue);
}

void bbe::TerrainTransformed::construct(int width, int height, const char * baseTexturePath, int seed)
{
	m_width = width;
	m_height = height;

	m_valueNoise.create(width, height, seed);
	m_valueNoise.preCalculate();
	m_valueNoise.standardize();


	setTransform(Matrix4());

	m_heightMap.setRepeatMode(bbe::ImageRepeatMode::MIRROR_CLAMP_TO_EDGE);
	m_heightMap.load(width, height, m_valueNoise.getRaw(), bbe::ImageFormat::R32FLOAT);

	m_valueNoise.unload();

	m_baseTexture.load(baseTexturePath);

	m_heightmapScale.x = 1.0f / width;
	m_heightmapScale.y = 1.0f / height;
}

uint32_t bbe::TerrainTransformed::getAmountOfIndizes() const
{
	return m_amountOfIndizes;
}

bbe::TerrainTransformed::TerrainTransformed(int width, int height, const char * baseTexturePath)
{
	construct(width, height, baseTexturePath, (int)TimeHelper::getTimeStamp());
}

bbe::TerrainTransformed::TerrainTransformed(int width, int height, const char * baseTexturePath, int seed)
{
	construct(width, height, baseTexturePath, seed);
}

bbe::TerrainTransformed::~TerrainTransformed()
{
	destroy();
}

bbe::Matrix4 bbe::TerrainTransformed::getTransform() const
{
	return m_transform;
}

void bbe::TerrainTransformed::setTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	setTransform(Matrix4::createTransform(pos, scale, rotationVector, radians));
}

void bbe::TerrainTransformed::setTransform(const Matrix4 & transform)
{
	m_transform = transform;
}

bbe::Vector2 bbe::TerrainTransformed::getBaseTextureOffset()
{
	return m_baseTextureBias.m_textureOffset;
}

void bbe::TerrainTransformed::setBaseTextureOffset(const Vector2 & offset)
{
	m_baseTextureBias.m_textureOffset = offset;
}

bbe::Vector2 bbe::TerrainTransformed::getBaseTextureMult()
{
	return m_baseTextureBias.m_textureMult;
}

void bbe::TerrainTransformed::setBaseTextureMult(const Vector2 & mult)
{
	m_baseTextureBias.m_textureMult = mult;
}

void bbe::TerrainTransformed::setMaxHeight(float height)
{
	m_maxHeight = height;
}

float bbe::TerrainTransformed::getMaxHeight() const
{
	return m_maxHeight;
}

void bbe::TerrainTransformed::addTexture(const char * texturePath, const float * weights)
{
	m_additionalTextures[m_currentAdditionalTexture].load(texturePath);
	m_additionalTextureWeights[m_currentAdditionalTexture].load(m_width, m_height, weights, bbe::ImageFormat::R8);

	m_currentAdditionalTexture++;
}

int bbe::TerrainTransformed::getWidth() const
{
	return m_width;
}

int bbe::TerrainTransformed::getHeight() const
{
	return m_height;
}

bbe::Vector3 bbe::TerrainTransformed::projectOnTerrain(const Vector3 & pos) const
{
	Vector3 TerrainSinglePos = m_transform.extractTranslation();
	Vector3 transformedPos = pos - TerrainSinglePos;
	Vector3 transformedPos2 = transformedPos * VERTICES_PER_METER;
	int indexX = (int)transformedPos2.x;
	int indexY = (int)transformedPos2.y;
	float px = transformedPos2.x - indexX;
	float py = transformedPos2.y - indexY;
	float h1 = m_valueNoise.get(indexX + 0, indexY + 0);
	float h2 = m_valueNoise.get(indexX + 0, indexY + 1);
	float h3 = m_valueNoise.get(indexX + 1, indexY + 0);
	float h4 = m_valueNoise.get(indexX + 1, indexY + 1);

	float w1 = bbe::Math::interpolateLinear(h1, h2, py);
	float w2 = bbe::Math::interpolateLinear(h3, h4, py);

	float w = bbe::Math::interpolateLinear(w1, w2, px);

	return Vector3(pos.x, pos.y, w * getMaxHeight() + TerrainSinglePos.z);
}
