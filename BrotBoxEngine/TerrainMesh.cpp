#include "stdafx.h"
#include "BBE/TerrainMesh.h"
#include "BBE/TimeHelper.h"
#include "BBE/ValueNoise2D.h"
#include "BBE/Random.h"
#include "BBE/Math.h"
#include "BBE/VulkanDevice.h"

static const int PATCH_SIZE = 256;
static const float VERTICES_PER_METER = 2;

VkDevice         bbe::TerrainMeshPatch::s_device = VK_NULL_HANDLE;
VkPhysicalDevice bbe::TerrainMeshPatch::s_physicalDevice = VK_NULL_HANDLE;
VkQueue          bbe::TerrainMeshPatch::s_queue = VK_NULL_HANDLE;
bbe::INTERNAL::vulkan::VulkanCommandPool *bbe::TerrainMeshPatch::s_pcommandPool = nullptr;
bbe::List<bbe::INTERNAL::vulkan::VulkanBuffer> bbe::TerrainMeshPatch::s_indexBuffer;
bbe::List<int> bbe::TerrainMeshPatch::s_indexCount;
static bbe::Random random;

void bbe::TerrainMesh::init(const INTERNAL::vulkan::VulkanDevice & device, const INTERNAL::vulkan::VulkanCommandPool & commandPool, const INTERNAL::vulkan::VulkanDescriptorPool & descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutHeightMap, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutTexture, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutBaseTextureBias, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutAdditionalTextures, const INTERNAL::vulkan::VulkanDescriptorSetLayout & setLayoutAdditionalTextureWeights) const
{
	if (!m_wasInit)
	{
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
		int amountOfVerticesPerPatch = getAmountOfVerticesPerPatch();
		int amountOfLoDs = getAmountOfLoDs();
		bbe::INTERNAL::vulkan::VulkanBuffer alignmentRetrievalBuffer;
		alignmentRetrievalBuffer.preCreate(device, (PATCH_SIZE + 1) * (PATCH_SIZE + 1) * sizeof(Vector3), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(device.getDevice(), alignmentRetrievalBuffer.getBuffer(), &memoryRequirements);
		VkDeviceSize alignment = memoryRequirements.alignment;
		alignmentRetrievalBuffer.destroy();
		VkDeviceSize bufferSize = amountOfVerticesPerPatch * sizeof(Vector3) * m_patchesWidthAmount * m_patchesHeightAmount + m_patchesWidthAmount * m_patchesHeightAmount * amountOfLoDs * alignment;
		
		

		m_meshBuffer.create(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize offset = 0;
		for (int i = 0; i < m_patches.getLength(); i++)
		{
			m_patches[i].init(getMaxHeight(), m_meshBuffer, offset, alignment);
		}
		int i = 0;
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

void bbe::TerrainMesh::s_destroy()
{
	TerrainMeshPatch::s_destroy();
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

	m_baseTexture.load(baseTexturePath);

	m_patchSize = PATCH_SIZE / VERTICES_PER_METER;

	m_heightmapScale.x = 1.0f / m_patchesWidthAmount;
	m_heightmapScale.y = 1.0f / m_patchesHeightAmount;
}

int bbe::TerrainMesh::getAmountOfVerticesPerPatch() const
{
	//TODO calculate the following using a formula
	int amountOfVerticesPerPatch = 0;
	int size = PATCH_SIZE;
	while (size > 0)
	{
		amountOfVerticesPerPatch += (size + 1) * (size + 1);
		size /= 2;
	}
	return amountOfVerticesPerPatch;
}

int bbe::TerrainMesh::getAmountOfLoDs() const
{
	//TODO calculate the following using a formula
	int amount = 0;
	int size = PATCH_SIZE;
	while (size > 0)
	{
		amount++;
		size /= 2;
	}
	return amount;
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

float bbe::TerrainMesh::s_getVerticesPerMeter()
{
	return VERTICES_PER_METER;
}

void bbe::TerrainMeshPatch::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_device = device;
	s_physicalDevice = physicalDevice;
	s_queue = queue;
	s_pcommandPool = &commandPool;

	s_initIndexBuffer();
}


void bbe::TerrainMeshPatch::init(float height, bbe::INTERNAL::vulkan::VulkanBuffer &parentBuffer, VkDeviceSize &offset, VkDeviceSize alignment) const
{
	initVertexBuffer(height, parentBuffer, offset, alignment);
}

void bbe::TerrainMeshPatch::s_initIndexBuffer()
{
	int size = PATCH_SIZE;
	while (size > 0)
	{
		List<uint32_t> indices;

		for (int i = 0; i < size; i++)
		{
			for (int k = 0; k < size; k++)
			{
				uint32_t coord = i * (size + 1) + k;
				indices.add(coord);
				indices.add(coord + size + 1);
				indices.add(coord + 1);
				indices.add(coord + size + 2);
			}
			indices.add(0xFFFFFFFF);
		}

		bbe::INTERNAL::vulkan::VulkanBuffer localBuff;

		localBuff.create(s_device, s_physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		void *dataBuf = localBuff.map();
		memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
		localBuff.unmap();

		localBuff.upload(*s_pcommandPool, s_queue);

		s_indexCount.add(indices.getLength());
		s_indexBuffer.add(localBuff);

		size /= 2;
	}
	
}

void bbe::TerrainMeshPatch::initVertexBuffer(float height, bbe::INTERNAL::vulkan::VulkanBuffer &parentBuffer, VkDeviceSize &offset, VkDeviceSize alignment) const
{
	int size = PATCH_SIZE;
	int prevSize = PATCH_SIZE;
	float verticesPerMeter = VERTICES_PER_METER;
	float* lodData = new float[(PATCH_SIZE + 1) * (PATCH_SIZE + 1)];
	memcpy(lodData, m_pdata, (PATCH_SIZE + 1) * (PATCH_SIZE + 1) * sizeof(float));
	int additionalOffset = 0;


	while (size > 0)
	{
		if (size != PATCH_SIZE) //NOT first iteration
		{
			for (int k = 0; k < size + 1; k++)
			{
				for (int i = 0; i < size + 1; i++)
				{
					lodData[k * (size + 1) + i] = lodData[k * 2 * (prevSize + 1) + i * 2];
				}
			}
		}
		List<Vector3> vertices;

		const float OFFSETX = size / verticesPerMeter * m_patchX;
		const float OFFSETY = size / verticesPerMeter * m_patchY;

		for (int i = 0; i < size + 1; i++)
		{
			for (int k = 0; k < size + 1; k++)
			{
				vertices.add(
					Vector3(
						OFFSETX + k / verticesPerMeter,
						OFFSETY + i / verticesPerMeter,
						lodData[k * (size + 1) + i] * height)
				);
			}
		}


		bbe::INTERNAL::vulkan::VulkanBuffer localBuff;
		
		localBuff.create(s_device, s_physicalDevice, sizeof(Vector3) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		void* dataBuf = localBuff.map();
		memcpy(dataBuf, vertices.getRaw(), sizeof(Vector3) * vertices.getLength());
		localBuff.unmap();

		offset = bbe::Math::nextMultiple<VkDeviceSize>(alignment, offset);
		localBuff.upload(*s_pcommandPool, s_queue, parentBuffer, offset);
		offset += sizeof(Vector3) * vertices.getLength();

		m_vertexBuffer.add(localBuff);
		prevSize = size;
		additionalOffset += size;
		size /= 2;
		verticesPerMeter /= 2.0f;
	}
	
	delete[] lodData;
}

void bbe::TerrainMeshPatch::destroy() const
{
	for (int i = 0; i < m_vertexBuffer.getLength(); i++)
	{
		m_vertexBuffer[i].destroy();
	}
}

void bbe::TerrainMeshPatch::s_destroy()
{
	for (int i = 0; i < s_indexBuffer.getLength(); i++)
	{
		s_indexBuffer[i].destroy();
	}
}

bbe::Vector2 bbe::TerrainMeshPatch::getOffset() const
{
	return Vector2(
		PATCH_SIZE / VERTICES_PER_METER * m_patchX,
		PATCH_SIZE / VERTICES_PER_METER * m_patchY
	);
}

float bbe::TerrainMeshPatch::getSize() const
{
	return PATCH_SIZE / VERTICES_PER_METER;
}

int bbe::TerrainMeshPatch::getMaxLod() const
{
	return m_vertexBuffer.getLength() - 1;
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
	m_pdata  = other.m_pdata;
	m_patchX = other.m_patchX;
	m_patchY = other.m_patchY;

	other.m_pdata = nullptr;
}
