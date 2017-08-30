#include "stdafx.h"
#include "BBE/Terrain.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Random.h"
#include "BBE/Math.h"
#include "BBE/ValueNoise2D.h"


VkDevice         bbe::TerrainPatch::s_device         = VK_NULL_HANDLE;
VkPhysicalDevice bbe::TerrainPatch::s_physicalDevice = VK_NULL_HANDLE;
VkQueue          bbe::TerrainPatch::s_queue          = VK_NULL_HANDLE;
bbe::INTERNAL::vulkan::VulkanCommandPool *bbe::TerrainPatch::s_pcommandPool = nullptr;
const int bbe::Terrain::AMOUNT_OF_LOD_LEVELS = 5;
static bbe::Random random;

void bbe::TerrainPatch::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	s_device = device;
	s_physicalDevice = physicalDevice;
	s_queue = queue;
	s_pcommandPool = &commandPool;
}

void bbe::TerrainPatch::init() const
{
	if (m_created)
	{
		return;
	}

	initIndexBuffer();
	initVertexBuffer();

	m_created = true;
}

void bbe::TerrainPatch::initIndexBuffer() const
{
	int lodWidth = m_width;
	int lodHeight = m_height;
	for (int lod = 0; lod < Terrain::AMOUNT_OF_LOD_LEVELS; lod++)
	{
		List<uint32_t> indices;

		for (int i = 0; i < lodWidth - 1; i++)
		{
			for (int k = 0; k < lodHeight; k++)
			{
				indices.add(k * lodWidth + i);
				indices.add(k * lodWidth + i + 1);
			}
			indices.add(0xFFFFFFFF);
		}

		INTERNAL::vulkan::VulkanBuffer indexBuffer;

		indexBuffer.create(s_device, s_physicalDevice, sizeof(uint32_t) * indices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		void *dataBuf = indexBuffer.map();
		memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * indices.getLength());
		indexBuffer.unmap();

		indexBuffer.upload(*s_pcommandPool, s_queue);

		m_numberOfVertices.add(indices.getLength());

		m_indexBuffers.add(indexBuffer);

		lodWidth /= 2;
		lodHeight /= 2;

		lodWidth += 1;
		lodHeight += 1;
	}
	
}

