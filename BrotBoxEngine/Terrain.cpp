#include "BBE/Terrain.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Random.h"
#include "BBE/Math.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/TimeHelper.h"

static const int PATCH_SIZE = 64;
static const float VERTICES_PER_METER = 1;

void bbe::Terrain::init(
	const INTERNAL::vulkan::VulkanDevice & device,
	const INTERNAL::vulkan::VulkanCommandPool & commandPool,
	const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutHeightMap,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutTexture,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextures,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextureWeights,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutViewFrustrums) const
{
	if (!m_wasInit)
	{
		m_heightMap.createAndUpload(device, commandPool, descriptorPool, setLayoutHeightMap);
		m_wasInit = true;

		m_baseTexture.createAndUpload(device, commandPool, descriptorPool, setLayoutTexture);

		m_viewFrustrumBuffer.create(device, sizeof(bbe::Vector4) * 5, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_viewFrustrumDescriptor.addUniformBuffer(m_viewFrustrumBuffer, 0, 0);
		m_viewFrustrumDescriptor.create(device, descriptorPool, setLayoutViewFrustrums);


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

void bbe::Terrain::destroy()
{
	m_baseTexture.destroy();
	m_indexBuffer.destroy();
	m_vertexBuffer.destroy();
	m_viewFrustrumBuffer.destroy();
	for (int i = 0; i < m_currentAdditionalTexture; i++)
	{
		m_additionalTextures[i].destroy();
		m_additionalTextureWeights[i].destroy();
	}
}

void bbe::Terrain::initIndexBuffer(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const
{
	int w = m_width / PATCH_SIZE;
	int h = m_height / PATCH_SIZE;

	List<uint32_t> indices (w * h * 4);
	for (int i = 0; i < w; i++)
	{
		for (int k = 0; k < h; k++)
		{
			uint32_t coord = i * (w + 1) + k;
			indices.add(coord);
			indices.add(coord + 1);
			indices.add(coord + w + 2);
			indices.add(coord + w + 1);
		}
	}

	m_indexBuffer.create(device.getDevice(), device.getPhysicalDevice(), sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void *dataBuf = m_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	m_indexBuffer.unmap();

	m_indexBuffer.upload(commandPool, queue);
	m_amountOfIndizes = indices.getLength();
}

void bbe::Terrain::initVertexBuffer(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue) const
{
	List<Vector2> vertices;

	int w = m_width / PATCH_SIZE;
	int h = m_height / PATCH_SIZE;

	for (int i = 0; i <= w; i++)
	{
		for (int k = 0; k <= h; k++)
		{
			vertices.add(Vector2(k, i));
		}
	}

	m_vertexBuffer.create(device.getDevice(), device.getPhysicalDevice(), sizeof(Vector2) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = m_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(Vector2) * vertices.getLength());
	m_vertexBuffer.unmap();

	m_vertexBuffer.upload(commandPool, queue);
}

void bbe::Terrain::loadViewFrustrum(const bbe::Matrix4 &mvpMat, const bbe::INTERNAL::vulkan::VulkanDevice &device) const
{
	if (!m_wasInit)
	{
		throw NotInitializedException();
	}

	m_viewFrustum.updatePlanes(mvpMat);
	void *data = m_viewFrustrumBuffer.map();
	memcpy(data, m_viewFrustum.getPlanes(), sizeof(bbe::Vector4) * 5);
	m_viewFrustrumBuffer.unmap();
	m_viewFrustrumDescriptor.update(device);
}

void bbe::Terrain::construct(int width, int height, const char * baseTexturePath, int seed)
{
	m_width = width;
	m_height = height;

	if (width % PATCH_SIZE != 0)
	{
		throw IllegalArgumentException();
	}
	if (height % PATCH_SIZE != 0)
	{
		throw IllegalArgumentException();
	}

	m_patchesWidthAmount = width / PATCH_SIZE;
	m_patchesHeightAmount = height / PATCH_SIZE;
	m_valueNoise.create(width, height, seed);
	m_valueNoise.preCalculate();
	m_valueNoise.standardize();


	setTransform(Matrix4());

	m_heightMap.setRepeatMode(bbe::ImageRepeatMode::MIRROR_CLAMP_TO_EDGE);
	m_heightMap.load(width, height, m_valueNoise.getRaw(), bbe::ImageFormat::R32FLOAT);

	m_valueNoise.unload();

	m_baseTexture.load(baseTexturePath);

	m_patchSize = PATCH_SIZE / VERTICES_PER_METER;

	m_heightmapScale.x = 1.0f / m_patchesWidthAmount;
	m_heightmapScale.y = 1.0f / m_patchesHeightAmount;
}

uint32_t bbe::Terrain::getAmountOfIndizes() const
{
	return m_amountOfIndizes;
}

bbe::Terrain::Terrain(int width, int height, const char* baseTexturePath)
{
	construct(width, height, baseTexturePath, (int)TimeHelper::getTimeStamp());
}

bbe::Terrain::Terrain(int width, int height, const char * baseTexturePath, int seed)
{
	construct(width, height, baseTexturePath, seed);
}

bbe::Terrain::~Terrain()
{
	destroy();
}

bbe::Matrix4 bbe::Terrain::getTransform() const
{
	return m_transform;
}

void bbe::Terrain::setTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	setTransform(Matrix4::createTransform(pos, scale, rotationVector, radians));
}

void bbe::Terrain::setTransform(const Matrix4 & transform)
{
	m_transform = transform;
}

bbe::Vector2 bbe::Terrain::getBaseTextureOffset()
{
	return m_baseTextureBias.m_textureOffset;
}

void bbe::Terrain::setBaseTextureOffset(const Vector2 & offset)
{
	m_baseTextureBias.m_textureOffset = offset;
}

bbe::Vector2 bbe::Terrain::getBaseTextureMult()
{
	return m_baseTextureBias.m_textureMult;
}

void bbe::Terrain::setBaseTextureMult(const Vector2 & mult)
{
	m_baseTextureBias.m_textureMult = mult;
}

void bbe::Terrain::setMaxHeight(float height)
{
	m_maxHeight = height;
}

float bbe::Terrain::getMaxHeight() const
{
	return m_maxHeight;
}

void bbe::Terrain::addTexture(const char * texturePath, const float * weights)
{
	m_additionalTextures[m_currentAdditionalTexture].load(texturePath);
	m_additionalTextureWeights[m_currentAdditionalTexture].load(m_width, m_height, weights, bbe::ImageFormat::R8);

	m_currentAdditionalTexture++;
}

bbe::Vector3 bbe::Terrain::projectOnTerrain(const Vector3 & pos) const
{
	Vector3 TerrainPos = m_transform.extractTranslation();
	Vector3 transformedPos = pos - TerrainPos;
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

	return Vector3(pos.x, pos.y, w * getMaxHeight() + TerrainPos.z);
}

int bbe::Terrain::getWidth() const
{
	return m_width;
}

int bbe::Terrain::getHeight() const
{
	return m_height;
}
