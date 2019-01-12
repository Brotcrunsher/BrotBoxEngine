#pragma once

#include "../BBE/Vector3.h"

namespace bbe
{
	namespace INTERNAL
	{
		class VertexWithNormal
		{
		public:
			Vector3 m_pos;
			Vector3 m_normal;


			VertexWithNormal(const Vector3 &pos, const Vector3 &normal);
		};
	}
}