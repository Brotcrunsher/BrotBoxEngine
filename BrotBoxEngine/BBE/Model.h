#pragma once

#include <cstdint>
#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/PosNormalPair.h"
#include "../BBE/AutoRefCountable.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			class OpenGLModel;
			class OpenGLManager;
		}
	}
	class MeshBuilder;

	class Model
	{
		friend class MeshBuilder;
		friend class INTERNAL::openGl::OpenGLModel;
		friend class INTERNAL::openGl::OpenGLManager;
	private:
		bbe::List<bbe::PosNormalPair> m_vertices;
		bbe::List<uint32_t> m_indices;
		bbe::List<bbe::Vector2> m_bakingUvs;

		mutable bbe::AutoRef m_prendererData;

	public:
		Model();
		Model(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices);

		void add(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices);
		size_t getAmountOfIndices() const;
		size_t getAmountOfVertices() const;

		Model finalize(const bbe::Vector2i& textureDimensions) const;
		Model toBakingModel() const;

		static Model fromObj(const bbe::String& obj);
	};
}