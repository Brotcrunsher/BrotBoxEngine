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
	class Model
	{
		friend class INTERNAL::openGl::OpenGLModel;
		friend class INTERNAL::openGl::OpenGLManager;
	private:
		bbe::List<bbe::PosNormalPair> m_vertices;
		bbe::List<uint32_t> m_indices;

		mutable bbe::AutoRef m_prendererData;

	public:
		Model();
		Model(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices);

		void add(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices);
		size_t getAmountOfIndices() const;
		size_t getAmountOfVertices() const;

		Model finalize() const;

		static Model fromObj(const bbe::String& obj);
	};
}