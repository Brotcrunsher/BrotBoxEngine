#pragma once

#include "../../BBE/Math.h"
#include "../../BBE/Rectangle.h"
#include "../../BBE/Vector2.h"

namespace bbe::editor
{
	enum class RectSelectionHitZone : int32_t
	{
		None = 0,
		Inside,
		Left,
		Right,
		Top,
		Bottom,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
		RotationHandle,
	};

	inline bool isResizeZone(RectSelectionHitZone z)
	{
		return z != RectSelectionHitZone::None && z != RectSelectionHitZone::Inside && z != RectSelectionHitZone::RotationHandle;
	}

	inline bbe::Vector2 rotationHandleCanvasPos(const bbe::Rectanglei &rect, float stemLenCanvas = 30.f)
	{
		return {
			rect.x + rect.width / 2.f,
			rect.y - stemLenCanvas
		};
	}

	inline RectSelectionHitZone hitTest(const bbe::Rectanglei &rect, const bbe::Vector2i &point, int32_t padding, bool allowRotationHandle = true, float rotationStemLenCanvas = 30.f, float rotationHitRadiusCanvas = 8.f)
	{
		if (rect.width <= 0 || rect.height <= 0) return RectSelectionHitZone::None;

		const int32_t left = rect.x;
		const int32_t top = rect.y;
		const int32_t right = rect.x + rect.width - 1;
		const int32_t bottom = rect.y + rect.height - 1;
		padding = bbe::Math::max<int32_t>(1, padding);

		const bool nearLeft = point.x >= left - padding && point.x <= left + padding && point.y >= top - padding && point.y <= bottom + padding;
		const bool nearRight = point.x >= right - padding && point.x <= right + padding && point.y >= top - padding && point.y <= bottom + padding;
		const bool nearTop = point.y >= top - padding && point.y <= top + padding && point.x >= left - padding && point.x <= right + padding;
		const bool nearBottom = point.y >= bottom - padding && point.y <= bottom + padding && point.x >= left - padding && point.x <= right + padding;

		if (nearLeft && nearTop) return RectSelectionHitZone::TopLeft;
		if (nearRight && nearTop) return RectSelectionHitZone::TopRight;
		if (nearLeft && nearBottom) return RectSelectionHitZone::BottomLeft;
		if (nearRight && nearBottom) return RectSelectionHitZone::BottomRight;
		if (nearLeft) return RectSelectionHitZone::Left;
		if (nearRight) return RectSelectionHitZone::Right;
		if (nearTop) return RectSelectionHitZone::Top;
		if (nearBottom) return RectSelectionHitZone::Bottom;

		if (allowRotationHandle)
		{
			const bbe::Vector2 handlePos = rotationHandleCanvasPos(rect, rotationStemLenCanvas);
			const float hitRadius = rotationHitRadiusCanvas;
			const float dx = point.x - handlePos.x;
			const float dy = point.y - handlePos.y;
			if (dx * dx + dy * dy <= hitRadius * hitRadius) return RectSelectionHitZone::RotationHandle;
		}

		if (rect.isPointInRectangle(point, true)) return RectSelectionHitZone::Inside;
		return RectSelectionHitZone::None;
	}
}

