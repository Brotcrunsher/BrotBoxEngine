#include "BBE/Model.h"
#include "BBE/DynamicArray.h"
#include "BBE/List.h"
#include "BBE/HashMap.h"

bbe::Model::Model()
{
	// Do nothing.
}

bbe::Model::Model(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices) :
	m_vertices(vertices),
	m_indices(indices)
{
}

void bbe::Model::add(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices)
{
	m_vertices.addList(vertices);
	m_indices.addList(indices);
}

size_t bbe::Model::getAmountOfIndices() const
{
	return m_indices.getLength();
}

size_t bbe::Model::getAmountOfVertices() const
{
	return m_vertices.getLength();
}

bbe::Model bbe::Model::finalize(const bbe::Vector2i& textureDimensions) const
{
	bbe::Model copy = *this;
	for (bbe::PosNormalPair& vertex : copy.m_vertices)
	{
		vertex.uvCoord.x /= textureDimensions.x;
		vertex.uvCoord.y /= textureDimensions.y;
	}

	for (bbe::Vector2& bakingPos : copy.m_bakingUvs)
	{
		bakingPos.x /= textureDimensions.x;
		bakingPos.y /= textureDimensions.y;
	}

	return copy;
}

bbe::Model bbe::Model::toBakingModel() const
{
	if (m_bakingUvs.getLength() != m_vertices.getLength())
	{
		// The Model wasn't properly prepared for baking.
		throw bbe::IllegalStateException();
	}
	bbe::Model copy = *this;
	for (size_t i = 0; i < m_bakingUvs.getLength(); i++)
	{
		copy.m_vertices[i].uvCoord = m_bakingUvs[i];
	}
	return copy;
}

bbe::Model bbe::Model::fromObj(const bbe::String& obj)
{
	const bbe::DynamicArray<bbe::String> lines = obj.split("\n", false);
	bbe::List<bbe::Vector3> rawVertices;
	bbe::List<bbe::Vector3> rawNormals;
	bbe::List<bbe::Vector2> rawUvCoords;
	bbe::List<bbe::DynamicArray<bbe::String>> rawFaces; // Must be processed later so that we can be sure that all vertices and normals have been read and we can maybe discard some of them.

	bbe::List<bbe::String> ignoredTokens;

	for (bbe::String line : lines)
	{
		line = line.trim();
		if (line.startsWith("#")) continue;
		if (line.getLength() == 0) continue;

		const bbe::DynamicArray<bbe::String> tokens = line.split(" ", false);
		if (tokens[0] == "v") // tokens[0] is save to query because we know that the line wasn't empty.
		{
			if (tokens.getLength() < 4) throw IllegalStateException();
			rawVertices.add(bbe::Vector3(
				tokens[1].toFloat(),
				tokens[3].toFloat(),
				tokens[2].toFloat()
			));
		}
		else if (tokens[0] == "vn")
		{
			if (tokens.getLength() < 4) throw IllegalStateException();
			rawNormals.add(bbe::Vector3(
				tokens[1].toFloat(),
				tokens[3].toFloat(),
				tokens[2].toFloat()
			));
		}
		else if (tokens[0] == "vt")
		{
			if (tokens.getLength() < 3) throw IllegalStateException();
			rawUvCoords.add(bbe::Vector2(
				tokens[1].toFloat(),
				tokens[2].toFloat()
			));
		}
		else if (tokens[0] == "f")
		{
			rawFaces.add(tokens);
		}
		else
		{
			if (!ignoredTokens.contains(tokens[0]))
			{
				ignoredTokens.add(tokens[0]);
				std::cout << "Warning: Did not interpret obj element that started with " << tokens[0] << std::endl;
			}
		}
	}

	bbe::List<bbe::PosNormalPair> vertices;
	bbe::List<uint32_t> indices;
	bbe::HashMap<bbe::String, int32_t> seenFaceVertices;

	size_t REMOVE_ME_DEBUG_COUNTER = (size_t)-1;
	for (const bbe::DynamicArray<bbe::String>& face : rawFaces)
	{
		REMOVE_ME_DEBUG_COUNTER++;
		if (REMOVE_ME_DEBUG_COUNTER == 5)
		{
			int a = 0;
		}
		// A face with less than 3 (4 with the prefix) tokens doesn't make sense.
		if (face.getLength() < 4) throw IllegalStateException();

		for (size_t i = 1; i<face.getLength(); i++)
		{
			const bbe::String& vertexInfo = face[i];
			if (i >= 4)
			{
				// Prepare a triangle with the previous indices
				indices.add(indices.last());
				indices.add(indices[indices.getLength() - 4]);
			}
			if (int32_t* index = seenFaceVertices.get(vertexInfo))
			{
				indices.add(*index);
			}
			else
			{
				// "7/9/1" => ["7", "9", "1]
				const DynamicArray<bbe::String>& subTokens = vertexInfo.split("/", true);
				// TODO: The obj file format also allows fewer indizes, but we don't support them right now.
				if (subTokens.getLength() < 3) throw IllegalStateException();
				vertices.add({ 
						rawVertices[subTokens[0].toLong() - 1], 
						rawNormals [subTokens[2].toLong() - 1],
						rawUvCoords[subTokens[1].toLong() - 1]
					});
				seenFaceVertices.add(vertexInfo, vertices.getLength() - 1);
				indices.add(vertices.getLength() - 1);
			}
		}
	}

	return Model(vertices, indices);
}
