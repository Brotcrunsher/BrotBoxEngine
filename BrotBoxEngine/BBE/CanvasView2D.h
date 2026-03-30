#pragma once

#include "../BBE/Math.h"
#include "../BBE/Rectangle.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	// Minimal helper for zoomable 2D canvases (screen-space ↔ canvas-space).
	struct CanvasView2D
	{
		bbe::Vector2 offset = { 0.f, 0.f }; // screen-space offset of canvas origin
		float zoom = 1.f;                  // screen pixels per canvas unit

		bbe::Vector2 screenToCanvas(const bbe::Vector2 &screenPos) const
		{
			return (screenPos - offset) / zoom;
		}

		bbe::Vector2 canvasToScreen(const bbe::Vector2 &canvasPos) const
		{
			return canvasPos * zoom + offset;
		}

		// Applies a zoom change while keeping the canvas position under the given screen anchor stable.
		void zoomAt(const bbe::Vector2 &screenAnchor, float factor)
		{
			const bbe::Vector2 before = screenToCanvas(screenAnchor);
			zoom *= factor;
			const bbe::Vector2 after = screenToCanvas(screenAnchor);
			offset += (after - before) * zoom;
		}

		// Computes a navigator widget rect anchored in the bottom-right of the window.
		// The returned rect is in screen-space.
		static bbe::Rectangle computeNavigatorRect(float canvasW, float canvasH, float windowW, float windowH, float windowScale, float baseMaxSize = 160.f, float margin = 8.f)
		{
			if (canvasW <= 0.f || canvasH <= 0.f || windowW <= 0.f || windowH <= 0.f) return {};
			const float navMaxSize = baseMaxSize * bbe::Math::sqrt(windowScale);
			float navW, navH;
			if (canvasW >= canvasH)
			{
				navW = navMaxSize;
				navH = navMaxSize * canvasH / canvasW;
			}
			else
			{
				navH = navMaxSize;
				navW = navMaxSize * canvasW / canvasH;
			}
			return bbe::Rectangle(windowW - navW - margin, windowH - navH - margin, navW, navH);
		}
	};
}

