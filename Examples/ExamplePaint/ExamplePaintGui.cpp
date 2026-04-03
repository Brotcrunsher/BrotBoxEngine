#include "ExamplePaintEditor.h"
#include "ExamplePaintGui.h"
#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <string>

#ifdef BBE_RENDERER_OPENGL
#include "BBE/glfwWrapper.h"

static void updateIconSlot(ImTextureID &texId, const bbe::Image *&cachedPtr, const bbe::Image *current)
{
	if (current == cachedPtr && texId) return;
	if (texId)
	{
		GLuint old = (GLuint)(intptr_t)texId;
		glDeleteTextures(1, &old);
		texId = nullptr;
	}
	cachedPtr = current;
	if (!current || current->getWidth() <= 0 || current->getHeight() <= 0) return;
	const int w = current->getWidth();
	const int h = current->getHeight();
	std::vector<unsigned char> pixels(w * h * 4);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			const bbe::Colori c = current->getPixel(x, y);
			const size_t idx = (y * w + x) * 4;
			pixels[idx + 0] = c.r;
			pixels[idx + 1] = c.g;
			pixels[idx + 2] = c.b;
			pixels[idx + 3] = c.a;
		}
	}
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	texId = (ImTextureID)(intptr_t)tex;
}

struct ToolIconTextures
{
	struct Slot { ImTextureID texId = nullptr; const bbe::Image *cachedPtr = nullptr; };
	Slot brush, eraser, spray, fill, line, rectangle, circle, selection, ellipseSelection, lasso, polygonLasso, magicWand, text, pipette, arrow, bezier;
	Slot undo, redo;
	Slot layerNew, layerDelete, layerUp, layerDown, layerDuplicate, layerMergeDown;
	Slot symmetryOff, symmetryHorizontal, symmetryVertical, symmetryFourWay, symmetryRadial;
	Slot clipboardCopyCanvas, clipboardPasteNew;

	void refresh()
	{
		updateIconSlot(brush.texId,          brush.cachedPtr,          assetStore::iconBrush());
		updateIconSlot(eraser.texId,         eraser.cachedPtr,         assetStore::iconEraser());
		updateIconSlot(spray.texId,          spray.cachedPtr,          assetStore::iconSpray());
		updateIconSlot(fill.texId,           fill.cachedPtr,           assetStore::iconFill());
		updateIconSlot(line.texId,           line.cachedPtr,           assetStore::iconLine());
		updateIconSlot(rectangle.texId,      rectangle.cachedPtr,      assetStore::iconRectangle());
		updateIconSlot(circle.texId,         circle.cachedPtr,         assetStore::iconCircle());
		updateIconSlot(selection.texId,      selection.cachedPtr,      assetStore::iconSelection());
		updateIconSlot(ellipseSelection.texId, ellipseSelection.cachedPtr, assetStore::iconEllipseSelection());
		updateIconSlot(lasso.texId,          lasso.cachedPtr,          assetStore::iconLasso());
		updateIconSlot(polygonLasso.texId,   polygonLasso.cachedPtr,   assetStore::iconPolygonLasso());
		updateIconSlot(magicWand.texId,      magicWand.cachedPtr,      assetStore::iconMagicWand());
		updateIconSlot(text.texId,           text.cachedPtr,           assetStore::iconText());
		updateIconSlot(pipette.texId,        pipette.cachedPtr,        assetStore::iconPipette());
		updateIconSlot(arrow.texId,          arrow.cachedPtr,          assetStore::iconArrow());
		updateIconSlot(bezier.texId,         bezier.cachedPtr,         assetStore::iconBezier());
		updateIconSlot(undo.texId,           undo.cachedPtr,           assetStore::iconUndo());
		updateIconSlot(redo.texId,           redo.cachedPtr,           assetStore::iconRedo());
		updateIconSlot(layerNew.texId,       layerNew.cachedPtr,       assetStore::iconLayerNew());
		updateIconSlot(layerDelete.texId,    layerDelete.cachedPtr,    assetStore::iconLayerDelete());
		updateIconSlot(layerUp.texId,        layerUp.cachedPtr,        assetStore::iconLayerUp());
		updateIconSlot(layerDown.texId,      layerDown.cachedPtr,      assetStore::iconLayerDown());
		updateIconSlot(layerDuplicate.texId, layerDuplicate.cachedPtr, assetStore::iconLayerDuplicate());
		updateIconSlot(layerMergeDown.texId, layerMergeDown.cachedPtr, assetStore::iconLayerMergeDown());
		updateIconSlot(symmetryOff.texId, symmetryOff.cachedPtr, assetStore::iconSymmetryOff());
		updateIconSlot(symmetryHorizontal.texId, symmetryHorizontal.cachedPtr, assetStore::iconSymmetryHorizontal());
		updateIconSlot(symmetryVertical.texId, symmetryVertical.cachedPtr, assetStore::iconSymmetryVertical());
		updateIconSlot(symmetryFourWay.texId, symmetryFourWay.cachedPtr, assetStore::iconSymmetryFourWay());
		updateIconSlot(symmetryRadial.texId, symmetryRadial.cachedPtr, assetStore::iconSymmetryRadial());
		updateIconSlot(clipboardCopyCanvas.texId, clipboardCopyCanvas.cachedPtr, assetStore::iconClipboardCopyCanvas());
		updateIconSlot(clipboardPasteNew.texId, clipboardPasteNew.cachedPtr, assetStore::iconClipboardPasteNew());
	}
};
static ToolIconTextures s_toolIcons;
#endif

void drawSelectionOutlineForGui(bbe::PrimitiveBrush2D &brush, const PaintEditor &editor, const bbe::Rectanglei &rect, bool alwaysDrawOutline)
{
	const bool showSelectionChrome =
		editor.mode == PaintEditor::MODE_SELECTION
		|| editor.mode == PaintEditor::MODE_ELLIPSE_SELECTION
		|| editor.mode == PaintEditor::MODE_MAGIC_WAND
		|| editor.mode == PaintEditor::MODE_LASSO
		|| editor.mode == PaintEditor::MODE_POLYGON_LASSO
		|| (editor.mode == PaintEditor::MODE_RECTANGLE && (editor.rectangle.draftActive || editor.rectangle.dragActive))
		|| (editor.mode == PaintEditor::MODE_CIRCLE && (editor.circle.draftActive || editor.circle.dragActive));

	if (!alwaysDrawOutline && !showSelectionChrome)
		return;

	const bbe::Rectangle screenRect = editor.selectionRectToScreen(rect);
	brush.setColorRGB(0.0f, 0.0f, 0.0f);
	brush.sketchRect(screenRect);
	if (screenRect.width > 2 && screenRect.height > 2)
	{
		brush.setColorRGB(1.0f, 1.0f, 1.0f);
		brush.sketchRect(screenRect.shrinked(1.0f));
	}

	if (showSelectionChrome && editor.selection.hasSelection && !editor.selection.dragActive && !editor.selection.lassoDragActive && editor.selection.polygonLassoVertices.empty())
	{
		const float sx = screenRect.x;
		const float sy = screenRect.y;
		const float w = screenRect.width;
		const float h = screenRect.height;
		// Same ordering as canvas resize handles: corners and edge midpoints.
		const bbe::Vector2 resizeHandles[8] = {
			{ sx, sy },
			{ sx + w * 0.5f, sy },
			{ sx + w, sy },
			{ sx + w, sy + h * 0.5f },
			{ sx + w, sy + h },
			{ sx + w * 0.5f, sy + h },
			{ sx, sy + h },
			{ sx, sy + h * 0.5f },
		};
		constexpr float hs = 5.f;
		for (int32_t i = 0; i < 8; i++)
		{
			const float hx = resizeHandles[i].x;
			const float hy = resizeHandles[i].y;
			brush.setColorRGB(1.f, 1.f, 1.f);
			brush.fillRect(hx - hs, hy - hs, hs * 2.f, hs * 2.f);
			brush.setColorRGB(0.f, 0.f, 0.f);
			brush.sketchRect(bbe::Rectangle(hx - hs, hy - hs, hs * 2.f, hs * 2.f));
		}

		const float cx = sx + w * 0.5f;
		const float ty = sy;
		constexpr float stemLen = 30.f;
		const float handleY = ty - stemLen;
		constexpr float handleR = 6.f;

		brush.setColorRGB(0.f, 0.f, 0.f);
		brush.fillLine(cx + 1.f, ty, cx + 1.f, handleY, 1.f);
		brush.setColorRGB(1.f, 1.f, 1.f);
		brush.fillLine(cx, ty, cx, handleY, 1.f);

		brush.setColorRGB(0.f, 0.f, 0.f);
		brush.fillCircle(cx - handleR - 1.f, handleY - handleR - 1.f, (handleR + 1.f) * 2.f, (handleR + 1.f) * 2.f);
		brush.setColorRGB(1.f, 1.f, 1.f);
		brush.fillCircle(cx - handleR, handleY - handleR, handleR * 2.f, handleR * 2.f);
	}
}

void drawSelectionPixelMaskOverlayForGui(bbe::PrimitiveBrush2D &brush, const PaintEditor &editor, const bbe::Rectanglei &canvasRect)
{
	if (!editor.selection.hasSelection || !editor.hasSelectionPixelMask()) return;
	const bbe::Image &m = editor.selection.mask;
	if (canvasRect.width <= 0 || canvasRect.height <= 0 || m.getWidth() != (size_t)canvasRect.width || m.getHeight() != (size_t)canvasRect.height) return;

	// Screen-space hairline: stays ~1 device pixel so zoomed-in canvas pixels show a thin contour, not a thick band.
	const float hair = std::max(1.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f);

	auto isOn = [&](int32_t lx, int32_t ly) -> bool
	{
		if (lx < 0 || ly < 0 || lx >= canvasRect.width || ly >= canvasRect.height) return false;
		return m.getPixel((size_t)lx, (size_t)ly).a >= 128;
	};

	// Boundary edges only: side of a selected pixel that borders a non-selected neighbor (or mask bounds).
	for (int32_t ly = 0; ly < canvasRect.height; ly++)
	{
		for (int32_t lx = 0; lx < canvasRect.width; lx++)
		{
			if (!isOn(lx, ly)) continue;
			const int32_t cx = canvasRect.x + lx;
			const int32_t cy = canvasRect.y + ly;
			const bbe::Rectangle scr = editor.selectionRectToScreen(bbe::Rectanglei(cx, cy, 1, 1));

			if (ly == 0 || !isOn(lx, ly - 1))
			{
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.fillRect(scr.x, scr.y, scr.width, hair);
				if (scr.height >= hair * 2.5f)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.fillRect(scr.x, scr.y + hair, scr.width, hair);
				}
			}
			if (ly == canvasRect.height - 1 || !isOn(lx, ly + 1))
			{
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.fillRect(scr.x, scr.y + scr.height - hair, scr.width, hair);
				if (scr.height >= hair * 2.5f)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.fillRect(scr.x, scr.y + scr.height - hair * 2.f, scr.width, hair);
				}
			}
			if (lx == 0 || !isOn(lx - 1, ly))
			{
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.fillRect(scr.x, scr.y, hair, scr.height);
				if (scr.width >= hair * 2.5f)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.fillRect(scr.x + hair, scr.y, hair, scr.height);
				}
			}
			if (lx == canvasRect.width - 1 || !isOn(lx + 1, ly))
			{
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.fillRect(scr.x + scr.width - hair, scr.y, hair, scr.height);
				if (scr.width >= hair * 2.5f)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.fillRect(scr.x + scr.width - hair * 2.f, scr.y, hair, scr.height);
				}
			}
		}
	}
}

