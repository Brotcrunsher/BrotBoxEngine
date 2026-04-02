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

	/// Hit-test in continuous canvas space. \p slopCanvas is the desired half-thickness of edge/corner zones in
	/// canvas units (e.g. screenSlopPx / zoom). Slop is clamped per axis so top/bottom and left/right strips
	/// never consume the whole selection — needed for thin selections and subpixel-accurate picking when zoomed in.
	inline RectSelectionHitZone hitTestCanvas(const bbe::Rectanglei &rect, const bbe::Vector2 &point, float slopCanvas, bool allowRotationHandle = true, float rotationStemLenCanvas = 30.f, float rotationHitRadiusCanvas = 8.f)
	{
		if (rect.width <= 0 || rect.height <= 0) return RectSelectionHitZone::None;

		const float L = (float)rect.x;
		const float T = (float)rect.y;
		const float w = (float)rect.width;
		const float h = (float)rect.height;
		const float R = L + w;
		const float B = T + h;

		constexpr float eps = 1e-3f;
		const float slopX = bbe::Math::max(0.f, bbe::Math::min(slopCanvas, w * 0.5f - eps));
		const float slopY = bbe::Math::max(0.f, bbe::Math::min(slopCanvas, h * 0.5f - eps));

		const float px = point.x;
		const float py = point.y;

		const bool nearLeft = px >= L - slopX && px <= L + slopX && py >= T - slopY && py <= B + slopY;
		const bool nearRight = px >= R - slopX && px <= R + slopX && py >= T - slopY && py <= B + slopY;
		const bool nearTop = py >= T - slopY && py <= T + slopY && px >= L - slopX && px <= R + slopX;
		const bool nearBottom = py >= B - slopY && py <= B + slopY && px >= L - slopX && px <= R + slopX;

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
			const float dx = px - handlePos.x;
			const float dy = py - handlePos.y;
			if (dx * dx + dy * dy <= hitRadius * hitRadius) return RectSelectionHitZone::RotationHandle;
		}

		if (px >= L && px < R && py >= T && py < B) return RectSelectionHitZone::Inside;
		return RectSelectionHitZone::None;
	}

	inline RectSelectionHitZone hitTest(const bbe::Rectanglei &rect, const bbe::Vector2i &point, int32_t padding, bool allowRotationHandle = true, float rotationStemLenCanvas = 30.f, float rotationHitRadiusCanvas = 8.f)
	{
		return hitTestCanvas(rect, bbe::Vector2((float)point.x, (float)point.y), (float)bbe::Math::max<int32_t>(1, padding), allowRotationHandle, rotationStemLenCanvas, rotationHitRadiusCanvas);
	}
}

