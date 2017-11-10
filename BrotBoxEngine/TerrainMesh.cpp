#include "stdafx.h"
#include "BBE/TerrainMesh.h"
#include "BBE/TimeHelper.h"
#include "BBE/ValueNoise2D.h"
#include "BBE/Random.h"

static const int PATCH_SIZE = 256;
static const float VERTICES_PER_METER = 2;

VkDevice         bbe::TerrainMeshPatch::s_device = VK_NULL_HANDLE;
VkPhysicalDevice bbe::TerrainMeshPatch::s_physicalDevice = VK_NULL_HANDLE;
VkQueue          bbe::TerrainMeshPatch::s_queue = VK_NULL_HANDLE;
bbe::INTERNAL::vulkan::VulkanCommandPool *bbe::TerrainMeshPatch::s_pcommandPool = nullptr;
static bbe::Random random;

void bbe::TerrainMesh::init(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, const INTERNAL::vulkan::VulkanDescriptorPool & descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutHeightMap, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutTexture, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutBaseTextureBias, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutAdditionalTextures, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutAdditionalTextureWeights) const
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

		for (int i = 0; i < m_patches.getLength(); i++)
		{
			m_patches[i].init(getMaxHeight());
		}
	}
}

void bbe::TerrainMesh::destroy()
{
	m_baseTexture.destroy();
	m_baseTextureBiasBuffer.destroy();
	for (int i = 0; i < m_patches.getLength(); i++)
	{
		m_patches[i].destroy();
	}
}

void bbe::TerrainMesh::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	TerrainMeshPatch::s_init(device, physicalDevice, commandPool, queue);
}

void bbe::TerrainMesh::loadTextureBias() const
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

void bbe::TerrainMesh::construct(int width, int height, const char * baseTexturePath, int seed)
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
			m_patches.add(TerrainMeshPatch(data, i, k));
		}
	}


	setTransform(Matrix4());

	m_heightMap.setRepeatMode(bbe::ImageRepeatMode::MIRROR_CLAMP_TO_EDGE);
	m_heightMap.load(width, height, valueNoise.getRaw(), bbe::ImageFormat::R8);

	m_baseTexture.load(baseTexturePath);

	m_patchSize = PATCH_SIZE / VERTICES_PER_METER;

	m_heightmapScale.x = 1.0f / m_patchesWidthAmount;
	m_heightmapScale.y = 1.0f / m_patchesHeightAmount;
}

bbe::TerrainMesh::TerrainMesh(int width, int height, const char * baseTexturePath)
{
	construct(width, height, baseTexturePath, (int)TimeHelper::getTimeStamp());
}

bbe::TerrainMesh::TerrainMesh(int width, int height, const char * baseTexturePath, int seed)
{
	construct(width, height, baseTexturePath, seed);
}

bbe::TerrainMesh::~TerrainMesh()
{
	destroy();
}

bbe::Matrix4 bbe::TerrainMesh::getTransform() const
{
	return m_transform;
}

void bbe::TerrainMesh::setTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	setTransform(Matrix4::createTransform(pos, scale, rotationVector, radians));
}

void bbe::TerrainMesh::setTransform(const Matrix4 & transform)
{
	m_transform = transform;
}

bbe::Vector2 bbe::TerrainMesh::getBaseTextureOffset()
{
	return m_baseTextureBias.m_textureOffset;
}

void bbe::TerrainMesh::setBaseTextureOffset(const Vector2 & offset)
{
	m_baseTextureBias.m_textureOffset = offset;
	m_textureBiasDirty = true;
}

bbe::Vector2 bbe::TerrainMesh::getBaseTextureMult()
{
	return m_baseTextureBias.m_textureMult;
}

void bbe::TerrainMesh::setBaseTextureMult(const Vector2 & mult)
{
	m_baseTextureBias.m_textureMult = mult;
	m_textureBiasDirty = true;
}

