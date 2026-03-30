#pragma once

#include "BBE/BrotBoxEngine.h"

struct PaintEditor;

/// Draws ImGui + canvas. `mouseScreenPos` is the cursor in screen pixels (same space as `editor.viewport`).
void drawExamplePaintGui(PaintEditor &editor, bbe::PrimitiveBrush2D &brush, const bbe::Vector2 &mouseScreenPos);
