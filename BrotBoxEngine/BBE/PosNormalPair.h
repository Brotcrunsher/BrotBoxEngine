#pragma once

#include "../BBE/Vector2.h"
#include "../BBE/Vector3.h"
#include "../BBE/Matrix4.h"

namespace bbe
{
	struct PosNormalPair
	{
		bbe::Vector3 pos;
		bbe::Vector3 normal;
		bbe::Vector2 uvCoord;

		static void transform(bbe::List<PosNormalPair>& vertices, const bbe::Matrix4& matrix);
	};
	static_assert(alignof(PosNormalPair) == alignof(float));
	static_assert(sizeof(PosNormalPair) == (8 * sizeof(float)));
}