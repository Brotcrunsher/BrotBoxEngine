#include "stdafx.h"
#include "BBE/IcoSphere.h"
#include "BBE/VertexWithNormal.h"
#include "BBE/Math.h"
#include "BBE/List.h"
#include <string.h>

bbe::INTERNAL::vulkan::VulkanBuffer bbe::IcoSphere::s_indexBuffer;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::IcoSphere::s_vertexBuffer;
size_t bbe::IcoSphere::amountOfVertices = 0;
size_t bbe::IcoSphere::amountOfIndices = 0;

size_t getHalfPointIndex(bbe::List<bbe::Vector3> &vertices, bbe::Vector3 &a, bbe::Vector3 &b)
{
	bbe::Vector3 halfPoint = (a + b).normalize() / 2;
	for (size_t i = 0; i < vertices.getLength(); i++)
	{
		if (halfPoint == vertices[i])
		{
			return i;
		}
	}

	vertices.add(halfPoint);
	return vertices.getLength() - 1;
}

void createIcoSphereMesh(bbe::List<uint32_t> &indices, bbe::List<bbe::VertexWithNormal> &vertices, int iterations)
{
	indices.clear();
	vertices.clear();

	float x = (1 + bbe::Math::sqrt(5)) / 4;
	bbe::List<bbe::Vector3> simpleVertices = {
		bbe::Vector3(-0.5,  x,  0).normalize() / 2,
		bbe::Vector3( 0.5,  x,  0).normalize() / 2,
		bbe::Vector3(-0.5, -x,  0).normalize() / 2,
		bbe::Vector3( 0.5, -x,  0).normalize() / 2,

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
			int a = getHalfPointIndex(simpleVertices, simpleVertices[indices[k + 0]], simpleVertices[indices[k + 1]]);
			int b = getHalfPointIndex(simpleVertices, simpleVertices[indices[k + 1]], simpleVertices[indices[k + 2]]);
			int c = getHalfPointIndex(simpleVertices, simpleVertices[indices[k + 2]], simpleVertices[indices[k + 0]]);

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
		vertices.add(bbe::VertexWithNormal(simpleVertices[i], simpleVertices[i]));
	}

}

void bbe::IcoSphere::s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool & commandPool, VkQueue queue)
{
	bbe::List<uint32_t> indices;
	bbe::List<bbe::VertexWithNormal> vertices;

	createIcoSphereMesh(indices, vertices, 2);

	amountOfIndices = indices.getLength();
	s_indexBuffer.create(device, physicalDevice, sizeof(uint32_t) * amountOfIndices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	void* dataBuf = s_indexBuffer.map();
	memcpy(dataBuf, indices.getRaw(), sizeof(uint32_t) * amountOfIndices);
	s_indexBuffer.unmap();

	s_indexBuffer.upload(commandPool, queue);

	amountOfVertices = vertices.getLength();
	s_vertexBuffer.create(device, physicalDevice, sizeof(VertexWithNormal) * amountOfVertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	dataBuf = s_vertexBuffer.map();
	memcpy(dataBuf, vertices.getRaw(), sizeof(VertexWithNormal) * amountOfVertices);
	s_vertexBuffer.unmap();

	s_vertexBuffer.upload(commandPool, queue);
}

void bbe::IcoSphere::s_destroy()
{
	s_indexBuffer.destroy();
	s_vertexBuffer.destroy();
}

bbe::IcoSphere::IcoSphere()
{
}

bbe::IcoSphere::IcoSphere(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	set(pos, scale, rotationVector, radians);
}

bbe::IcoSphere::IcoSphere(const Matrix4 & transform)
{
	m_transform = transform;
}

void bbe::IcoSphere::set(const Vector3 & pos, const Vector3 & scale, const Vector3 & rotationVector, float radians)
{
	Matrix4 matTranslation = Matrix4::createTranslationMatrix(pos);
	Matrix4 matScale = Matrix4::createScaleMatrix(scale);
	Matrix4 matRotation = Matrix4::createRotationMatrix(radians, rotationVector);

	m_transform = matTranslation * matRotation * matScale;
}

bbe::Vector3 bbe::IcoSphere::getPos() const
{
	return m_transform.extractTranslation();
}

float bbe::IcoSphere::getX() const
{
	return getPos().x;
}

float bbe::IcoSphere::getY() const
{
	return getPos().y;
}

float bbe::IcoSphere::getZ() const
{
	return getPos().z;
}

bbe::Vector3 bbe::IcoSphere::getScale() const
{
	return m_transform.extractScale();
}

float bbe::IcoSphere::getWidth() const
{
	return getScale().x;
}

float bbe::IcoSphere::getHeight() const
{
	return getScale().z;
}

float bbe::IcoSphere::getDepth() const
{
	return getScale().y;
}

bbe::Matrix4 bbe::IcoSphere::getTransform() const
{
	return m_transform;
}