void bbe::TerrainMesh::setMaxHeight(float height)
{
	m_maxHeight = height;
}

float bbe::TerrainMesh::getMaxHeight() const
{
	return m_maxHeight;
}

void bbe::TerrainMesh::addTexture(const char * texturePath, const float * weights)
{
	m_additionalTextures[m_currentAdditionalTexture].load(texturePath);
	m_additionalTextureWeights[m_currentAdditionalTexture].load(m_width, m_height, weights, bbe::ImageFormat::R8);

	m_currentAdditionalTexture++;
}

int bbe::TerrainMesh::getWidth() const
{
	return m_width;
}

int bbe::TerrainMesh::getHeight() const
{
	return m_height;
}

void bbe::TerrainMeshPatch::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_device = device;
	s_physicalDevice = physicalDevice;
	s_queue = queue;
	s_pcommandPool = &commandPool;
}


void bbe::TerrainMeshPatch::init(float height) const
{
	initIndexBuffer();
	initVertexBuffer(height);
}

void bbe::TerrainMeshPatch::initIndexBuffer() const
{
	List<uint32_t> indices;

	for (int i = 0; i < PATCH_SIZE; i++)
	{
		for (int k = 0; k < PATCH_SIZE; k++)
		{
			uint32_t coord = i * (PATCH_SIZE + 1) + k;
			indices.add(coord);
			indices.add(coord + PATCH_SIZE + 1);
			indices.add(coord + 1);
			indices.add(coord + PATCH_SIZE + 2);
		}
		indices.add(0xFFFFFFFF);
	}

	m_indexBuffer.create(s_device, s_physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void *dataBuf = m_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
	m_indexBuffer.unmap();

	m_indexBuffer.upload(*s_pcommandPool, s_queue);

	m_indexCount = indices.getLength();
}

void bbe::TerrainMeshPatch::initVertexBuffer(float height) const
{
	List<Vector3> vertices;

	const float OFFSETX = PATCH_SIZE / VERTICES_PER_METER * m_patchX;
	const float OFFSETY = PATCH_SIZE / VERTICES_PER_METER * m_patchY;

	for (int i = 0; i < PATCH_SIZE + 1; i++)
	{
		for (int k = 0; k < PATCH_SIZE + 1; k++)
		{
			vertices.add(
				Vector3(
					OFFSETX + k / VERTICES_PER_METER, 
					OFFSETY + i / VERTICES_PER_METER, 
					m_pdata[k * (PATCH_SIZE + 1) + i] * height)
			);
		}
	}

	m_vertexBuffer.create(s_device, s_physicalDevice, sizeof(Vector3) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	void* dataBuf = m_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(Vector3) * vertices.getLength());
	m_vertexBuffer.unmap();

	m_vertexBuffer.upload(*s_pcommandPool, s_queue);
}

void bbe::TerrainMeshPatch::destroy() const
{
	m_vertexBuffer.destroy();
	m_indexBuffer.destroy();
}

bbe::TerrainMeshPatch::TerrainMeshPatch(float * data, int patchX, int patchY)
{
	m_pdata = new float[(PATCH_SIZE + 1) * (PATCH_SIZE + 1)]; //TODO use allocator
	memcpy(m_pdata, data, (PATCH_SIZE + 1) * (PATCH_SIZE + 1) * sizeof(float));

	m_patchX = patchX;
	m_patchY = patchY;
}

bbe::TerrainMeshPatch::~TerrainMeshPatch()
{
	delete[] m_pdata;
}

bbe::TerrainMeshPatch::TerrainMeshPatch(TerrainMeshPatch && other)
{
	m_pdata            = other.m_pdata;
	m_patchX = other.m_patchX;
	m_patchY = other.m_patchY;

	m_indexCount = other.m_indexCount;

	other.m_pdata = nullptr;

	m_indexBuffer = other.m_indexBuffer;
}
