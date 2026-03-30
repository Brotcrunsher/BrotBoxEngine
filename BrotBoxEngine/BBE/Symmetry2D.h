#pragma once
 
#include "../BBE/List.h"
#include "../BBE/Math.h"
#include "../BBE/Vector2.h"
 
namespace bbe
{
	enum class SymmetryMode : int32_t
	{
		None = 0,
		Horizontal = 1,
		Vertical = 2,
		FourWay = 3,
		Radial = 4,
	};
 
	// Returns all positions for pos under the given symmetry mode around center.
	inline bbe::List<bbe::Vector2> getSymmetryPositions(const bbe::Vector2 &pos, const bbe::Vector2 &center, SymmetryMode mode, int32_t radialCount = 6)
	{
		bbe::List<bbe::Vector2> result;
		switch (mode)
		{
		case SymmetryMode::None:
			result.add(pos);
			break;
		case SymmetryMode::Horizontal:
			result.add(pos);
			result.add({ pos.x, 2.f * center.y - pos.y });
			break;
		case SymmetryMode::Vertical:
			result.add(pos);
			result.add({ 2.f * center.x - pos.x, pos.y });
			break;
		case SymmetryMode::FourWay:
			result.add(pos);
			result.add({ 2.f * center.x - pos.x, pos.y });
			result.add({ pos.x, 2.f * center.y - pos.y });
			result.add({ 2.f * center.x - pos.x, 2.f * center.y - pos.y });
			break;
		case SymmetryMode::Radial:
		{
			const int32_t count = bbe::Math::max(1, radialCount);
			const float step = 2.f * bbe::Math::PI / (float)count;
			for (int32_t i = 0; i < count; i++)
			{
				result.add(pos.rotate(step * (float)i, center));
			}
			break;
		}
		}
		return result;
	}
 
	// Returns the rotation offset (radians) for each entry returned by getSymmetryPositions.
	inline bbe::List<float> getSymmetryRotationAngles(SymmetryMode mode, int32_t radialCount = 6)
	{
		bbe::List<float> result;
		switch (mode)
		{
		case SymmetryMode::None:
			result.add(0.f);
			break;
		case SymmetryMode::Horizontal:
		case SymmetryMode::Vertical:
			result.add(0.f);
			result.add(0.f);
			break;
		case SymmetryMode::FourWay:
			result.add(0.f);
			result.add(0.f);
			result.add(0.f);
			result.add(0.f);
			break;
		case SymmetryMode::Radial:
		{
			const int32_t count = bbe::Math::max(1, radialCount);
			const float step = 2.f * bbe::Math::PI / (float)count;
			for (int32_t i = 0; i < count; i++)
			{
				result.add(step * (float)i);
			}
			break;
		}
		}
		return result;
	}
}

