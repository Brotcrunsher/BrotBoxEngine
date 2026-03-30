#pragma once

#include "BBE/BrotBoxEngine.h"

/// Viewport size and UI scale for ExamplePaint (from the host window).
struct PaintWindowMetrics
{
	int32_t width = 0;
	int32_t height = 0;
	float scale = 1.f;
};
