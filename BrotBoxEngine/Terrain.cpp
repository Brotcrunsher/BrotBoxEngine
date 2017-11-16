#include "stdafx.h"
#include "BBE/Terrain.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Random.h"
#include "BBE/Math.h"
#include "BBE/ValueNoise2D.h"
#include "BBE/VulkanBuffer.h"
#include "BBE/TimeHelper.h"

static const int PATCH_SIZE = 256;
static const float VERTICES_PER_METER = 1;

VkDevice         bbe::TerrainPatch::s_device         = VK_NULL_HANDLE;
VkPhysicalDevice bbe::TerrainPatch::s_physicalDevice = VK_NULL_HANDLE;
VkQueue          bbe::TerrainPatch::s_queue          = VK_NULL_HANDLE;
bbe::INTERNAL::vulkan::VulkanCommandPool *bbe::TerrainPatch::s_pcommandPool = nullptr;
static bbe::Random random;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::TerrainPatch::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::TerrainPatch::s_vertexBuffer;

void bbe::TerrainPatch::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_device = device;
	s_physicalDevice = physicalDevice;
	s_queue = queue;
	s_pcommandPool = &commandPool;
	s_initIndexBuffer();
	s_initVertexBuffer();
}

void bbe::TerrainPatch::s_initIndexBuffer()
{
	List<uint32_t> indices;

	indices.add(0);
	indices.add(1);
	indices.add(2);
	indices.add(3);

	s_indexBuffer.create(s_device, s_physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void *dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	s_indexBuffer.unmap();

	s_indexBuffer.upload(*s_pcommandPool, s_queue);
}

void bbe::TerrainPatch::s_initVertexBuffer()
{
	List<Vector2> vertices;
	//TODO change heightmap UV coords

	vertices.add(Vector2(0, 0));
	vertices.add(Vector2(1, 0));
	vertices.add(Vector2(1, 1));
	vertices.add(Vector2(0, 1));

	s_vertexBuffer.create(s_device, s_physicalDevice, sizeof(Vector2) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(Vector2) * vertices.getLength());
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(*s_pcommandPool, s_queue);
}

void bbe::TerrainPatch::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

bbe::TerrainPatch::TerrainPatch(float* data, int patchX, int patchY, int maxPatchX, int maxPatchY)
{
	m_pdata = new float[(PATCH_SIZE + 1) * (PATCH_SIZE + 1)]; //TODO use allocator
	memcpy(m_pdata, data, (PATCH_SIZE + 1) * (PATCH_SIZE + 1) * sizeof(float));

	m_patchTextureWidth = (patchX + 1) / (float)maxPatchX;
	m_patchTextureHeight = (patchY + 1) / (float)maxPatchY;
	m_patchX = patchX / (float)maxPatchX;
	m_patchY = patchY / (float)maxPatchY;

	m_offset = Vector2(patchX * PATCH_SIZE / VERTICES_PER_METER, patchY * PATCH_SIZE / VERTICES_PER_METER);
}

bbe::TerrainPatch::~TerrainPatch()
{
	delete[] m_pdata;
}

bbe::TerrainPatch::TerrainPatch(TerrainPatch && other)
{
	m_pdata            = other.m_pdata;

	m_offset             = other.m_offset;
	m_patchTextureWidth  = other.m_patchTextureWidth;
	m_patchTextureHeight = other.m_patchTextureHeight;
	m_patchX             = other.m_patchX;
	m_patchY             = other.m_patchY;

	m_offset = other.m_offset;

	other.m_pdata = nullptr;
}

void bbe::Terrain::init(
	const INTERNAL::vulkan::VulkanDevice & device, 
	const INTERNAL::vulkan::VulkanCommandPool & commandPool, 
	const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, 
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutHeightMap, 
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutTexture,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutBaseTextureBias,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextures,
	const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayoutAdditionalTextureWeights) const
{
	if (!m_wasInit)
	{
		m_heightMap.createAndUpload(device, commandPool, descriptorPool, setLayoutHeightMap);
		m_wasInit = true;

		m_baseTexture.createAndUpload(device, commandPool, descriptorPool, setLayoutTexture);


		m_baseTextureBiasBuffer.create(device, sizeof(TextureBias), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		m_baseTextureDescriptor.addUniformBuffer(m_baseTextureBiasBuffer, 0, 0);
		m_baseTextureDescriptor.create(device, descriptorPool, setLayoutBaseTextureBias);

		

		for (int i = 1; i < m_currentAdditionalTexture; i++)
		{
			m_additionalTextures[i].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures, &(m_additionalTextures[0]));
			m_additionalTextureWeights[i].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures, &(m_additionalTextureWeights[0]));
		}

		m_additionalTextures[0].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures);
		m_additionalTextureWeights[0].createAndUpload(device, commandPool, descriptorPool, setLayoutAdditionalTextures);

		loadTextureBias();
	}

	
}

void bbe::Terrain::destroy()
{
	m_baseTexture.destroy();
	m_baseTextureBiasBuffer.destroy();
}

void bbe::Terrain::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	TerrainPatch::s_init(device, physicalDevice, commandPool, queue);
}

void bbe::Terrain::s_destroy()
{
	TerrainPatch::s_destroy();
}

void bbe::Terrain::loadTextureBias() const
{
	if (!m_wasInit)
	{
		throw NotInitializedException();
	}

	TextureBias tb = m_baseTextureBias;
	tb.m_textureMult.x *= m_patchesWidthAmount;
	tb.m_textureMult.y *= m_patchesHeightAmount;

	void* data = m_baseTextureBiasBuffer.map();
	memcpy(data, &tb, sizeof(TextureBias));
	m_baseTextureBiasBuffer.unmap();

	m_textureBiasDirty = false;
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
	ValueNoise2D valueNoise;
	valueNoise.create(width, height, seed);

	for (int i = 0; i < m_patchesWidthAmount; i++)
	{
		for (int k = 0; k < m_patchesHeightAmount; k++)
		{
			float data[(PATCH_SIZE + 1) * (PATCH_SIZE + 1)];
			for (int x = 0; x < (PATCH_SIZE + 1); x++)
			{
				for (int y = 0; y < (PATCH_SIZE + 1); y++)
				{
					data[x * (PATCH_SIZE + 1) + y] = valueNoise.get(i * PATCH_SIZE + x, k * PATCH_SIZE + y);
				}
			}
			m_patches.add(TerrainPatch(data, i, k, m_patchesWidthAmount, m_patchesHeightAmount));
		}
	}


	setTransform(Matrix4());

	//m_heightMap.setRepeatMode(bbe::ImageRepeatMode::MIRROR_CLAMP_TO_EDGE);
	//m_heightMap.load(width, height, valueNoise.getRaw(), bbe::ImageFormat::R8);

	m_baseTexture.load(baseTexturePath);

	m_patchSize = PATCH_SIZE / VERTICES_PER_METER;

	m_heightmapScale.x = 1.0f / m_patchesWidthAmount;
	m_heightmapScale.y = 1.0f / m_patchesHeightAmount;
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
	m_textureBiasDirty = true;
}

bbe::Vector2 bbe::Terrain::getBaseTextureMult()
{
	return m_baseTextureBias.m_textureMult;
}

void bbe::Terrain::setBaseTextureMult(const Vector2 & mult)
{
	m_baseTextureBias.m_textureMult = mult;
	m_textureBiasDirty = true;
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

int bbe::Terrain::getWidth() const
{
	return m_width;
}

int bbe::Terrain::getHeight() const
{
	return m_height;
}
