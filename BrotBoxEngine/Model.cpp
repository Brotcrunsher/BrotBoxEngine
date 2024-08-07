#include "BBE/Model.h"
#include "BBE/DynamicArray.h"
#include "BBE/List.h"
#include "BBE/HashMap.h"
#include "BBE/Logging.h"

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
		bbe::Crash(bbe::Error::IllegalState);
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
		if (line.isEmpty()) continue;

		const bbe::DynamicArray<bbe::String> tokens = line.split(" ", false);
		if (tokens[0] == "v") // tokens[0] is save to query because we know that the line wasn't empty.
		{
			if (tokens.getLength() < 4) bbe::Crash(bbe::Error::IllegalState);
			rawVertices.add(bbe::Vector3(
				tokens[1].toFloat(),
				tokens[3].toFloat(),
				tokens[2].toFloat()
			));
		}
		else if (tokens[0] == "vn")
		{
			if (tokens.getLength() < 4) bbe::Crash(bbe::Error::IllegalState);
			rawNormals.add(bbe::Vector3(
				tokens[1].toFloat(),
				tokens[3].toFloat(),
				tokens[2].toFloat()
			));
		}
		else if (tokens[0] == "vt")
		{
			if (tokens.getLength() < 3) bbe::Crash(bbe::Error::IllegalState);
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
				BBELOGLN("Warning: Did not interpret obj element that started with " << tokens[0]);
			}
		}
	}

	bbe::List<bbe::PosNormalPair> vertices;
	bbe::List<uint32_t> indices;
	bbe::HashMap<bbe::String, int32_t> seenFaceVertices;

	for (const bbe::DynamicArray<bbe::String>& face : rawFaces)
	{
		// A face with less than 3 (4 with the prefix) tokens doesn't make sense.
		if (face.getLength() < 4) bbe::Crash(bbe::Error::IllegalState);

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
				if (subTokens.getLength() < 3) bbe::Crash(bbe::Error::IllegalState);
				vertices.add({ 
						rawVertices[subTokens[0].toLong() - 1], 
						rawNormals [subTokens[2].toLong() - 1],
						rawUvCoords[subTokens[1].toLong() - 1]
					});
				seenFaceVertices.add(vertexInfo, (int32_t)vertices.getLength() - 1);
				indices.add((uint32_t)vertices.getLength() - 1);
			}
		}
	}

	return Model(vertices, indices);
}