void bbe::TerrainPatch::initVertexBuffer() const
{
	int lodWidth = m_width;
	int lodHeight = m_height;
	List<float> verticesLast;
	int lodWidthLast = m_width;
	int lodHeightLast = m_height;
	float distMultiplier = 0.5f;

	for (int lod = 0; lod < Terrain::AMOUNT_OF_LOD_LEVELS; lod++)
	{
		List<float> vertices;

		if (lod == 0)
		{
			

			for (int i = 0; i < lodHeight; i++)
			{
				for (int k = 0; k < lodWidth; k++)
				{
					float height = m_pdata[i * lodWidth + k];
					vertices.add(height * 100.0f);
				}
			}
		}
		else
		{
			for (int i = 0; i < lodHeight; i++)
			{
				for (int k = 0; k < lodWidth; k++)
				{
					float height = 0;
					int parentX = k * 2;
					int parentY = i * 2;

					if (k == lodWidth - 1)
					{
						parentX = (k - 1) * 2;
					}
					if (i == lodHeight - 1)
					{
						parentY = (i - 1) * 2;
					}

					float val1 = verticesLast[parentX + parentY * lodHeightLast];
					float val2 = verticesLast[parentX + parentY * lodHeightLast + 1];
					float val3 = verticesLast[parentX + parentY * lodHeightLast + lodHeightLast];
					float val4 = verticesLast[parentX + parentY * lodHeightLast + lodHeightLast + 1];

					/*if ((i & 1) ^ (k & 1))
					{
						height = Math::max(val1, val2, val3, val4);
					}
					else
					{
						height = Math::min(val1, val2, val3, val4);
					}*/

					//height = (val1 + val2 + val3 + val4) / 4;

					if (random.randomBool())
					{
						height = Math::max(val1, val2, val3, val4);
					}
					else
					{
						height = Math::min(val1, val2, val3, val4);
					}

					vertices.add(height);
				}
			}
		}
		

		INTERNAL::vulkan::VulkanBuffer vertexBuffer;

		vertexBuffer.create(s_device, s_physicalDevice, sizeof(float) * vertices.getLength(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

		void* dataBuf = vertexBuffer.map();
		memcpy(dataBuf, vertices.getRaw(), sizeof(float) * vertices.getLength());
		vertexBuffer.unmap();

		vertexBuffer.upload(*s_pcommandPool, s_queue);

		m_vertexBuffers.add(vertexBuffer);
		lodWidthLast = lodWidth;
		lodHeightLast = lodHeight;

		lodWidth /= 2;
		lodHeight /= 2;
		lodWidth += 1;
		lodHeight += 1;
		distMultiplier *= 2;

		verticesLast = std::move(vertices);
	}
	
}

void bbe::TerrainPatch::destroy() const
{
	for (int i = 0; i < Terrain::AMOUNT_OF_LOD_LEVELS; i++)
	{
		m_indexBuffers[i].destroy();
		m_vertexBuffers[i].destroy();
	}
}

bbe::TerrainPatch::TerrainPatch(int width, int height, float* data)
	: m_width(width), m_height(height)
{
	m_pdata = new float[width * height]; //TODO use allocator
	memcpy(m_pdata, data, width * height * sizeof(float));
}

bbe::TerrainPatch::~TerrainPatch()
{
	if (m_needsDestruction)
	{
		destroy();
		delete[] m_pdata;
	}
}

bbe::TerrainPatch::TerrainPatch(TerrainPatch && other)
{
	m_transform        = other.m_transform;

	m_indexBuffers     = other.m_indexBuffers     ;
	m_vertexBuffers    = other.m_vertexBuffers    ;
	m_numberOfVertices = other.m_numberOfVertices ;

	m_width            = other.m_width            ;
	m_height           = other.m_height           ;

	m_created          = other.m_created          ;
	m_needsDestruction = other.m_needsDestruction ;
	m_pdata            = other.m_pdata            ;

	other.m_needsDestruction = false;
}

bbe::Matrix4 bbe::TerrainPatch::getTransform() const
{
	return m_transform;
}

void bbe::TerrainPatch::setTransform(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	m_transform = Matrix4::createTransform(pos, scale, rotationVector, radians);
}

void bbe::TerrainPatch::setTransform(const Matrix4 & transform)
{
	m_transform = transform;
}

void bbe::Terrain::init() const
{
	for (int i = 0; i < m_patches.getLength(); i++)
	{
		m_patches[i].init();
	}
}

void bbe::Terrain::destroy() const
{
	for (int i = 0; i < m_patches.getLength(); i++)
	{
		m_patches[i].destroy();
	}
}

void bbe::Terrain::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	TerrainPatch::s_init(device, physicalDevice, commandPool, queue);
}

bbe::Terrain::Terrain(int width, int height)
{
	if (width % 256 != 0)
	{
		throw IllegalArgumentException();
	}
	if (height % 256 != 0)
	{
		throw IllegalArgumentException();
	}

	m_patchesWidthAmount = width / 256;
	m_patchesHeightAmount = height / 256;
	ValueNoise2D valueNoise;
	valueNoise.create(width, height);

	for (int i = 0; i < m_patchesWidthAmount; i++)
	{
		for (int k = 0; k < m_patchesHeightAmount; k++)
		{
			float data[257 * 257];
			for (int x = 0; x < 257; x++)
			{
				for (int y = 0; y < 257; y++)
				{
					data[x * 257 + y] = valueNoise.get(i * 256 + x, k * 256 + y);
				}
			}
			m_patches.add(TerrainPatch(257, 257, data));
		}
	}

	setTransform(Matrix4());
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

	for(int i = 0; i<m_patchesHeightAmount; i++)
	{
		for(int k = 0; k<m_patchesWidthAmount; k++)
		{
			int index = i * m_patchesWidthAmount + k;
			m_patches[index].setTransform(Matrix4::createTranslationMatrix(Vector3(i * 128.f, k * 128.f, 0)) * m_transform);
		}
	}
}