void drawTextPreviewForGui(bbe::PrimitiveBrush2D &brush, PaintEditor &editor, const bbe::Vector2i &topLeft)
{
	bbe::Vector2 origin;
	bbe::Rectangle bounds;
	if (!editor.getTextOriginAndBounds(topLeft, origin, bounds)) return;

	const bbe::String text = editor.getTextBufferString();
	const bbe::Font &font = editor.getTextToolFont();
	const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(origin, text);
	const bbe::Color previewColor = bbe::Color(editor.leftColor).blendTo(bbe::Color::white(), 0.15f);

	const int32_t tileDraw = editor.tiled ? 20 : 0;
	for (int32_t ti = -tileDraw; ti <= tileDraw; ti++)
	{
		for (int32_t tk = -tileDraw; tk <= tileDraw; tk++)
		{
			const float tileOffX = ti * editor.getCanvasWidth() * editor.zoomLevel;
			const float tileOffY = tk * editor.getCanvasHeight() * editor.zoomLevel;

			brush.setColorRGB(previewColor);
			auto it = text.getIterator();
			for (size_t i = 0; i < renderPositions.getLength() && it.valid(); i++, ++it)
			{
				const int32_t codePoint = it.getCodepoint();
				if (codePoint == ' ' || codePoint == '\n' || codePoint == '\r' || codePoint == '\t') continue;

				const bbe::Image &glyph = editor.getTextGlyphImage(font, codePoint);
				brush.drawImage(
					editor.offset.x + tileOffX + renderPositions[i].x * editor.zoomLevel,
					editor.offset.y + tileOffY + renderPositions[i].y * editor.zoomLevel,
					glyph.getWidth() * editor.zoomLevel,
					glyph.getHeight() * editor.zoomLevel,
					glyph);
			}

			drawSelectionOutlineForGui(brush, editor, bbe::Rectanglei(
				topLeft.x + ti * editor.getCanvasWidth(),
				topLeft.y + tk * editor.getCanvasHeight(),
				(int32_t)bbe::Math::ceil(bounds.width),
				(int32_t)bbe::Math::ceil(bounds.height)),
				true);
		}
	}
}

