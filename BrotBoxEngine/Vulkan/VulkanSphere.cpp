#include "BBE/Vulkan/VulkanSphere.h"
#include "BBE/List.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Math.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanSphere::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::INTERNAL::vulkan::VulkanSphere::s_vertexBuffer;
uint32_t bbe::INTERNAL::vulkan::VulkanSphere::amountOfVertices = 0;
uint32_t bbe::INTERNAL::vulkan::VulkanSphere::amountOfIndices = 0;

static uint32_t getHalfPointIndex(bbe::List<bbe::Vector3>& vertices, bbe::Vector3& a, bbe::Vector3& b)
{
	bbe::Vector3 halfPoint = (a + b).normalize() / 2;
	for (uint32_t i = 0; i < vertices.getLength(); i++)
	{
		if (halfPoint == vertices[i])
		{
			return i;
		}
	}

	vertices.add(halfPoint);
	return static_cast<uint32_t>(vertices.getLength() - 1);
}

static void createIcoSphereMesh(bbe::List<uint32_t>& indices, bbe::List<bbe::INTERNAL::VertexWithNormal>& vertices, int iterations)
{
	indices.clear();
	vertices.clear();

	float x = (1 + bbe::Math::sqrt(5)) / 4;
	bbe::List<bbe::Vector3> simpleVertices = {
		bbe::Vector3(-0.5,  x,  0).normalize() / 2,
		bbe::Vector3(0.5,  x,  0).normalize() / 2,
		bbe::Vector3(-0.5, -x,  0).normalize() / 2,
		bbe::Vector3(0.5, -x,  0).normalize() / 2,

		bbe::Vector3(0, -0.5,  x).normalize() / 2,
		bbe::Vector3(0,  0.5,  x).normalize() / 2,
		bbe::Vector3(0, -0.5, -x).normalize() / 2,
		bbe::Vector3(0,  0.5, -x).normalize() / 2,

		bbe::Vector3(x,  0, -0.5).normalize() / 2,
		bbe::Vector3(x,  0,  0.5).normalize() / 2,
		bbe::Vector3(-x, 0, -0.5).normalize() / 2,
		bbe::Vector3(-x, 0,  0.5).normalize() / 2,
	};


	indices = {
		5,  11, 0,
		1,  5,  0,
		7,  1,  0,
		10, 7,  0,
		11, 10, 0,

		9, 5,  1,
		4, 11, 5,
		2, 10, 11,
		6, 7,  10,
		8, 1,  7,

		4, 9, 3,
		2, 4, 3,
		6, 2, 3,
		8, 6, 3,
		9, 8, 3,

		5,  9, 4,
		11, 4, 2,
		10, 2, 6,
		7,  6, 8,
		1,  8, 9,
	};

	for (int i = 0; i < iterations; i++)
	{
		bbe::List<uint32_t> newIndices;

		for (size_t k = 0; k < indices.getLength(); k += 3)
		{
			uint32_t a = getHalfPointIndex(simpleVertices, simpleVertices[indices[k + 0]], simpleVertices[indices[k + 1]]);
			uint32_t b = getHalfPointIndex(simpleVertices, simpleVertices[indices[k + 1]], simpleVertices[indices[k + 2]]);
			uint32_t c = getHalfPointIndex(simpleVertices, simpleVertices[indices[k + 2]], simpleVertices[indices[k + 0]]);

			if (iterations % 2 == 0)
			{
				newIndices.addAll(
					c, a, indices[k + 0],
					a, b, indices[k + 1],
					b, c, indices[k + 2],
					c, b, a
				);
			}
			else
			{
				newIndices.addAll(
					a, c, indices[k + 0],
					b, a, indices[k + 1],
					c, b, indices[k + 2],
					b, c, a
				);
			}
		}

		indices = std::move(newIndices);
	}

	for (size_t i = 0; i < simpleVertices.getLength(); i++)
	{
		vertices.add(bbe::INTERNAL::VertexWithNormal(simpleVertices[i], simpleVertices[i]));
	}
}

void bbe::INTERNAL::vulkan::VulkanSphere::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool& commandPool, VkQueue queue)
{
	bbe::List<uint32_t> indices;
	bbe::List<bbe::INTERNAL::VertexWithNormal> vertices;

	createIcoSphereMesh(indices, vertices, 2);

	amountOfIndices = static_cast<uint32_t>(indices.getLength());
	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * amountOfIndices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * amountOfIndices);
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);

	amountOfVertices = static_cast<uint32_t>(vertices.getLength());
	s_vertexBuffer.create(device, physicalDevice, sizeof(INTERNAL::VertexWithNormal) * amountOfVertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(INTERNAL::VertexWithNormal) * amountOfVertices);
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::INTERNAL::vulkan::VulkanSphere::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}
