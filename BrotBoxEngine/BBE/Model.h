#pragma once

#include <cstdint>
#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/PosNormalPair.h"
#include "../BBE/ManuallyRefCountable.h"

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

		mutable bbe::ManuallyRefCountable* m_prendererData = nullptr;

	public:
		Model();
		Model(const bbe::List<bbe::PosNormalPair>& vertices, const bbe::List<uint32_t>& indices);
		virtual ~Model();

		Model(const Model& other) = delete; //Copy Constructor
		Model(Model&& other) noexcept = default; //Move Constructor
		Model& operator=(const Model& other) = delete; //Copy Assignment
		Model& operator=(Model&& other) noexcept = default; //Move Assignment

		static Model fromObj(const bbe::String& obj);
	};
}