static void drawPaintToolOptionsPanel(PaintEditor &editor, float toolbarWidth)
{
	if (!editor.showToolOptionsPanel) return;

	const float scale = editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f;
	const float optW = 292.f * scale;
	const float menuH = ImGui::GetFrameHeight();
	const float vh = (float)editor.viewport.height - menuH;

	ImGui::SetNextWindowPos(ImVec2(toolbarWidth, menuH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(optW, vh), ImGuiCond_FirstUseEver);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.15f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
	if (ImGui::Begin("Tool options", &editor.showToolOptionsPanel,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		ImGui::PushID("paintToolOpts");

		bool optionsHeaderShown = false;
		auto ensureOptionsHeader = [&]()
		{
			if (!optionsHeaderShown)
			{
				ImGui::SeparatorText("Options");
				optionsHeaderShown = true;
			}
		};
		const float stepBtnW = ImGui::GetFrameHeight();
		auto widthWithSteppers = [&](const char *label, int *value, void (PaintEditor::*clampFn)(), void (PaintEditor::*refreshFn)())
		{
			ensureOptionsHeader();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label);
			ImGui::SameLine();
			if (ImGui::Button("-##wdec", ImVec2(stepBtnW, stepBtnW)))
			{
				--*value;
				(editor.*clampFn)();
				if (refreshFn) (editor.*refreshFn)();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(std::max(1.f, ImGui::GetContentRegionAvail().x - stepBtnW * 2.f - ImGui::GetStyle().ItemSpacing.x * 2.f));
			if (ImGui::InputInt("##wval", value, 0, 0))
			{
				(editor.*clampFn)();
				if (refreshFn) (editor.*refreshFn)();
			}
			ImGui::SameLine();
			if (ImGui::Button("+##winc", ImVec2(stepBtnW, stepBtnW)))
			{
				++*value;
				(editor.*clampFn)();
				if (refreshFn) (editor.*refreshFn)();
			}
		};

		if (editor.mode == PaintEditor::MODE_ERASER)
		{
			widthWithSteppers("Size", &editor.eraserSize, &PaintEditor::clampEraserSize, nullptr);
			ImGui::TextDisabled("Square stamp; alpha→0. [+ / -] also adjusts.");
		}
		if (editor.mode == PaintEditor::MODE_BRUSH || editor.mode == PaintEditor::MODE_LINE || editor.mode == PaintEditor::MODE_RECTANGLE || editor.mode == PaintEditor::MODE_CIRCLE || editor.mode == PaintEditor::MODE_ARROW || editor.mode == PaintEditor::MODE_BEZIER)
		{
			widthWithSteppers("Width", &editor.brushWidth, &PaintEditor::clampBrushWidth, &PaintEditor::refreshBrushBasedDrafts);
		}
		if (editor.mode == PaintEditor::MODE_SPRAY)
		{
			widthWithSteppers("Spray width", &editor.sprayWidth, &PaintEditor::clampSprayWidth, nullptr);
			if (ImGui::InputInt("Droplets", &editor.sprayDensity))
			{
				editor.clampSprayDensity();
			}
			ImGui::TextDisabled("Dots per burst; denser near the center of the spray disk. [+ / -] adjusts spray width.");
		}
		if (editor.mode == PaintEditor::MODE_MAGIC_WAND)
		{
			ensureOptionsHeader();
			if (ImGui::SliderInt("Tolerance", &editor.magicWandTolerance, 0, 255))
			{
				editor.clampMagicWandTolerance();
			}
			ImGui::TextDisabled("Click visible pixels to select. [+ / -] nudge tolerance. Ctrl adds to selection.");
		}
		if (editor.mode == PaintEditor::MODE_FLOOD_FILL)
		{
			ensureOptionsHeader();
			if (ImGui::SliderInt("Tolerance", &editor.floodFillTolerance, 0, 255))
			{
				editor.clampFloodFillTolerance();
			}
			ImGui::TextDisabled("Fills the active layer from the click; per-channel match vs seed color. [+ / -] nudges tolerance.");
		}
		if (editor.mode == PaintEditor::MODE_LASSO)
		{
			ensureOptionsHeader();
			ImGui::TextDisabled("Click and drag to outline a region (auto-closed). Ctrl adds to the current selection. Move/resize like rectangular selection.");
		}
		if (editor.mode == PaintEditor::MODE_POLYGON_LASSO)
		{
			ensureOptionsHeader();
			ImGui::TextDisabled("Click to place corners. Close: click first point, Enter, or right-click (3+ points). Backspace removes last point. Ctrl adds to selection.");
		}
		if (editor.mode == PaintEditor::MODE_RECTANGLE)
		{
			ensureOptionsHeader();
			if (ImGui::InputInt("Corner Radius", &editor.cornerRadius))
			{
				if (editor.cornerRadius < 0) editor.cornerRadius = 0;
				if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
			}
			ImGui::TextDisabled(editor.rectangle.draftActive
				? "Drag inside/border to move/resize.\nClick outside to place."
				: "Drag to draw. Click outside to place.");
		}
		if (editor.mode == PaintEditor::MODE_CIRCLE)
		{
			ensureOptionsHeader();
			ImGui::TextDisabled(editor.circle.draftActive
				? "Drag inside/border to move/resize.\nClick outside to place."
				: "Drag to draw. Click outside to place.");
		}
		if (editor.mode == PaintEditor::MODE_RECTANGLE || editor.mode == PaintEditor::MODE_CIRCLE)
		{
			ensureOptionsHeader();
			if (ImGui::Checkbox("Fill with secondary color", &editor.shapeFillWithSecondary))
			{
				if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
				if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
			}
			if (ImGui::Checkbox("Striped outline", &editor.shapeStripedStroke))
			{
				if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
				if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
			}
			if (editor.shapeStripedStroke)
			{
				if (ImGui::InputInt("Stripe repeat (px)", &editor.shapeStripePeriodPx))
				{
					editor.clampShapeStripePeriod();
					if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
					if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
				}
				ImGui::TextDisabled("Target dash+gap along the outline; actual spacing snaps so the pattern closes with no seam.");
			}
		}
		if (editor.mode == PaintEditor::MODE_LINE)
		{
			ensureOptionsHeader();
			ImGui::Checkbox("Apply on release", &editor.endpointApplyStrokeOnRelease);
			ImGui::TextDisabled(editor.endpointApplyStrokeOnRelease
				? (editor.line.draftActive
					   ? "Drag endpoints to adjust.\nClick outside or R-click to place."
					   : "Drag to draw; the stroke is committed when you release the button.")
				: (editor.line.draftActive
					   ? "Drag endpoints to adjust.\nClick outside or R-click to place."
					   : "Drag to draw."));
		}
		if (editor.mode == PaintEditor::MODE_ARROW)
		{
			ensureOptionsHeader();
			ImGui::Checkbox("Apply on release", &editor.endpointApplyStrokeOnRelease);
			bool arrowOptionChanged = false;
			if (ImGui::InputInt("Head Size", &editor.arrowHeadSize))
			{
				if (editor.arrowHeadSize < 1) editor.arrowHeadSize = 1;
				arrowOptionChanged = true;
			}
			if (ImGui::InputInt("Head Width", &editor.arrowHeadWidth))
			{
				if (editor.arrowHeadWidth < 1) editor.arrowHeadWidth = 1;
				arrowOptionChanged = true;
			}
			if (ImGui::Checkbox("Double Headed", &editor.arrowDoubleHeaded)) arrowOptionChanged = true;
			if (ImGui::Checkbox("Filled Head",   &editor.arrowFilledHead))   arrowOptionChanged = true;
			if (arrowOptionChanged && editor.arrow.draftActive) editor.redrawArrowDraft();
			ImGui::TextDisabled(editor.endpointApplyStrokeOnRelease
				? (editor.arrow.draftActive
					   ? "Drag endpoints to adjust.\nClick outside or R-click to place."
					   : "Drag to draw; the stroke is committed when you release the button.")
				: (editor.arrow.draftActive
					   ? "Drag endpoints to adjust.\nClick outside or R-click to place."
					   : "Drag to draw."));
		}
		if (editor.mode == PaintEditor::MODE_BEZIER)
		{
			ensureOptionsHeader();
			ImGui::TextDisabled(editor.bezier.controlPoints.isEmpty()
				? "L-click to place control points.\nR-click to commit curve."
				: "L-click to add/drag points.\nBackspace removes last point.\nR-click to commit curve.");
			if (!editor.bezier.controlPoints.isEmpty())
			{
				ImGui::Text("%d control point(s)", (int)editor.bezier.controlPoints.getLength());
				if (ImGui::Button("Commit", ImVec2(-1, 0)))
				{
					editor.finalizeBezierDraft();
				}
			}
		}
		if (editor.mode == PaintEditor::MODE_TEXT)
		{
			ensureOptionsHeader();
			static char fontFilter[128] = "";
			ImGui::InputText("Filter##fontFilter", fontFilter, sizeof(fontFilter));
			editor.clampTextFontIndex();
			const char *currentFontName = editor.availableFonts.isEmpty() ? "None" : editor.availableFonts[(size_t)editor.textFontIndex].displayName.getRaw();
			if (ImGui::BeginCombo("Font", currentFontName))
			{
				for (size_t i = 0; i < editor.availableFonts.getLength(); i++)
				{
					const char *name = editor.availableFonts[i].displayName.getRaw();
					if (fontFilter[0] != '\0')
					{
						std::string nameLower = name;
						std::string filterLower = fontFilter;
						for (char &c : nameLower)   c = (char)std::tolower((unsigned char)c);
						for (char &c : filterLower) c = (char)std::tolower((unsigned char)c);
						if (nameLower.find(filterLower) == std::string::npos) continue;
					}
					const bool selected = (editor.textFontIndex == (int32_t)i);
					if (ImGui::Selectable(name, selected))
					{
						editor.textFontIndex = (int32_t)i;
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (ImGui::InputInt("Font Size", &editor.textFontSize))
			{
				editor.clampTextFontSize();
			}
			ImGui::InputTextMultiline("##text", editor.textBuffer, sizeof(editor.textBuffer), ImVec2(-1, ImGui::GetTextLineHeight() * 4.0f));
			ImGui::TextDisabled("L/R click places text.");
		}

		if (!optionsHeaderShown)
			ImGui::TextDisabled("No adjustable options for this tool.");

		ImGui::PopID();
	}
	ImGui::End();
	ImGui::PopStyleColor(2);
}

void drawExamplePaintGui(PaintEditor &editor, bbe::PrimitiveBrush2D &brush, const bbe::Vector2 &mouseScreenPos)
{
#ifdef BBE_RENDERER_OPENGL
		s_toolIcons.refresh();
#endif
		const float PANEL_WIDTH = 236.f * editor.viewport.scale;
		ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, (float)editor.viewport.height - ImGui::GetFrameHeight()), ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
		ImGui::Begin("##panel", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove);

		const float availY = ImGui::GetContentRegionAvail().y;
		const float layerDockMin = 200.f * editor.viewport.scale;
		const float layerDockMax = 340.f * editor.viewport.scale;
		const float layerDockH = std::clamp(availY * 0.42f, layerDockMin, layerDockMax);
		const float toolScrollH = std::max(80.f, availY - layerDockH - ImGui::GetStyle().ItemSpacing.y);

		ImGui::BeginChild("##toolScroll", ImVec2(-1, toolScrollH), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		auto doUndo = [&]() { editor.undo(); };
		auto doRedo = [&]() { editor.redo(); };

		// --- Undo / Redo ---
		const float halfW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		constexpr float undoRedoIconSize = 24.f;
		ImGui::BeginDisabled(!editor.canvas.isUndoable());
#ifdef BBE_RENDERER_OPENGL
		if (s_toolIcons.undo.texId)
		{
			if (ImGui::ImageButton("Undo", s_toolIcons.undo.texId, ImVec2(undoRedoIconSize, undoRedoIconSize))) doUndo();
		}
		else
#endif
		{
			if (ImGui::Button("Undo", ImVec2(halfW, 0))) doUndo();
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Undo");
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!editor.canvas.isRedoable());
#ifdef BBE_RENDERER_OPENGL
		if (s_toolIcons.redo.texId)
		{
			if (ImGui::ImageButton("Redo", s_toolIcons.redo.texId, ImVec2(undoRedoIconSize, undoRedoIconSize))) doRedo();
		}
		else
#endif
		{
			if (ImGui::Button("Redo", ImVec2(halfW, 0))) doRedo();
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Redo");
		ImGui::EndDisabled();

		// --- Colors (swatches + pipette on one row) ---
		ImGui::SeparatorText("Colors");
		const ImGuiColorEditFlags colorPickFlags =
			ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
			| ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar;
		const float colorRowPadY = 5.f * editor.viewport.scale;
		const float availColorW = ImGui::GetContentRegionAvail().x;
		const float colorGap = ImGui::GetStyle().ItemSpacing.x;
		const float pipetteBtn = std::max(1.f, std::floor(26.f * editor.viewport.scale));
		const float swatchPairW = std::max(1.f, availColorW - pipetteBtn - colorGap);
		const float swatchColW = std::max(1.f, (swatchPairW - colorGap) * 0.5f);
		ImGui::BeginGroup();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, colorRowPadY));
		ImGui::SetNextItemWidth(swatchColW);
		const bool leftColorChanged = ImGui::ColorEdit4("##primaryColor", editor.leftColor, colorPickFlags);
		ImGui::SameLine(0.f, colorGap);
		ImGui::SetNextItemWidth(swatchColW);
		const bool rightColorChanged = ImGui::ColorEdit4("##secondaryColor", editor.rightColor, colorPickFlags);
		ImGui::PopStyleVar();
		ImGui::SameLine(0.f, colorGap);
		{
			const bool pipActive = editor.mode == PaintEditor::MODE_PIPETTE;
			if (pipActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.pipette.texId)
			{
				if (ImGui::ImageButton("Pipette##colorsPipette", s_toolIcons.pipette.texId, ImVec2(pipetteBtn, pipetteBtn)))
					editor.mode = PaintEditor::MODE_PIPETTE;
			}
			else
#endif
			{
				if (ImGui::Button("Pick##colorsPipette", ImVec2(pipetteBtn, pipetteBtn)))
					editor.mode = PaintEditor::MODE_PIPETTE;
			}
			if (pipActive) ImGui::PopStyleColor();
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pipette — click canvas to sample a color");
		}
		ImGui::EndGroup();
		if (editor.rectangle.draftActive && (editor.shapeFillWithSecondary || editor.shapeStripedStroke
			? (leftColorChanged || rightColorChanged)
			: ((leftColorChanged && !editor.rectangle.draftUsesRightColor) || (rightColorChanged && editor.rectangle.draftUsesRightColor))))
		{
			editor.refreshActiveRectangleDraftImage();
		}
		if (editor.circle.draftActive && (editor.shapeFillWithSecondary || editor.shapeStripedStroke
			? (leftColorChanged || rightColorChanged)
			: ((leftColorChanged && !editor.circle.draftUsesRightColor) || (rightColorChanged && editor.circle.draftUsesRightColor))))
		{
			editor.refreshActiveCircleDraftImage();
		}
		if (editor.line.draftActive && ((leftColorChanged && !editor.line.draftUsesRightColor) || (rightColorChanged && editor.line.draftUsesRightColor))) editor.redrawLineDraft();
		if (editor.arrow.draftActive && ((leftColorChanged && !editor.arrow.draftUsesRightColor) || (rightColorChanged && editor.arrow.draftUsesRightColor))) editor.redrawArrowDraft();
		if (!editor.bezier.controlPoints.isEmpty() && ((leftColorChanged && !editor.bezier.usesRightColor) || (rightColorChanged && editor.bezier.usesRightColor))) editor.redrawBezierDraft();

		// --- Tools: drawing / paint vs selection (matches isSelectionLikeTool split) ---
		{
			constexpr int kToolCols = 5;
			const float rowW = ImGui::GetContentRegionAvail().x;
			const float sp = ImGui::GetStyle().ItemSpacing.x;
			const float cell = std::max(1.f, (rowW - sp * (float)(kToolCols - 1)) / (float)kToolCols);
			const float w = cell;
			struct ToolBtn
			{
				const char *label;
				int32_t toolMode;
				ImTextureID icon;
			};
#ifdef BBE_RENDERER_OPENGL
			const ToolBtn paintTools[] = {
				{ "Brush",     PaintEditor::MODE_BRUSH,             s_toolIcons.brush.texId },
				{ "Eraser",    PaintEditor::MODE_ERASER,          s_toolIcons.eraser.texId },
				{ "Rectangle", PaintEditor::MODE_RECTANGLE,       s_toolIcons.rectangle.texId },
				{ "Fill",      PaintEditor::MODE_FLOOD_FILL,      s_toolIcons.fill.texId },
				{ "Circle",    PaintEditor::MODE_CIRCLE,          s_toolIcons.circle.texId },
				{ "Text",      PaintEditor::MODE_TEXT,            s_toolIcons.text.texId },
				{ "Line",      PaintEditor::MODE_LINE,            s_toolIcons.line.texId },
				{ "Arrow",     PaintEditor::MODE_ARROW,           s_toolIcons.arrow.texId },
				{ "Spray",     PaintEditor::MODE_SPRAY,           s_toolIcons.spray.texId },
				{ "Bezier",    PaintEditor::MODE_BEZIER,          s_toolIcons.bezier.texId },
			};
			const ToolBtn selectionTools[] = {
				{ "Selection",   PaintEditor::MODE_SELECTION,         s_toolIcons.selection.texId },
				{ "Ellipse Sel", PaintEditor::MODE_ELLIPSE_SELECTION, s_toolIcons.ellipseSelection.texId },
				{ "Lasso",       PaintEditor::MODE_LASSO,             s_toolIcons.lasso.texId },
				{ "Wand",        PaintEditor::MODE_MAGIC_WAND,        s_toolIcons.magicWand.texId },
				{ "Poly Lasso",  PaintEditor::MODE_POLYGON_LASSO,     s_toolIcons.polygonLasso.texId },
			};
#else
			const ToolBtn paintTools[] = {
				{ "Brush",     PaintEditor::MODE_BRUSH,             nullptr },
				{ "Eraser",    PaintEditor::MODE_ERASER,          nullptr },
				{ "Rectangle", PaintEditor::MODE_RECTANGLE,       nullptr },
				{ "Fill",      PaintEditor::MODE_FLOOD_FILL,      nullptr },
				{ "Circle",    PaintEditor::MODE_CIRCLE,          nullptr },
				{ "Text",      PaintEditor::MODE_TEXT,            nullptr },
				{ "Line",      PaintEditor::MODE_LINE,            nullptr },
				{ "Arrow",     PaintEditor::MODE_ARROW,           nullptr },
				{ "Spray",     PaintEditor::MODE_SPRAY,           nullptr },
				{ "Bezier",    PaintEditor::MODE_BEZIER,          nullptr },
			};
			const ToolBtn selectionTools[] = {
				{ "Selection",   PaintEditor::MODE_SELECTION,         nullptr },
				{ "Ellipse Sel", PaintEditor::MODE_ELLIPSE_SELECTION, nullptr },
				{ "Lasso",       PaintEditor::MODE_LASSO,             nullptr },
				{ "Wand",        PaintEditor::MODE_MAGIC_WAND,        nullptr },
				{ "Poly Lasso",  PaintEditor::MODE_POLYGON_LASSO,     nullptr },
			};
#endif
			const float iconSize = std::max(1.f, std::min(24.f * editor.viewport.scale, std::floor(cell)));
			auto drawToolGrid = [&](const ToolBtn *tools, size_t count)
			{
				for (size_t i = 0; i < count; i++)
				{
					const ToolBtn &tb = tools[i];
					const bool active = editor.mode == tb.toolMode;
					if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
					bool clicked = false;
					if (tb.icon)
						clicked = ImGui::ImageButton(tb.label, tb.icon, ImVec2(iconSize, iconSize));
					else
						clicked = ImGui::Button(tb.label, ImVec2(w, 0));
					if (clicked) editor.mode = tb.toolMode;
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tb.label);
					if (active) ImGui::PopStyleColor();
					if (i % kToolCols != (size_t)kToolCols - 1 && i + 1 < count) ImGui::SameLine();
				}
			};

			ImGui::SeparatorText("Draw & paint");
			drawToolGrid(paintTools, sizeof(paintTools) / sizeof(*paintTools));
			ImGui::SeparatorText("Selection");
			drawToolGrid(selectionTools, sizeof(selectionTools) / sizeof(*selectionTools));
		}

		// --- Symmetry ---
		ImGui::SeparatorText("Symmetry");
		{
			const float symIconSize = 24.f * editor.viewport.scale;
			const float symRowW = ImGui::GetContentRegionAvail().x;
			const float symBtnW = (symRowW - ImGui::GetStyle().ItemSpacing.x * 4.f) / 5.f;
			const struct
			{
				const char *id;
				const char *fallback;
				bbe::SymmetryMode mode;
				const char *tip;
			} symModes[] = {
				{ "##symOff", "Off", bbe::SymmetryMode::None, "No mirror symmetry" },
				{ "##symH", "H", bbe::SymmetryMode::Horizontal, "Mirror horizontally (reflect across the vertical center)" },
				{ "##symV", "V", bbe::SymmetryMode::Vertical, "Mirror vertically (reflect across the horizontal center)" },
				{ "##sym4", "4W", bbe::SymmetryMode::FourWay, "Mirror on both axes (four-way symmetry)" },
				{ "##symRad", "Rad", bbe::SymmetryMode::Radial, "Radial symmetry around canvas center" },
			};
			for (size_t i = 0; i < sizeof(symModes) / sizeof(*symModes); i++)
			{
				const bool active = editor.symmetryMode == symModes[i].mode;
				if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				bool clicked = false;
#ifdef BBE_RENDERER_OPENGL
				ImTextureID symTex = nullptr;
				switch (i)
				{
				case 0: symTex = s_toolIcons.symmetryOff.texId; break;
				case 1: symTex = s_toolIcons.symmetryHorizontal.texId; break;
				case 2: symTex = s_toolIcons.symmetryVertical.texId; break;
				case 3: symTex = s_toolIcons.symmetryFourWay.texId; break;
				case 4: symTex = s_toolIcons.symmetryRadial.texId; break;
				default: break;
				}
				if (symTex)
					clicked = ImGui::ImageButton(symModes[i].id, symTex, ImVec2(symIconSize, symIconSize));
				else
#endif
				{
					clicked = ImGui::Button(symModes[i].fallback, ImVec2(symBtnW, 0));
				}
				if (clicked) editor.symmetryMode = symModes[i].mode;
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", symModes[i].tip);
				if (active) ImGui::PopStyleColor();
				if (i + 1 < sizeof(symModes) / sizeof(*symModes)) ImGui::SameLine();
			}
			if (editor.symmetryMode == bbe::SymmetryMode::Radial && ImGui::InputInt("Spokes##radialCount", &editor.radialSymmetryCount))
			{
				if (editor.radialSymmetryCount < 2) editor.radialSymmetryCount = 2;
			}
		}

		// --- Selection actions ---
		if (PaintEditor::isSelectionLikeTool(editor.mode))
		{
			ImGui::SeparatorText("Selection");
			ImGui::BeginDisabled(!editor.selection.hasSelection);
			if (ImGui::Button("Copy",   ImVec2(-1, 0))) editor.storeSelectionInClipboard();
			if (ImGui::Button("Cut",    ImVec2(-1, 0))) editor.cutSelection();
			if (ImGui::Button("Delete", ImVec2(-1, 0))) editor.deleteSelection();
			ImGui::EndDisabled();
			if (editor.selection.hasSelection)
				ImGui::Text("%d x %d px", editor.selection.rect.width, editor.selection.rect.height);
			else
				ImGui::TextDisabled("No selection");
		}

		// --- Clipboard ---
		const bool supportsClipboardImages = editor.platform.supportsClipboardImages && editor.platform.supportsClipboardImages();
		const bool clipboardHasImage = editor.platform.isClipboardImageAvailable && editor.platform.isClipboardImageAvailable();
		ImGui::SeparatorText("Clipboard");
		constexpr float clipIconSize = 24.f;
#ifdef BBE_RENDERER_OPENGL
		const bool clipboardUseIconRow = s_toolIcons.clipboardCopyCanvas.texId && s_toolIcons.clipboardPasteNew.texId;
#else
		const bool clipboardUseIconRow = false;
#endif
		ImGui::BeginDisabled(!supportsClipboardImages || !editor.platform.setClipboardImage);
#ifdef BBE_RENDERER_OPENGL
		if (clipboardUseIconRow)
		{
			if (ImGui::ImageButton("##clipCopyCanvas", s_toolIcons.clipboardCopyCanvas.texId, ImVec2(clipIconSize, clipIconSize)))
				editor.platform.setClipboardImage(editor.flattenVisibleLayers());
		}
		else
#endif
		{
			if (ImGui::Button("Copy Canvas to Clipboard", ImVec2(-1, 0)))
				editor.platform.setClipboardImage(editor.flattenVisibleLayers());
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Copy flattened visible canvas to the system clipboard as an image");
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!supportsClipboardImages || !clipboardHasImage || !editor.platform.getClipboardImage);
#ifdef BBE_RENDERER_OPENGL
		if (clipboardUseIconRow)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton("##clipPasteNew", s_toolIcons.clipboardPasteNew.texId, ImVec2(clipIconSize, clipIconSize)))
			{
				editor.canvas.get().layers.clear();
				bbe::Image pasted = editor.platform.getClipboardImage();
				editor.setCanvasFallbackFromImageAlpha(pasted);
				editor.canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, bbe::BlendMode::Normal, std::move(pasted) });
				editor.path = "";
				editor.submitCanvas();
				editor.setupCanvas(false);
			}
		}
		else
#endif
		{
			if (ImGui::Button("Paste as New Canvas", ImVec2(-1, 0)))
			{
				editor.canvas.get().layers.clear();
				bbe::Image pasted = editor.platform.getClipboardImage();
				editor.setCanvasFallbackFromImageAlpha(pasted);
				editor.canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, bbe::BlendMode::Normal, std::move(pasted) });
				editor.path = "";
				editor.submitCanvas();
				editor.setupCanvas(false);
			}
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Replace the document with the image from the clipboard (new single layer)");
		ImGui::EndDisabled();
		if (!supportsClipboardImages)
			ImGui::TextDisabled("Not supported on this platform");

		ImGui::EndChild();

		ImGui::Separator();
		ImGui::BeginChild("##layerDock", ImVec2(-1, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		// --- Layers ---
		ImGui::SeparatorText("Layers");
		static float s_canvasBackdropAlphaBackup = 1.f;
		bool canvasBackdropOn = editor.canvas.get().canvasFallbackRgba[3] > 0.001f;
		if (ImGui::Checkbox("Canvas backdrop", &canvasBackdropOn))
		{
			if (!canvasBackdropOn)
			{
				s_canvasBackdropAlphaBackup = editor.canvas.get().canvasFallbackRgba[3];
				editor.canvas.get().canvasFallbackRgba[3] = 0.f;
			}
			else
			{
				editor.canvas.get().canvasFallbackRgba[3] =
					(s_canvasBackdropAlphaBackup > 0.001f) ? s_canvasBackdropAlphaBackup : 1.f;
			}
			editor.submitCanvas();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!canvasBackdropOn);
		if (ImGui::ColorEdit4("##canvasFallback", editor.canvas.get().canvasFallbackRgba,
				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
		{
			if (editor.canvas.get().canvasFallbackRgba[3] > 0.001f)
				s_canvasBackdropAlphaBackup = editor.canvas.get().canvasFallbackRgba[3];
			editor.submitCanvas();
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Color behind all layers. Alpha 0 = transparent document when backdrop is off.");
		{
			const float layerIconSize = 24.f * editor.viewport.scale;
			const float layerRowW = ImGui::GetContentRegionAvail().x;
			const float layerHalfW = (layerRowW - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			const float btnW = (layerRowW - ImGui::GetStyle().ItemSpacing.x * 3) * 0.25f;
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.layerNew.texId)
			{
				if (ImGui::ImageButton("##layerNew", s_toolIcons.layerNew.texId, ImVec2(layerIconSize, layerIconSize))) editor.addLayer();
			}
			else
#endif
			{
				if (ImGui::Button("+ New", ImVec2(btnW * 1.5f, 0))) editor.addLayer();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("New layer");
			ImGui::SameLine();
			ImGui::BeginDisabled(editor.canvas.get().layers.getLength() <= 1);
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.layerDelete.texId)
			{
				if (ImGui::ImageButton("##layerDel", s_toolIcons.layerDelete.texId, ImVec2(layerIconSize, layerIconSize))) editor.deleteActiveLayer();
			}
			else
#endif
			{
				if (ImGui::Button("- Del", ImVec2(btnW * 1.5f, 0))) editor.deleteActiveLayer();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete active layer");
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled((size_t)editor.activeLayerIndex + 1 >= editor.canvas.get().layers.getLength());
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.layerUp.texId)
			{
				if (ImGui::ImageButton("##layerUp", s_toolIcons.layerUp.texId, ImVec2(layerIconSize, layerIconSize))) editor.moveActiveLayerUp();
			}
			else
#endif
			{
				if (ImGui::Button("Up", ImVec2(btnW, 0))) editor.moveActiveLayerUp();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move layer up (toward front)");
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(editor.activeLayerIndex <= 0);
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.layerDown.texId)
			{
				if (ImGui::ImageButton("##layerDn", s_toolIcons.layerDown.texId, ImVec2(layerIconSize, layerIconSize))) editor.moveActiveLayerDown();
			}
			else
#endif
			{
				if (ImGui::Button("Dn", ImVec2(btnW, 0))) editor.moveActiveLayerDown();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move layer down (toward back)");
			ImGui::EndDisabled();

			ImGui::Dummy(ImVec2(0, ImGui::GetStyle().ItemSpacing.y * 0.5f));
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.layerDuplicate.texId)
			{
				if (ImGui::ImageButton("##layerDup", s_toolIcons.layerDuplicate.texId, ImVec2(layerIconSize, layerIconSize))) editor.duplicateActiveLayer();
			}
			else
#endif
			{
				if (ImGui::Button("Dup", ImVec2(layerHalfW, 0))) editor.duplicateActiveLayer();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate active layer");
			ImGui::SameLine();
			ImGui::BeginDisabled(editor.activeLayerIndex <= 0);
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.layerMergeDown.texId)
			{
				if (ImGui::ImageButton("##layerMerge", s_toolIcons.layerMergeDown.texId, ImVec2(layerIconSize, layerIconSize))) editor.mergeActiveLayerDown();
			}
			else
#endif
			{
				if (ImGui::Button("Merge Dn", ImVec2(layerHalfW, 0))) editor.mergeActiveLayerDown();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Merge active layer into the one below");
			ImGui::EndDisabled();
		}
		if (!editor.canvas.get().layers.isEmpty())
		{
			if (ImGui::bbe::InputText("Name##layerName", editor.getActiveLayer().name))
			{
				editor.submitCanvas();
			}
			float opacity = editor.getActiveLayer().opacity;
			if (ImGui::SliderFloat("Opacity##layerOpacity", &opacity, 0.0f, 1.0f))
			{
				editor.getActiveLayer().opacity = opacity;
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				editor.submitCanvas();
			}
			const char *blendModeNames[] = { "Normal", "Multiply", "Screen", "Overlay" };
			int blendModeIdx = (int)editor.getActiveLayer().blendMode;
			if (ImGui::Combo("Blend##layerBlend", &blendModeIdx, blendModeNames, 4))
			{
				editor.getActiveLayer().blendMode = (bbe::BlendMode)blendModeIdx;
				editor.submitCanvas();
			}
		}
		if (ImGui::BeginChild("##layerList", ImVec2(-1, ImGui::GetContentRegionAvail().y), true))
		{
			for (int32_t layerIndex = (int32_t)editor.canvas.get().layers.getLength() - 1; layerIndex >= 0; layerIndex--)
			{
				PaintLayer &layer = editor.canvas.get().layers[(size_t)layerIndex];
				ImGui::PushID(layerIndex);
				bool visible = layer.visible;
				if (ImGui::Checkbox("##vis", &visible))
				{
					layer.visible = visible;
					editor.submitCanvas();
				}
				ImGui::SameLine();
				if (ImGui::Selectable(layer.name.getRaw(), editor.activeLayerIndex == layerIndex))
				{
					editor.setActiveLayerIndex(layerIndex);
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		ImGui::EndChild();

		ImGui::End();
		ImGui::PopStyleColor(2);

		drawPaintToolOptionsPanel(editor, PANEL_WIDTH);

		bool anyNonNormalBlendMode = false;
		for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
		{
			const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
			if (layer.visible && layer.blendMode != bbe::BlendMode::Normal)
			{
				anyNonNormalBlendMode = true;
				break;
			}
		}
		bbe::Image blendModePreview;
		if (anyNonNormalBlendMode)
		{
			blendModePreview = editor.flattenVisibleLayers();
		}

		const bbe::Color canvasBackdrop(
			editor.canvas.get().canvasFallbackRgba[0],
			editor.canvas.get().canvasFallbackRgba[1],
			editor.canvas.get().canvasFallbackRgba[2],
			editor.canvas.get().canvasFallbackRgba[3]);

		const int32_t repeats = editor.tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				const float tileX = editor.offset.x + i * editor.getCanvasWidth() * editor.zoomLevel;
				const float tileY = editor.offset.y + k * editor.getCanvasHeight() * editor.zoomLevel;
				const float tileW = editor.getCanvasWidth() * editor.zoomLevel;
				const float tileH = editor.getCanvasHeight() * editor.zoomLevel;
				brush.setColorRGB(canvasBackdrop);
				brush.fillRect(tileX, tileY, tileW, tileH);
				if (anyNonNormalBlendMode)
				{
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
					brush.drawImage(tileX, tileY, tileW, tileH, blendModePreview);
				}
				else
				{
					for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
					{
						const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
						if (!layer.visible) continue;
						brush.setColorRGB(1.0f, 1.0f, 1.0f, layer.opacity);
						brush.drawImage(tileX, tileY, tileW, tileH, layer.image);
					}
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
				}
				brush.drawImage(tileX, tileY, tileW, tileH, editor.workArea);
			}
		}
		if (editor.zoomLevel > 3 && editor.drawGridLines)
		{
			bbe::Vector2 zeroPos = editor.screenToCanvas({ 0, 0 });
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.5f);
			for (float i = -(zeroPos.x - (int)zeroPos.x) * editor.zoomLevel; i < (float)editor.viewport.width; i += editor.zoomLevel)
			{
				brush.fillLine(i, 0, i, (float)editor.viewport.height);
			}
			for (float i = -(zeroPos.y - (int)zeroPos.y) * editor.zoomLevel; i < (float)editor.viewport.height; i += editor.zoomLevel)
			{
				brush.fillLine(0, i, (float)editor.viewport.width, i);
			}
		}
		// Canvas resize handles (hidden while a selection exists so marquee handles stay unambiguous)
		if (editor.getCanvasWidth() > 0 && editor.getCanvasHeight() > 0 && !editor.tiled && !editor.selection.hasSelection)
		{
			constexpr float hs = 5.f;
			for (int32_t i = 0; i < 8; i++)
			{
				const bbe::Vector2 hp = editor.getCanvasHandleScreenPos(i);
				brush.setColorRGB(1.f, 1.f, 1.f);
				brush.fillRect(hp.x - hs, hp.y - hs, hs * 2.f, hs * 2.f);
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.sketchRect(bbe::Rectangle(hp.x - hs, hp.y - hs, hs * 2.f, hs * 2.f));
			}
			if (editor.canvasResizeActive && editor.canvasResizePreviewRect.width > 0 && editor.canvasResizePreviewRect.height > 0)
			{
				const bbe::Rectangle previewScreen = editor.selectionRectToScreen(editor.canvasResizePreviewRect);
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.sketchRect(previewScreen);
				if (previewScreen.width > 2 && previewScreen.height > 2)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.sketchRect(previewScreen.shrinked(1.f));
				}
			}
		}

		const int32_t ghostRepeats = editor.tiled ? 20 : 0;
		auto drawInAllTiles = [&](const bbe::Rectanglei &rect, const bbe::Image &image, float rotation = 0.f)
		{
			// Pre-rasterize rotation so the preview matches the committed pixel-grid result.
			const bool hasRot = std::abs(rotation) > 0.0001f;
			bbe::Image rotatedImg;
			bbe::Rectanglei displayRect = rect;
			const bbe::Image *pImg = &image;
			if (hasRot)
			{
				rotatedImg = image.rotatedToFit(rotation, editor.antiAliasingEnabled);
				if (rotatedImg.getWidth() > 0 && rotatedImg.getHeight() > 0)
				{
					pImg = &rotatedImg;
					const float cx = rect.x + rect.width / 2.f;
					const float cy = rect.y + rect.height / 2.f;
					displayRect = bbe::Rectanglei(
						(int32_t)std::floor(cx - rotatedImg.getWidth() / 2.f),
						(int32_t)std::floor(cy - rotatedImg.getHeight() / 2.f),
						rotatedImg.getWidth(),
						rotatedImg.getHeight());
				}
			}
			for (int32_t i = -ghostRepeats; i <= ghostRepeats; i++)
			{
				for (int32_t k = -ghostRepeats; k <= ghostRepeats; k++)
				{
					const bbe::Rectanglei tileDisplay(
						displayRect.x + i * editor.getCanvasWidth(),
						displayRect.y + k * editor.getCanvasHeight(),
						displayRect.width,
						displayRect.height);
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
					brush.drawImage(editor.selectionRectToScreen(tileDisplay), *pImg);
					const bbe::Rectanglei tileOutline(
						rect.x + i * editor.getCanvasWidth(),
						rect.y + k * editor.getCanvasHeight(),
						rect.width,
						rect.height);
					drawSelectionOutlineForGui(brush, editor, tileOutline, false);
				}
			}
		};

		if (editor.rectangle.dragActive && editor.rectangle.dragPreviewRect.width > 0 && editor.rectangle.dragPreviewRect.height > 0)
		{
			drawInAllTiles(editor.rectangle.dragPreviewRect, editor.rectangle.dragPreviewImage);
		}
		else if (editor.circle.dragActive && editor.circle.dragPreviewRect.width > 0 && editor.circle.dragPreviewRect.height > 0)
		{
			drawInAllTiles(editor.circle.dragPreviewRect, editor.circle.dragPreviewImage);
		}
		else if (editor.selection.moveActive || editor.selection.resizeActive)
		{
			const bbe::Image previewImage = editor.selection.resizeActive ? editor.buildSelectionPreviewResultImage() : editor.selection.previewImage;
			drawInAllTiles(editor.selection.previewRect, previewImage, editor.selection.rotation);
		}
		else if (editor.selection.floating)
		{
			if (!editor.antiAliasingEnabled && std::abs(editor.selection.rotation) > 0.01f && (editor.rectangle.draftActive || editor.circle.draftActive))
			{
				// AA-off + rotation: re-render from SDF so preview matches the committed result.
				const bbe::Colori color = editor.rectangle.draftActive ? editor.getRectangleDraftColor() : editor.getCircleDraftColor();
				const bbe::Image img = editor.rectangle.draftActive
					? editor.createRectangleImage(editor.selection.rect.width, editor.selection.rect.height, color, editor.selection.rotation, editor.rectangle.draftUsesRightColor)
					: editor.createCircleImage(editor.selection.rect.width, editor.selection.rect.height, color, editor.selection.rotation, editor.circle.draftUsesRightColor);
				const float cx = editor.selection.rect.x + editor.selection.rect.width  * 0.5f;
				const float cy = editor.selection.rect.y + editor.selection.rect.height * 0.5f;
				const bbe::Rectanglei bbRect(
					(int32_t)std::floor(cx - img.getWidth()  * 0.5f),
					(int32_t)std::floor(cy - img.getHeight() * 0.5f),
					img.getWidth(), img.getHeight());
				drawInAllTiles(bbRect, img);
			}
			else
			{
				drawInAllTiles(editor.selection.rect, editor.selection.floatingImage, editor.selection.rotation);
			}
		}
		else if (editor.selection.dragActive)
		{
			if (editor.mode == PaintEditor::MODE_ELLIPSE_SELECTION && editor.selection.previewRect.width > 0 && editor.selection.previewRect.height > 0)
			{
				const int32_t W = editor.getCanvasWidth();
				const int32_t H = editor.getCanvasHeight();
				const float lwOuter = std::max(2.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f) + 1.f;
				const float lwInner = std::max(1.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f);
				const bbe::Rectanglei &r = editor.selection.previewRect;
				const float cx = r.x + r.width * 0.5f;
				const float cy = r.y + r.height * 0.5f;
				const float rx = r.width * 0.5f;
				const float ry = r.height * 0.5f;
				constexpr int seg = 64;
				for (int32_t ti = -ghostRepeats; ti <= ghostRepeats; ti++)
				{
					for (int32_t tk = -ghostRepeats; tk <= ghostRepeats; tk++)
					{
						const int32_t ox = ti * W;
						const int32_t oy = tk * H;
						bbe::List<bbe::Vector2> strip;
						for (int i = 0; i < seg; i++)
						{
							const double t = (i / (double)seg) * (2.0 * 3.14159265358979323846);
							const float px = (float)(cx + std::cos(t) * rx) + (float)ox;
							const float py = (float)(cy + std::sin(t) * ry) + (float)oy;
							const bbe::Rectangle scr = editor.selectionRectToScreen(bbe::Rectanglei((int32_t)std::floor(px), (int32_t)std::floor(py), 1, 1));
							strip.add({ scr.x + scr.width * 0.5f, scr.y + scr.height * 0.5f });
						}
						brush.setColorRGB(0.f, 0.f, 0.f);
						brush.fillLineStrip(strip, true, lwOuter);
						brush.setColorRGB(1.f, 1.f, 1.f);
						brush.fillLineStrip(strip, true, lwInner);
					}
				}
			}
			else
			{
				drawSelectionOutlineForGui(brush, editor, editor.selection.previewRect, false);
			}
		}
		else if (editor.selection.lassoDragActive && editor.selection.lassoPath.size() >= 2)
		{
			const int32_t W = editor.getCanvasWidth();
			const int32_t H = editor.getCanvasHeight();
			const float lwOuter = std::max(2.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f) + 1.f;
			const float lwInner = std::max(1.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f);
			for (int32_t ti = -ghostRepeats; ti <= ghostRepeats; ti++)
			{
				for (int32_t tk = -ghostRepeats; tk <= ghostRepeats; tk++)
				{
					const int32_t ox = ti * W;
					const int32_t oy = tk * H;
					bbe::List<bbe::Vector2> strip;
					for (size_t pi = 0; pi < editor.selection.lassoPath.size(); pi++)
					{
						const bbe::Vector2 &pf = editor.selection.lassoPath[pi];
						const float sx = editor.offset.x + (pf.x + (float)ox) * editor.zoomLevel;
						const float sy = editor.offset.y + (pf.y + (float)oy) * editor.zoomLevel;
						strip.add({ sx, sy });
					}
					brush.setColorRGB(0.f, 0.f, 0.f);
					brush.fillLineStrip(strip, false, lwOuter);
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.fillLineStrip(strip, false, lwInner);
				}
			}
		}
		else if (editor.mode == PaintEditor::MODE_POLYGON_LASSO && !editor.selection.polygonLassoVertices.empty())
		{
			const int32_t W = editor.getCanvasWidth();
			const int32_t H = editor.getCanvasHeight();
			const float lwOuter = std::max(2.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f) + 1.f;
			const float lwInner = std::max(1.f, editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f);
			for (int32_t ti = -ghostRepeats; ti <= ghostRepeats; ti++)
			{
				for (int32_t tk = -ghostRepeats; tk <= ghostRepeats; tk++)
				{
					const int32_t ox = ti * W;
					const int32_t oy = tk * H;
					bbe::List<bbe::Vector2> strip;
					for (size_t pi = 0; pi < editor.selection.polygonLassoVertices.size(); pi++)
					{
						const bbe::Vector2i &p = editor.selection.polygonLassoVertices[pi];
						const bbe::Rectangle scr = editor.selectionRectToScreen(bbe::Rectanglei(p.x + ox, p.y + oy, 1, 1));
						strip.add({ scr.x + scr.width * 0.5f, scr.y + scr.height * 0.5f });
					}
					if (editor.hasPointerPos && W > 0 && H > 0)
					{
						const int32_t cx = bbe::Math::clamp((int32_t)std::floor(editor.lastPointerCanvasPos.x), 0, W - 1);
						const int32_t cy = bbe::Math::clamp((int32_t)std::floor(editor.lastPointerCanvasPos.y), 0, H - 1);
						const bbe::Rectangle scrC = editor.selectionRectToScreen(bbe::Rectanglei(cx + ox, cy + oy, 1, 1));
						strip.add({ scrC.x + scrC.width * 0.5f, scrC.y + scrC.height * 0.5f });
					}
					brush.setColorRGB(0.f, 0.f, 0.f);
					brush.fillLineStrip(strip, false, lwOuter);
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.fillLineStrip(strip, false, lwInner);
					if (editor.selection.polygonLassoVertices.size() >= 3)
					{
						const bbe::Vector2i &fp = editor.selection.polygonLassoVertices[0];
						const bbe::Rectangle scrF = editor.selectionRectToScreen(bbe::Rectanglei(fp.x + ox, fp.y + oy, 1, 1));
						const float fx = scrF.x + scrF.width * 0.5f;
						const float fy = scrF.y + scrF.height * 0.5f;
						const float r = std::max(3.f, lwOuter);
						brush.setColorRGB(0.f, 0.f, 0.f);
						brush.fillCircle(fx - r - 1.f, fy - r - 1.f, (r + 1.f) * 2.f, (r + 1.f) * 2.f);
						brush.setColorRGB(1.f, 1.f, 0.4f);
						brush.fillCircle(fx - r, fy - r, r * 2.f, r * 2.f);
					}
				}
			}
		}
		else if (editor.selection.hasSelection)
		{
			const bool overlayMask = editor.hasSelectionPixelMask() && !editor.selection.moveActive && !editor.selection.resizeActive && !editor.selection.dragActive &&
									 !editor.selection.lassoDragActive && editor.selection.polygonLassoVertices.empty() &&
									 !editor.selection.floating && std::abs(editor.selection.rotation) < 0.0001f;
			for (int32_t ti = -ghostRepeats; ti <= ghostRepeats; ti++)
			{
				for (int32_t tk = -ghostRepeats; tk <= ghostRepeats; tk++)
				{
					const bbe::Rectanglei tileR(
						editor.selection.rect.x + ti * editor.getCanvasWidth(),
						editor.selection.rect.y + tk * editor.getCanvasHeight(),
						editor.selection.rect.width,
						editor.selection.rect.height);
					drawSelectionOutlineForGui(brush, editor, tileR, false);
					if (overlayMask) drawSelectionPixelMaskOverlayForGui(brush, editor, tileR);
				}
			}
		}
		if (editor.mode == PaintEditor::MODE_TEXT)
		{
			bbe::Vector2 previewPos = editor.screenToCanvas(mouseScreenPos);
			if (editor.toTiledPos(previewPos))
			{
				drawTextPreviewForGui(brush, editor, editor.toCanvasPixel(previewPos));
			}
		}
		if (editor.mode == PaintEditor::MODE_ERASER && editor.getCanvasWidth() > 0 && editor.getCanvasHeight() > 0)
		{
			bbe::Vector2 previewCenter = editor.screenToCanvas(mouseScreenPos);
			if (editor.toTiledPos(previewCenter))
			{
				const int32_t cw = editor.getCanvasWidth();
				const int32_t ch = editor.getCanvasHeight();
				const int32_t tileRepeat = editor.tiled ? 20 : 0;
				const float z = editor.zoomLevel;
				constexpr float line = 1.f;
				brush.setColorRGB(1.f, 0.35f, 0.35f, 0.9f);
				auto drawEraserStampScreenOutline = [&](const bbe::Rectanglei &stamp)
				{
					for (int32_t ti = -tileRepeat; ti <= tileRepeat; ti++)
					{
						for (int32_t tk = -tileRepeat; tk <= tileRepeat; tk++)
						{
							const float ox = editor.offset.x + (float)(ti * cw) * z;
							const float oy = editor.offset.y + (float)(tk * ch) * z;
							const bbe::Rectangle scr(ox + (float)stamp.x * z, oy + (float)stamp.y * z, (float)stamp.width * z, (float)stamp.height * z);
							brush.fillRect(scr.x, scr.y, scr.width, line);
							brush.fillRect(scr.x, scr.y + scr.height - line, scr.width, line);
							brush.fillRect(scr.x, scr.y, line, scr.height);
							brush.fillRect(scr.x + scr.width - line, scr.y, line, scr.height);
						}
					}
				};
				if (editor.eraserPreviewSegmentActive)
				{
					bbe::List<bbe::Rectanglei> segStamps;
					editor.appendEraserStampRectsAlongSegment(editor.eraserPreviewSegmentPNew, editor.eraserPreviewSegmentPOld, segStamps);
					for (size_t ri = 0; ri < segStamps.getLength(); ri++)
					{
						drawEraserStampScreenOutline(segStamps[ri]);
					}
				}
				else
				{
					const bbe::List<bbe::Vector2> centers = editor.getSymmetryPositions(previewCenter);
					for (size_t si = 0; si < centers.getLength(); si++)
					{
						drawEraserStampScreenOutline(editor.getEraserPixelRect(centers[si]));
					}
				}
			}
		}
		if (editor.mode == PaintEditor::MODE_SPRAY && editor.getCanvasWidth() > 0 && editor.getCanvasHeight() > 0)
		{
			bbe::Vector2 previewCenter = editor.screenToCanvas(mouseScreenPos);
			if (editor.toTiledPos(previewCenter))
			{
				const int32_t cw = editor.getCanvasWidth();
				const int32_t ch = editor.getCanvasHeight();
				const int32_t tileRepeat = editor.tiled ? 20 : 0;
				const float z = editor.zoomLevel;
				const float rad = (float)editor.sprayWidth * z;
				const bbe::List<bbe::Vector2> centers = editor.getSymmetryPositions(previewCenter);
				for (size_t si = 0; si < centers.getLength(); si++)
				{
					for (int32_t ti = -tileRepeat; ti <= tileRepeat; ti++)
					{
						for (int32_t tk = -tileRepeat; tk <= tileRepeat; tk++)
						{
							const float cx = editor.offset.x + (centers[si].x + (float)(ti * cw)) * z;
							const float cy = editor.offset.y + (centers[si].y + (float)(tk * ch)) * z;
							const float r = std::max(1.f, rad);
							brush.setColorRGB(0.f, 0.f, 0.f, 0.9f);
							brush.fillCircle(cx - r - 1.f, cy - r - 1.f, (r + 1.f) * 2.f, (r + 1.f) * 2.f);
							brush.setColorRGB(0.35f, 0.9f, 1.f, 0.35f);
							brush.fillCircle(cx - r, cy - r, r * 2.f, r * 2.f);
						}
					}
				}
			}
		}
		auto drawEndpointHandle = [&](const bbe::Vector2 &canvasPos)
		{
			const float sx = editor.offset.x + canvasPos.x * editor.zoomLevel;
			const float sy = editor.offset.y + canvasPos.y * editor.zoomLevel;
			constexpr float hs = 4.f;
			brush.setColorRGB(1.f, 1.f, 1.f);
			brush.fillRect(sx - hs, sy - hs, hs * 2.f, hs * 2.f);
			brush.setColorRGB(0.f, 0.f, 0.f);
			brush.sketchRect(bbe::Rectangle(sx - hs, sy - hs, hs * 2.f, hs * 2.f));
		};
		if (editor.mode == PaintEditor::MODE_LINE && editor.line.draftActive)
		{
			drawEndpointHandle(editor.line.start);
			drawEndpointHandle(editor.line.end);
		}
		if (editor.mode == PaintEditor::MODE_ARROW && editor.arrow.draftActive)
		{
			drawEndpointHandle(editor.arrow.start);
			drawEndpointHandle(editor.arrow.end);
		}
		if (editor.mode == PaintEditor::MODE_BEZIER && !editor.bezier.controlPoints.isEmpty())
		{
			// Draw the control polygon
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.6f);
			for (size_t i = 0; i + 1 < editor.bezier.controlPoints.getLength(); i++)
			{
				const float x0 = editor.offset.x + editor.bezier.controlPoints[i    ].x * editor.zoomLevel;
				const float y0 = editor.offset.y + editor.bezier.controlPoints[i    ].y * editor.zoomLevel;
				const float x1 = editor.offset.x + editor.bezier.controlPoints[i + 1].x * editor.zoomLevel;
				const float y1 = editor.offset.y + editor.bezier.controlPoints[i + 1].y * editor.zoomLevel;
				brush.fillLine(x0, y0, x1, y1);
			}
			// Draw handles for each control point
			for (size_t i = 0; i < editor.bezier.controlPoints.getLength(); i++)
			{
				drawEndpointHandle(editor.bezier.controlPoints[i]);
			}
		}

		// Symmetry guide lines
		if (editor.symmetryMode != bbe::SymmetryMode::None && editor.getCanvasWidth() > 0)
		{
			const float cw = (float)editor.getCanvasWidth();
			const float ch = (float)editor.getCanvasHeight();
			const bbe::Vector2 center = editor.getSymmetryCenter();
			// Convert editor.canvas coords to screen coords: screen = pos * editor.zoomLevel + editor.offset
			auto c2s = [&](bbe::Vector2 p) -> bbe::Vector2
			{
				return p * editor.zoomLevel + editor.offset;
			};

			brush.setColorRGB(0.2f, 0.8f, 1.0f, 0.7f);
			brush.setOutlineWidth(0.f);

			if (editor.symmetryMode == bbe::SymmetryMode::Horizontal || editor.symmetryMode == bbe::SymmetryMode::FourWay)
			{
				const bbe::Vector2 a = c2s({ 0.f,  center.y });
				const bbe::Vector2 b = c2s({ cw,   center.y });
				brush.fillLine(a, b, 1.f);
			}
			if (editor.symmetryMode == bbe::SymmetryMode::Vertical || editor.symmetryMode == bbe::SymmetryMode::FourWay)
			{
				const bbe::Vector2 a = c2s({ center.x, 0.f });
				const bbe::Vector2 b = c2s({ center.x, ch  });
				brush.fillLine(a, b, 1.f);
			}
			if (editor.symmetryMode == bbe::SymmetryMode::Radial)
			{
				const float step = 2.f * bbe::Math::PI / (float)editor.radialSymmetryCount;
				const float extent = bbe::Math::sqrt(cw * cw + ch * ch) * 0.5f;
				for (int32_t i = 0; i < editor.radialSymmetryCount; i++)
				{
					const float angle = step * (float)i;
					const bbe::Vector2 dir = { std::cosf(angle) * extent, std::sinf(angle) * extent };
					brush.fillLine(c2s(center), c2s(center + dir), 1.f);
				}
			}
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Navigator
		if (editor.showNavigator && editor.getCanvasWidth() > 0)
		{
			const bbe::Rectangle navRect = editor.getNavigatorRect();
			const float navX = navRect.x;
			const float navY = navRect.y;
			const float navW = navRect.width;
			const float navH = navRect.height;

			// Background
			brush.setColorRGB(0.08f, 0.08f, 0.08f);
			brush.fillRect(navX - 2.f, navY - 2.f, navW + 4.f, navH + 4.f);

			// Layers (canvas backdrop then composited image)
			brush.setColorRGB(canvasBackdrop);
			brush.fillRect(navX, navY, navW, navH);
			if (anyNonNormalBlendMode)
			{
				brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
				brush.drawImage(navX, navY, navW, navH, blendModePreview);
			}
			else
			{
				for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
				{
					const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
					if (!layer.visible) continue;
					brush.setColorRGB(1.0f, 1.0f, 1.0f, layer.opacity);
					brush.drawImage(navX, navY, navW, navH, layer.image);
				}
			}
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);

			// Viewport editor.rectangle (clamped to navigator bounds)
			const float scaleX = navW / editor.getCanvasWidth();
			const float scaleY = navH / editor.getCanvasHeight();
			const bbe::Vector2 tlCanvas = editor.screenToCanvas({ 0.f, 0.f });
			const bbe::Vector2 brCanvas = editor.screenToCanvas({ (float)editor.viewport.width, (float)editor.viewport.height });
			const float vx1 = bbe::Math::clamp(navX + tlCanvas.x * scaleX, navX, navX + navW);
			const float vy1 = bbe::Math::clamp(navY + tlCanvas.y * scaleY, navY, navY + navH);
			const float vx2 = bbe::Math::clamp(navX + brCanvas.x * scaleX, navX, navX + navW);
			const float vy2 = bbe::Math::clamp(navY + brCanvas.y * scaleY, navY, navY + navH);
			brush.setColorRGB(1.0f, 1.0f, 0.0f);
			brush.sketchRect(vx1, vy1, vx2 - vx1, vy2 - vy1);
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// HACK: We can only open popups if we are in the same ID Stack. See: https://github.com/ocornut/imgui/issues/331
		bool openNewCanvas = false;
		bool openChangeCanvasSize = false;
		bool openMirrorImage = false;
		bool openRotateCanvas = false;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("New..."))
				{
					openNewCanvas = true;
				}
				if (ImGui::MenuItem("Open..."))
				{
					bbe::String newPath = editor.path;
					if (editor.platform.showOpenDialog && editor.platform.showOpenDialog(newPath))
					{
						editor.newCanvas(newPath.getRaw());
					}
				}
				if (ImGui::MenuItem("Save"))
				{
					editor.saveCanvas();
				}
				if (ImGui::MenuItem("Save As PNG..."))
				{
					editor.saveDocumentAs(PaintEditor::SaveFormat::PNG);
				}
				if (ImGui::MenuItem("Save As Layered..."))
				{
					editor.saveDocumentAs(PaintEditor::SaveFormat::LAYERED);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Change Canvas Size..."))
				{
					openChangeCanvasSize = true;
				}
				if (ImGui::MenuItem("Mirror..."))
				{
					openMirrorImage = true;
				}
				if (ImGui::MenuItem("Rotate Canvas 90°..."))
				{
					openRotateCanvas = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				auto toggleMenuItem = [&](const char *label, bool &value) { if (ImGui::MenuItem(label, nullptr, value)) value = !value; };
				toggleMenuItem("Draw Grid Lines", editor.drawGridLines);
				toggleMenuItem("Tiled", editor.tiled);
				toggleMenuItem("Navigator", editor.showNavigator);
				toggleMenuItem("Tool Options", editor.showToolOptionsPanel);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Preferences"))
			{
				if (ImGui::MenuItem("Anti-Aliasing", nullptr, editor.antiAliasingEnabled))
				{
					editor.antiAliasingEnabled = !editor.antiAliasingEnabled;
					editor.refreshBrushBasedDrafts();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Show Help"))
				{
					editor.showHelpWindow = true;
				}
				ImGui::EndMenu();
			}
			if (editor.canvasGeneration != editor.savedGeneration)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f);
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "*");
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Unsaved changes");
			}
			ImGui::EndMainMenuBar();
		}

		if (editor.openSaveChoicePopup)
		{
			ImGui::OpenPopup("Save Document");
			editor.openSaveChoicePopup = false;
		}
		if (editor.openSaveFailedPopup)
		{
			ImGui::OpenPopup("Save failed");
			editor.openSaveFailedPopup = false;
		}
		if (editor.openDropChoicePopup)
		{
			ImGui::OpenPopup("Dropped File(s)");
			editor.openDropChoicePopup = false;
		}
		if (openMirrorImage)
		{
			ImGui::OpenPopup("Mirror image");
			openMirrorImage = false;
		}
		if (openRotateCanvas)
		{
			ImGui::OpenPopup("Rotate canvas");
			openRotateCanvas = false;
		}
		if (ImGui::BeginPopupModal("Save Document", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Choose a save format for this document.");
			if (ImGui::Button("PNG", ImVec2(120, 0)))
			{
				editor.saveDocumentAs(PaintEditor::SaveFormat::PNG);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Layered", ImVec2(120, 0)))
			{
				editor.saveDocumentAs(PaintEditor::SaveFormat::LAYERED);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Save failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("The file could not be written (permission denied, read-only location, or full disk).");
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Dropped File(s)", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (editor.pendingDroppedPaths.getLength() == 1)
			{
				ImGui::Text("What would you like to do with \"%s\"?",
					std::filesystem::path(editor.pendingDroppedPaths[0].getRaw()).filename().string().c_str());
			}
			else
			{
				ImGui::Text("What would you like to do with %d dropped file(s)?",
					(int)editor.pendingDroppedPaths.getLength());
			}
			ImGui::Spacing();
			if (ImGui::Button("Open as Document", ImVec2(160, 0)))
			{
				// Use only the first valid file as the new document
				editor.newCanvas(editor.pendingDroppedPaths[0].getRaw());
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add as Layer(s)", ImVec2(160, 0)))
			{
				editor.importFileAsLayers(editor.pendingDroppedPaths);
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImVec2 mirrorCenter = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(mirrorCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Mirror image", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("Mirror the entire document (all layers).");
			ImGui::Spacing();
			ImGui::BeginDisabled(editor.getCanvasWidth() <= 0 || editor.getCanvasHeight() <= 0);
			if (ImGui::Button("Mirror vertically", ImVec2(200, 0)))
			{
				editor.mirrorAllLayersVertically();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Mirror horizontally", ImVec2(200, 0)))
			{
				editor.mirrorAllLayersHorizontally();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();
			ImGui::Spacing();
			if (ImGui::Button("Cancel", ImVec2(200, 0))) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImVec2 rotateCenter = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(rotateCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Rotate canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("Rotate the entire document (all layers). Width and height are swapped.");
			ImGui::Spacing();
			ImGui::BeginDisabled(editor.getCanvasWidth() <= 0 || editor.getCanvasHeight() <= 0);
			if (ImGui::Button("90° clockwise", ImVec2(200, 0)))
			{
				editor.rotateAllLayers90Clockwise();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("90° counter-clockwise", ImVec2(200, 0)))
			{
				editor.rotateAllLayers90CounterClockwise();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();
			ImGui::Spacing();
			if (ImGui::Button("Cancel", ImVec2(200, 0))) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (editor.showHelpWindow)
		{
			if (ImGui::Begin("ExamplePaint Help", &editor.showHelpWindow))
			{
				auto bulletList = [&](const char *title, std::initializer_list<const char *> items)
				{
					ImGui::SeparatorText(title);
					for (const char *item : items) ImGui::BulletText("%s", item);
				};
				bulletList("Tools", { "1 Brush", "2 Flood Fill", "3 Line", "4 Rectangle", "5 Selection", "6 Text", "7 Pipette", "8 Circle", "9 Arrow", "0 Bezier", "E Eraser", "R Spray", "O Ellipse selection", "L Lasso", "P Polygon Lasso", "M Magic Wand" });
				bulletList("General", { "+/- changes brush width, eraser size, spray width (spray tool), wand or flood-fill tolerance, or text size for the active tool", "X swaps primary and secondary color", "Ctrl+D resets colors to black/white", "Drag and drop PNG or .bbepaint files to open as a document or add as a new layer", "Space resets the camera", "Middle mouse pans", "Mouse wheel zooms" });
				bulletList("Edit", { "Ctrl+S saves", "Ctrl+Z / Ctrl+Y undo and redo", "Delete / Backspace deletes the current selection", "Edit → Mirror flips all layers (vertical or horizontal in the dialog)", "Edit → Rotate Canvas 90° turns all layers; canvas width and height swap" });
				bulletList("Selection", { "Drag to create a rectangular selection", "Ellipse selection: drag for an elliptical marquee; hold Shift for a circle", "Lasso: click and drag to outline an area (closed automatically)", "Polygon lasso: click corners, then close via first point, Enter, or right-click", "Magic Wand selects by similar color (visible flatten) with adjustable tolerance", "Ctrl+click with Magic Wand, Selection, Ellipse selection, Lasso, or Polygon lasso adds to the current selection", "Drag inside a selection to move it", "Drag corner or edge handles to resize", "Rectangle creates a floating selection first; click outside to place it", "Ctrl+A selects the whole active layer", "Ctrl+C / Ctrl+X / Ctrl+V copy, cut and paste" });
				bulletList("Layers", { "Painting and text placement affect only the active layer", "Canvas backdrop defaults to opaque white behind all layers; set alpha to 0 on the backdrop for a fully transparent document", "Visible layers are flattened when saving as PNG", "Save as Layered keeps all layers in .bbepaint", "Opening PNG still works as a normal single-layer document" });
			}
			ImGui::End();
		}

		// --- Change Canvas Size popup ---
		{
			enum CanvasResizeMode { RESIZE_CROP_EXTEND, RESIZE_SCALE_NEAREST, RESIZE_SCALE_BILINEAR };
			static int resizeW = 0;
			static int resizeH = 0;
			static int resizeMode = RESIZE_CROP_EXTEND;
			static int anchorX = 1; // 0=left, 1=center, 2=right
			static int anchorY = 1; // 0=top,  1=center, 2=bottom

			if (openChangeCanvasSize)
			{
				ImGui::OpenPopup("Change Canvas Size");
				resizeW = editor.getCanvasWidth();
				resizeH = editor.getCanvasHeight();
				resizeMode = RESIZE_CROP_EXTEND;
				anchorX = 0;
				anchorY = 0;
			}
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Change Canvas Size", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Current: %d x %d", editor.getCanvasWidth(), editor.getCanvasHeight());
				ImGui::Spacing();
				ImGui::InputInt("Width",  &resizeW);
				ImGui::InputInt("Height", &resizeH);
				if (resizeW < 1) resizeW = 1;
				if (resizeH < 1) resizeH = 1;
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				const char *modeNames[] = { "Crop / Extend", "Scale (Nearest)", "Scale (Bilinear)" };
				ImGui::Combo("Mode", &resizeMode, modeNames, IM_ARRAYSIZE(modeNames));

				if (resizeMode == RESIZE_CROP_EXTEND)
				{
					ImGui::Spacing();
					ImGui::Text("Anchor:");
					const float btnSize = ImGui::GetFrameHeight();
					for (int row = 0; row < 3; row++)
					{
						for (int col = 0; col < 3; col++)
						{
							if (col > 0) ImGui::SameLine();
							const bool selected = (anchorX == col && anchorY == row);
							if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
							char id[16];
							snprintf(id, sizeof(id), "##a%d%d", row, col);
							if (ImGui::Button(id, ImVec2(btnSize, btnSize)))
							{
								anchorX = col;
								anchorY = row;
							}
							if (selected) ImGui::PopStyleColor();
						}
					}
					ImGui::Text("New area: transparent pixels");
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					const int32_t oldW = editor.getCanvasWidth();
					const int32_t oldH = editor.getCanvasHeight();

					if (resizeMode == RESIZE_CROP_EXTEND)
					{
						int32_t offX = 0, offY = 0;
						if (anchorX == 1) offX = (resizeW - oldW) / 2;
						if (anchorX == 2) offX = resizeW - oldW;
						if (anchorY == 1) offY = (resizeH - oldH) / 2;
						if (anchorY == 2) offY = resizeH - oldH;
						const bbe::Color fillColor(0.f, 0.f, 0.f, 0.f);
						for (size_t li = 0; li < editor.canvas.get().layers.getLength(); li++)
						{
							editor.canvas.get().layers[li].image = editor.canvas.get().layers[li].image.resizedCanvas(
								resizeW, resizeH, bbe::Vector2i(offX, offY), fillColor);
							editor.prepareImageForCanvas(editor.canvas.get().layers[li].image);
						}
						editor.offset.x -= offX * editor.zoomLevel;
						editor.offset.y -= offY * editor.zoomLevel;
					}
					else if (resizeMode == RESIZE_SCALE_NEAREST)
					{
						for (size_t li = 0; li < editor.canvas.get().layers.getLength(); li++)
						{
							editor.canvas.get().layers[li].image = editor.canvas.get().layers[li].image.scaledNearest(resizeW, resizeH);
							editor.prepareImageForCanvas(editor.canvas.get().layers[li].image);
						}
					}
					else if (resizeMode == RESIZE_SCALE_BILINEAR)
					{
						for (size_t li = 0; li < editor.canvas.get().layers.getLength(); li++)
						{
							const bbe::Image &src = editor.canvas.get().layers[li].image;
							bbe::Image dst(resizeW, resizeH, bbe::Color(0.f, 0.f, 0.f, 0.f));
							for (int32_t y = 0; y < resizeH; y++)
							{
								for (int32_t x = 0; x < resizeW; x++)
								{
									const float srcX = ((float)x + 0.5f) * oldW / resizeW - 0.5f;
									const float srcY = ((float)y + 0.5f) * oldH / resizeH - 0.5f;
									dst.setPixel(x, y, src.sampleBilinearPremultiplied(srcX, srcY));
								}
							}
							editor.canvas.get().layers[li].image = std::move(dst);
							editor.prepareImageForCanvas(editor.canvas.get().layers[li].image);
						}
					}
					editor.clearSelectionState();
					editor.clearWorkArea();
					editor.submitCanvas();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		// --- New Canvas popup ---
		{
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			static int newWidth = 0;
			static int newHeight = 0;
			if (openNewCanvas)
			{
				ImGui::OpenPopup("New Canvas");
				newWidth = editor.getCanvasWidth();
				newHeight = editor.getCanvasHeight();
			}
			if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputInt("Width", &newWidth);
				ImGui::SameLine();
				ImGui::InputInt("Height", &newHeight);
				if (newWidth < 1) newWidth = 1;
				if (newHeight < 1) newHeight = 1;
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					editor.newCanvas((uint32_t)newWidth, (uint32_t)newHeight);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
}
