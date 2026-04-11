#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>

#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.
#include "ExamplePaintEditor.h"
#include "ExamplePaintGui.h"
/// Persisted 1–9 / 0 digit bindings (one \c int32_t action per key; \c formatVersion distinguishes this from older multi-field saves).
struct ExamplePaintDigitHotkeysPersist
{
	static constexpr int32_t kFormatVersion = 2;
	int32_t formatVersion = kFormatVersion;
	std::array<int32_t, 10> action{};

	ExamplePaintDigitHotkeysPersist()
	{
		formatVersion = kFormatVersion;
		const int32_t defaultTool[10] = {
			PaintEditor::MODE_BRUSH,
			PaintEditor::MODE_FLOOD_FILL,
			PaintEditor::MODE_LINE,
			PaintEditor::MODE_RECTANGLE,
			PaintEditor::MODE_SELECTION,
			PaintEditor::MODE_TEXT,
			PaintEditor::MODE_PIPETTE,
			PaintEditor::MODE_CIRCLE,
			PaintEditor::MODE_ARROW,
			PaintEditor::MODE_BEZIER,
		};
		for (int i = 0; i < 10; i++) action[(size_t)i] = defaultTool[(size_t)i];
	}

	void serialDescription(bbe::SerializedDescription &desc)
	{
		desc.describe(formatVersion);
		desc.describe(action);
	}
};

static void applyPersistedDigitHotkeys(PaintEditor &editor, const ExamplePaintDigitHotkeysPersist &p)
{
	if (p.formatVersion != ExamplePaintDigitHotkeysPersist::kFormatVersion) return;
	for (int i = 0; i < 10; i++)
	{
		const auto a = static_cast<PaintEditor::DigitHotkeyAction>(p.action[(size_t)i]);
		if (PaintEditor::digitHotkeyActionIsValid(a)) editor.digitHotkeys[i] = a;
	}
}

static void captureDigitHotkeysToPersist(const PaintEditor &editor, ExamplePaintDigitHotkeysPersist &p)
{
	p.formatVersion = ExamplePaintDigitHotkeysPersist::kFormatVersion;
	for (int i = 0; i < 10; i++) p.action[(size_t)i] = static_cast<int32_t>(editor.digitHotkeys[i]);
}

struct ExamplePaintFavoriteColorsPersist
{
	static constexpr int32_t kFormatVersion = 1;
	int32_t formatVersion = kFormatVersion;
	std::array<std::array<float, 4>, PaintEditor::favoriteColorSlotCount> rgba{};

	ExamplePaintFavoriteColorsPersist()
	{
		formatVersion = kFormatVersion;
		for (auto &slot : rgba)
			slot = { 1.f, 1.f, 1.f, 1.f };
	}

	void serialDescription(bbe::SerializedDescription &desc)
	{
		desc.describe(formatVersion);
		desc.describe(rgba);
	}
};

static void applyPersistedFavoriteColors(PaintEditor &editor, const ExamplePaintFavoriteColorsPersist &p)
{
	if (p.formatVersion != ExamplePaintFavoriteColorsPersist::kFormatVersion)
	{
		for (size_t i = 0; i < PaintEditor::favoriteColorSlotCount; i++)
			for (int j = 0; j < 4; j++) editor.favoriteColorRgba[i][j] = 1.f;
		return;
	}
	for (size_t i = 0; i < PaintEditor::favoriteColorSlotCount; i++)
		for (int j = 0; j < 4; j++) editor.favoriteColorRgba[i][j] = p.rgba[i][j];
}

static void captureFavoriteColorsToPersist(const PaintEditor &editor, ExamplePaintFavoriteColorsPersist &p)
{
	p.formatVersion = ExamplePaintFavoriteColorsPersist::kFormatVersion;
	for (size_t i = 0; i < PaintEditor::favoriteColorSlotCount; i++)
		for (int j = 0; j < 4; j++) p.rgba[i][j] = editor.favoriteColorRgba[i][j];
}

struct ExamplePaintColorHistoryPersist
{
	static constexpr int32_t kFormatVersion = 1;
	int32_t formatVersion = kFormatVersion;
	std::array<std::array<float, 4>, PaintEditor::colorHistoryCapacity> rgba{};

	ExamplePaintColorHistoryPersist()
	{
		formatVersion = kFormatVersion;
		for (auto &slot : rgba)
			slot = { 1.f, 1.f, 1.f, 1.f };
	}

	void serialDescription(bbe::SerializedDescription &desc)
	{
		desc.describe(formatVersion);
		desc.describe(rgba);
	}
};

static void applyPersistedColorHistory(PaintEditor &editor, const ExamplePaintColorHistoryPersist &p)
{
	if (p.formatVersion != ExamplePaintColorHistoryPersist::kFormatVersion)
	{
		for (size_t i = 0; i < PaintEditor::colorHistoryCapacity; i++)
			for (int j = 0; j < 4; j++) editor.colorHistoryRgba[i][j] = 1.f;
		editor.colorHistoryCount = PaintEditor::colorHistoryCapacity;
		return;
	}
	for (size_t i = 0; i < PaintEditor::colorHistoryCapacity; i++)
		for (int j = 0; j < 4; j++) editor.colorHistoryRgba[i][j] = p.rgba[i][j];
	editor.colorHistoryCount = PaintEditor::colorHistoryCapacity;
}

static void captureColorHistoryToPersist(const PaintEditor &editor, ExamplePaintColorHistoryPersist &p)
{
	p.formatVersion = ExamplePaintColorHistoryPersist::kFormatVersion;
	for (size_t i = 0; i < PaintEditor::colorHistoryCapacity; i++)
		for (int j = 0; j < 4; j++) p.rgba[i][j] = editor.colorHistoryRgba[i][j];
}

/// For axis mirror symmetries, text must be flipped (not just moved). Indices match `bbe::getSymmetryPositions` order.
/// `outFlipH` / `outFlipV` mean reflection across a **vertical** / **horizontal** line in canvas space (L-R vs upside down).
/// Note: `bbe::Image::mirrorVertically()` swaps pixels left↔right; `mirrorHorizontally()` swaps rows top↔bottom.
static void textSymmetryMirrorFlags(bbe::SymmetryMode mode, size_t symIndex, bool &outFlipH, bool &outFlipV)
{
	outFlipH = false;
	outFlipV = false;
	switch (mode)
	{
	case bbe::SymmetryMode::Horizontal:
		if (symIndex == 1) outFlipV = true;
		break;
	case bbe::SymmetryMode::Vertical:
		if (symIndex == 1) outFlipH = true;
		break;
	case bbe::SymmetryMode::FourWay:
		if (symIndex == 1) outFlipH = true;
		else if (symIndex == 2) outFlipV = true;
		else if (symIndex == 3)
		{
			outFlipH = true;
			outFlipV = true;
		}
		break;
	default:
		break;
	}
}

static void runPaintEditorUpdate(PaintEditor &editor, bbe::Game &g, float timeSinceLastFrame)
{
	PaintWindowMetrics w{};
	w.width = g.getWindowWidth();
	w.height = g.getWindowHeight();
	w.scale = g.getWindow()->getDpiScale();
	editor.setViewportMetrics(w);

	bbe::Vector2 currMousePos{};
	bool drawMode = false;
	bool shadowDrawMode = false;
	bool drawButtonDown = false;
	bool drawButtonDownForTools = false;

	if (editor.mode == PaintEditor::MODE_PIPETTE && editor.lastModeSnapshot != PaintEditor::MODE_PIPETTE)
	{
		editor.pipetteReturnMode = editor.lastModeSnapshot;
	}

	if (editor.suppressCanvasInputUntilMouseUp && !g.isMouseDown(bbe::MouseButton::LEFT) && !g.isMouseDown(bbe::MouseButton::RIGHT))
	{
		editor.suppressCanvasInputUntilMouseUp = false;
	}

	const bool ctrlDown = g.isKeyDown(bbe::Key::LEFT_CONTROL) || g.isKeyDown(bbe::Key::RIGHT_CONTROL);
	const bool shiftDown = g.isKeyDown(bbe::Key::LEFT_SHIFT) || g.isKeyDown(bbe::Key::RIGHT_SHIFT);
	editor.setSelectionAdditiveModifier(ctrlDown);
	drawButtonDown = g.isMouseDown(bbe::MouseButton::LEFT) || g.isMouseDown(bbe::MouseButton::RIGHT);
	drawButtonDownForTools = drawButtonDown && !editor.suppressCanvasInputUntilMouseUp;
	editor.setConstrainSquare(shiftDown);
	auto discardTransientWorkArea = [&]()
	{
		editor.clearWorkArea();
		editor.brushStrokeChangeRegistered = false;
		editor.resetColorHistoryStrokeTouched();
		editor.eraserStrokeHasPrev = false;
		editor.eraserPreviewSegmentActive = false;
		editor.sprayStrokeHasPrev = false;
	};
	if (editor.mode != editor.lastModeSnapshot && !drawButtonDown)
	{
		if (PaintEditor::isSelectionLikeTool(editor.lastModeSnapshot) && !PaintEditor::isSelectionLikeTool(editor.mode))
		{
			editor.applySelectionWhenLeavingTool();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_RECTANGLE && editor.rectangle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_CIRCLE && editor.circle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_LINE && editor.line.draftActive)
		{
			editor.finalizeLineDraft();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_ARROW && editor.arrow.draftActive)
		{
			editor.finalizeArrowDraft();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_BEZIER && !editor.bezier.controlPoints.isEmpty())
		{
			editor.finalizeBezierDraft();
		}
		discardTransientWorkArea();
	}

	const bbe::Vector2 prevMousePos = editor.screenToCanvas(g.getMousePrevious());
	const int32_t modeBeforeInput = editor.mode;
	bool refreshRectangleDraft = false;
	if (g.isKeyPressed(bbe::Key::SPACE))
	{
		editor.resetCamera();
	}
	if (g.isKeyPressed(bbe::Key::F1) && editor.symmetryMode != bbe::SymmetryMode::None)
	{
		editor.symmetryOffsetCustom = true;
		editor.symmetryOffset = editor.screenToCanvas(g.getMouse());
	}

	constexpr float CAM_WASD_SPEED = 400;
	if (!ctrlDown && g.isKeyDown(bbe::Key::W))
	{
		editor.offset.y += timeSinceLastFrame * CAM_WASD_SPEED;
	}
	if (!ctrlDown && g.isKeyDown(bbe::Key::S))
	{
		editor.offset.y -= timeSinceLastFrame * CAM_WASD_SPEED;
	}
	if (!ctrlDown && g.isKeyDown(bbe::Key::A))
	{
		editor.offset.x += timeSinceLastFrame * CAM_WASD_SPEED;
	}
	if (!ctrlDown && g.isKeyDown(bbe::Key::D))
	{
		editor.offset.x -= timeSinceLastFrame * CAM_WASD_SPEED;
	}

	if (g.isMouseDown(bbe::MouseButton::MIDDLE))
	{
		editor.offset += g.getMouseDelta();
		if (editor.tiled)
		{
			if (editor.offset.x < 0) editor.offset.x += editor.getCanvasWidth() * editor.zoomLevel;
			if (editor.offset.y < 0) editor.offset.y += editor.getCanvasHeight() * editor.zoomLevel;
			if (editor.offset.x > editor.getCanvasWidth() * editor.zoomLevel) editor.offset.x -= editor.getCanvasWidth() * editor.zoomLevel;
			if (editor.offset.y > editor.getCanvasHeight() * editor.zoomLevel) editor.offset.y -= editor.getCanvasHeight() * editor.zoomLevel;
		}
	}

	if (g.getMouseScrollY() < 0)
	{
		editor.changeZoom(1.0f / 1.1f, g.getMouse());
	}
	else if (g.getMouseScrollY() > 0)
	{
		editor.changeZoom(1.1f, g.getMouse());
	}
	currMousePos = editor.screenToCanvas(g.getMouse());

	if (!ctrlDown)
	{
		const bbe::Key digitKeys[10] = {
			bbe::Key::_1, bbe::Key::_2, bbe::Key::_3, bbe::Key::_4, bbe::Key::_5,
			bbe::Key::_6, bbe::Key::_7, bbe::Key::_8, bbe::Key::_9, bbe::Key::_0,
		};
		for (int di = 0; di < 10; di++)
		{
			if (g.isKeyPressed(digitKeys[di])) editor.applyDigitHotkeyBinding(editor.digitHotkeys[di]);
		}
	}
	if (g.isKeyPressed(bbe::Key::M))
	{
		editor.mode = PaintEditor::MODE_MAGIC_WAND;
	}
	if (g.isKeyPressed(bbe::Key::L))
	{
		editor.mode = PaintEditor::MODE_LASSO;
	}
	if (g.isKeyPressed(bbe::Key::P))
	{
		editor.mode = PaintEditor::MODE_POLYGON_LASSO;
	}
	if (g.isKeyPressed(bbe::Key::O))
	{
		editor.mode = PaintEditor::MODE_ELLIPSE_SELECTION;
	}
	if (g.isKeyPressed(bbe::Key::E))
	{
		editor.mode = PaintEditor::MODE_ERASER;
	}
	if (g.isKeyPressed(bbe::Key::R))
	{
		editor.mode = PaintEditor::MODE_SPRAY;
	}
	bool refreshCircleDraft = false;
	if (!ctrlDown && g.isKeyPressed(bbe::Key::X))
	{
		editor.swapColors();
		refreshRectangleDraft = editor.rectangle.draftActive;
		refreshCircleDraft = editor.circle.draftActive;
	}
	const bool increaseToolSize = g.isKeyTyped(bbe::Key::EQUAL) || g.isKeyTyped(bbe::Key::KP_ADD) || g.isKeyTyped(bbe::Key::RIGHT_BRACKET);
	const bool decreaseToolSize = g.isKeyTyped(bbe::Key::MINUS) || g.isKeyTyped(bbe::Key::KP_SUBTRACT);
	if (editor.mode == PaintEditor::MODE_ERASER)
	{
		if (increaseToolSize) editor.eraserSize++;
		if (decreaseToolSize) editor.eraserSize--;
		editor.clampEraserSize();
	}
	else if (editor.mode == PaintEditor::MODE_SPRAY)
	{
		if (increaseToolSize) editor.sprayWidth++;
		if (decreaseToolSize) editor.sprayWidth--;
		editor.clampSprayWidth();
	}
	else if (editor.mode == PaintEditor::MODE_BRUSH || editor.mode == PaintEditor::MODE_LINE || editor.mode == PaintEditor::MODE_RECTANGLE || editor.mode == PaintEditor::MODE_CIRCLE || editor.mode == PaintEditor::MODE_ARROW || editor.mode == PaintEditor::MODE_BEZIER)
	{
		const int32_t previousBrushWidth = editor.brushWidth;
		if (increaseToolSize) editor.brushWidth++;
		if (decreaseToolSize) editor.brushWidth--;
		editor.clampBrushWidth();
		if (editor.rectangle.draftActive && editor.brushWidth != previousBrushWidth)
		{
			refreshRectangleDraft = true;
		}
		if (editor.circle.draftActive && editor.brushWidth != previousBrushWidth)
		{
			refreshCircleDraft = true;
		}
	}
	else if (editor.mode == PaintEditor::MODE_TEXT)
	{
		if (increaseToolSize) editor.textFontSize++;
		if (decreaseToolSize) editor.textFontSize--;
		editor.clampTextFontSize();
	}
	else if (editor.mode == PaintEditor::MODE_MAGIC_WAND)
	{
		if (increaseToolSize) editor.magicWandTolerance += 4;
		if (decreaseToolSize) editor.magicWandTolerance -= 4;
		editor.clampMagicWandTolerance();
	}
	else if (editor.mode == PaintEditor::MODE_FLOOD_FILL)
	{
		if (increaseToolSize) editor.floodFillTolerance += 4;
		if (decreaseToolSize) editor.floodFillTolerance -= 4;
		editor.clampFloodFillTolerance();
	}

	if (ctrlDown)
	{
		if (g.isKeyTyped(bbe::Key::Z) && editor.canvas.isUndoable())
		{
			editor.undo();
		}
		if (g.isKeyTyped(bbe::Key::Y) && editor.canvas.isRedoable())
		{
			editor.redo();
		}
		if (g.isKeyPressed(bbe::Key::S))
		{
			editor.saveCanvas();
		}
		if (g.isKeyPressed(bbe::Key::D))
		{
			editor.resetColorsToDefault();
			refreshRectangleDraft = editor.rectangle.draftActive;
			refreshCircleDraft = editor.circle.draftActive;
		}
		if (g.isKeyPressed(bbe::Key::A))
		{
			editor.selectWholeLayer();
		}
		if (g.isKeyPressed(bbe::Key::C))
		{
			if (!PaintEditor::isSelectionLikeTool(editor.mode))
			{
				editor.mode = PaintEditor::MODE_SELECTION;
			}
			else
			{
				editor.storeSelectionInClipboard();
			}
		}
		if (g.isKeyPressed(bbe::Key::X))
		{
			editor.cutSelection();
		}
		if (g.isKeyPressed(bbe::Key::V))
		{
			editor.pasteSelectionAt(editor.toCanvasPixel(currMousePos));
		}
	}
	if (editor.mode != modeBeforeInput && !drawButtonDown)
	{
		if (PaintEditor::isSelectionLikeTool(modeBeforeInput) && !PaintEditor::isSelectionLikeTool(editor.mode))
		{
			editor.applySelectionWhenLeavingTool();
		}
		if (modeBeforeInput == PaintEditor::MODE_RECTANGLE && editor.rectangle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (modeBeforeInput == PaintEditor::MODE_CIRCLE && editor.circle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (modeBeforeInput == PaintEditor::MODE_LINE && editor.line.draftActive)
		{
			editor.finalizeLineDraft();
		}
		if (modeBeforeInput == PaintEditor::MODE_ARROW && editor.arrow.draftActive)
		{
			editor.finalizeArrowDraft();
		}
		if (modeBeforeInput == PaintEditor::MODE_BEZIER && !editor.bezier.controlPoints.isEmpty())
		{
			editor.finalizeBezierDraft();
		}
		discardTransientWorkArea();
	}
	editor.lastModeSnapshot = editor.mode;
	if (refreshRectangleDraft)
	{
		editor.refreshActiveRectangleDraftImage();
	}
	if (refreshCircleDraft)
	{
		editor.refreshActiveCircleDraftImage();
	}
	if (editor.selection.floating && !PaintEditor::isSelectionLikeTool(editor.mode) && !(editor.mode == PaintEditor::MODE_RECTANGLE && editor.rectangle.draftActive) && !(editor.mode == PaintEditor::MODE_CIRCLE && editor.circle.draftActive))
	{
		editor.commitFloatingSelection();
	}
	if (g.isKeyPressed(bbe::Key::ESCAPE))
	{
		editor.deselectAll();
	}
	if (editor.mode == PaintEditor::MODE_POLYGON_LASSO && (g.isKeyPressed(bbe::Key::ENTER) || g.isKeyPressed(bbe::Key::KP_ENTER)))
	{
		editor.confirmPolygonLassoIfReady();
	}
	if (g.isKeyPressed(bbe::Key::DELETE) || g.isKeyPressed(bbe::Key::BACKSPACE))
	{
		if (editor.mode == PaintEditor::MODE_POLYGON_LASSO && !editor.selection.polygonLassoVertices.empty())
		{
			editor.polygonLassoBackspace();
		}
		else
		{
			editor.deleteSelection();
		}
	}

	const bool mouseOnNavigator = editor.showNavigator && editor.getCanvasWidth() > 0 && editor.getNavigatorRect().isPointInRectangle(g.getMouse());
	if (!mouseOnNavigator && !editor.canvasResizeActive)
	{
		if (g.isKeyTyped(bbe::Key::LEFT)) editor.nudgeSelectionByPixels(-1, 0);
		if (g.isKeyTyped(bbe::Key::RIGHT)) editor.nudgeSelectionByPixels(1, 0);
		if (g.isKeyTyped(bbe::Key::UP)) editor.nudgeSelectionByPixels(0, -1);
		if (g.isKeyTyped(bbe::Key::DOWN)) editor.nudgeSelectionByPixels(0, 1);
	}


	if (!mouseOnNavigator && (g.isMousePressed(bbe::MouseButton::LEFT) || g.isMousePressed(bbe::MouseButton::RIGHT)))
	{
		editor.startMousePos = editor.screenToCanvas(g.getMouse());
	}

	if (!mouseOnNavigator && !editor.canvasResizeActive && g.isMousePressed(bbe::MouseButton::LEFT))
	{
		const int32_t hitHandle = editor.getCanvasResizeHitHandle(g.getMouse());
		if (hitHandle >= 0)
		{
			editor.canvasResizeActive = true;
			editor.canvasResizeHandleIndex = hitHandle;
			editor.canvasResizePreviewRect = { 0, 0, editor.getCanvasWidth(), editor.getCanvasHeight() };
			editor.updateCanvasResizePreview(currMousePos);
		}
	}
	if (editor.canvasResizeActive)
	{
		if (g.isMouseDown(bbe::MouseButton::LEFT))
		{
			editor.updateCanvasResizePreview(currMousePos);
		}
		if (g.isMouseReleased(bbe::MouseButton::LEFT))
		{
			editor.applyCanvasResize(editor.canvasResizePreviewRect);
			editor.canvasResizeActive = false;
			editor.canvasResizeHandleIndex = -1;
			editor.canvasResizePreviewRect = {};
		}
	}

	if (!mouseOnNavigator && !editor.canvasResizeActive)
	{
		// Pointer events are translated into editor commands.
		if (g.isMousePressed(bbe::MouseButton::LEFT)) editor.pointerDown(PaintEditor::PointerButton::Primary, currMousePos);
		if (g.isMousePressed(bbe::MouseButton::RIGHT)) editor.pointerDown(PaintEditor::PointerButton::Secondary, currMousePos);
		if (g.isMouseReleased(bbe::MouseButton::LEFT)) editor.pointerUp(PaintEditor::PointerButton::Primary, currMousePos);
		if (g.isMouseReleased(bbe::MouseButton::RIGHT)) editor.pointerUp(PaintEditor::PointerButton::Secondary, currMousePos);

		// Drive previews / drags from the controller (no editor update loop).
		const bool needsPointerMove =
			editor.pointerPrimaryDown || editor.pointerSecondaryDown ||
			editor.selection.dragActive || editor.selection.lassoDragActive || editor.selection.moveActive || editor.selection.resizeActive || editor.selection.rotationHandleActive ||
			(editor.mode == PaintEditor::MODE_POLYGON_LASSO && !editor.selection.polygonLassoVertices.empty()) ||
			editor.rectangle.dragActive || editor.circle.dragActive ||
			editor.line.dragInProgress || editor.line.draftActive ||
			editor.arrow.dragInProgress || editor.arrow.draftActive ||
			!editor.bezier.controlPoints.isEmpty();
		if (needsPointerMove)
		{
			editor.pointerMove(currMousePos);
		}

		if (editor.mode == PaintEditor::MODE_BEZIER && g.isKeyPressed(bbe::Key::BACKSPACE))
		{
			editor.bezierBackspace();
		}
	}
	if (!mouseOnNavigator && !editor.canvasResizeActive && editor.mode == PaintEditor::MODE_TEXT && (g.isMousePressed(bbe::MouseButton::LEFT) || g.isMousePressed(bbe::MouseButton::RIGHT)))
	{
		bbe::Vector2 pos = currMousePos;
		if (editor.toTiledPos(pos))
		{
			bool textChanged = false;
			const bbe::Vector2i originalTopLeft = editor.toCanvasPixel(pos);
			const auto symPositions = editor.getSymmetryPositions(pos);
			const auto symAngles = editor.getSymmetryRotationAngles();
			const bbe::Colori textColor = editor.activeDrawColor(g.isMouseDown(bbe::MouseButton::LEFT), g.isMouseDown(bbe::MouseButton::RIGHT));

			const bbe::Image textImg = editor.renderTextToImage(originalTopLeft, textColor);
			const bbe::Vector2 imgCenter = {
				originalTopLeft.x + textImg.getWidth() * 0.5f,
				originalTopLeft.y + textImg.getHeight() * 0.5f
			};
			const auto imgCenterSymPositions = editor.getSymmetryPositions(imgCenter);

			for (size_t i = 0; i < symPositions.getLength(); i++)
			{
				// Do not skip when symPos is outside the canvas: radial/mirror copies are centered on
				// imgCenterSymPositions[i] and may still overlap the layer; blendOverRotated / blendOver clip per pixel.

				if (std::abs(symAngles[i]) > 0.0001f)
				{
					if (textImg.getWidth() > 0 && textImg.getHeight() > 0)
					{
						const bbe::Rectanglei symRect = {
							(int32_t)std::round(imgCenterSymPositions[i].x - textImg.getWidth() * 0.5f),
							(int32_t)std::round(imgCenterSymPositions[i].y - textImg.getHeight() * 0.5f),
							textImg.getWidth(),
							textImg.getHeight()
						};
						editor.getActiveLayerImage().blendOverRotated(textImg, symRect, symAngles[i], editor.tiled, editor.antiAliasingEnabled);
						textChanged = true;
					}
				}
				else
				{
					bool flipH = false;
					bool flipV = false;
					textSymmetryMirrorFlags(editor.symmetryMode, i, flipH, flipV);
					if (!flipH && !flipV)
					{
						const bbe::Vector2i symTopLeft = editor.toCanvasPixel(symPositions[i]);
						textChanged |= editor.drawTextAt(symTopLeft, textColor);
					}
					else if (textImg.getWidth() > 0 && textImg.getHeight() > 0)
					{
						bbe::Image symImg = textImg;
						if (flipH) symImg.mirrorVertically();
						if (flipV) symImg.mirrorHorizontally();
						const bbe::Vector2i dstTopLeft = {
							(int32_t)std::round(imgCenterSymPositions[i].x - symImg.getWidth() * 0.5f),
							(int32_t)std::round(imgCenterSymPositions[i].y - symImg.getHeight() * 0.5f)
						};
						editor.getActiveLayerImage().blendOver(symImg, dstTopLeft, editor.tiled);
						textChanged = true;
					}
				}
			}
			if (textChanged)
			{
				editor.submitCanvas();
				editor.recordColorHistoryFromColori(textColor);
			}
		}
	}

	if (!mouseOnNavigator && !editor.canvasResizeActive && editor.mode == PaintEditor::MODE_MAGIC_WAND &&
		(g.isMousePressed(bbe::MouseButton::LEFT) || g.isMousePressed(bbe::MouseButton::RIGHT)) && !editor.suppressCanvasInputUntilMouseUp)
	{
		const bool selectionTransformClick = editor.selection.moveActive || editor.selection.resizeActive || editor.selection.rotationHandleActive;
		if (!selectionTransformClick && !editor.consumeMagicWandSuppressedPick())
		{
			bbe::Vector2 pos = editor.screenToCanvas(g.getMouse());
			if (editor.toTiledPos(pos))
			{
				editor.applyMagicWandAt(editor.toCanvasPixel(pos), ctrlDown);
			}
		}
	}

	const bbe::List<decltype(editor.mode)> shadowDrawModes = { PaintEditor::MODE_BRUSH, PaintEditor::MODE_ERASER, PaintEditor::MODE_SPRAY };
	drawMode = editor.mode != PaintEditor::MODE_SELECTION && editor.mode != PaintEditor::MODE_ELLIPSE_SELECTION && editor.mode != PaintEditor::MODE_MAGIC_WAND && editor.mode != PaintEditor::MODE_LASSO && editor.mode != PaintEditor::MODE_POLYGON_LASSO && editor.mode != PaintEditor::MODE_TEXT && editor.mode != PaintEditor::MODE_RECTANGLE && editor.mode != PaintEditor::MODE_CIRCLE && editor.mode != PaintEditor::MODE_LINE && editor.mode != PaintEditor::MODE_ARROW && editor.mode != PaintEditor::MODE_BEZIER && !editor.canvasResizeActive && !mouseOnNavigator && drawButtonDownForTools;
	shadowDrawMode = shadowDrawModes.contains(editor.mode);

	if (editor.brushStrokeChangeRegistered)
	{
		if (g.isMouseReleased(bbe::MouseButton::LEFT) || g.isMouseReleased(bbe::MouseButton::RIGHT))
		{
			if (!g.isMouseDown(bbe::MouseButton::LEFT) && !g.isMouseDown(bbe::MouseButton::RIGHT))
			{
				if (editor.mode == PaintEditor::MODE_ERASER) editor.applyEraserWorkArea();
				else editor.applyWorkArea();
				editor.submitCanvas();
				if (editor.mode != PaintEditor::MODE_ERASER)
				{
					if (editor.colorHistoryStrokeTouchedPrimary) editor.recordColorHistory(editor.leftColor);
					if (editor.colorHistoryStrokeTouchedSecondary) editor.recordColorHistory(editor.rightColor);
				}
				editor.resetColorHistoryStrokeTouched();
				editor.brushStrokeChangeRegistered = false;
			}
		}
	}

	if (drawMode || shadowDrawMode)
	{
		if (!drawMode)
		{
			editor.brushStrokeUpdateShadowCounter++;
			if (editor.brushStrokeUpdateShadowCounter > 1) editor.clearWorkArea();
		}
		else
		{
			if (editor.brushStrokeUpdateShadowCounter > 0) editor.clearWorkArea();
			editor.brushStrokeUpdateShadowCounter = 0;
		}

		if (editor.mode == PaintEditor::MODE_BRUSH)
		{
			const bool leftDown = g.isMouseDown(bbe::MouseButton::LEFT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool rightDown = g.isMouseDown(bbe::MouseButton::RIGHT) && !editor.suppressCanvasInputUntilMouseUp;

			if (!editor.brushStrokeUpdateLastDrawButtonDown && drawButtonDown)
			{
				editor.brushStrokeUpdateRecentPointCount = 0;
			}

			// Maintain up to 4 recent mouse positions (index 0 = newest).
			auto pushBrushPoint = [&](const bbe::Vector2 &p)
			{
				if (editor.brushStrokeUpdateRecentPointCount > 0 && (editor.brushStrokeUpdateRecentPoints[0] - p).getLength() < 0.01f) return;
				const int32_t maxIndex = bbe::Math::min<int32_t>(editor.brushStrokeUpdateRecentPointCount, 3);
				for (int32_t i = maxIndex; i > 0; --i)
				{
					editor.brushStrokeUpdateRecentPoints[i] = editor.brushStrokeUpdateRecentPoints[i - 1];
				}
				editor.brushStrokeUpdateRecentPoints[0] = p;
				if (editor.brushStrokeUpdateRecentPointCount < 4) editor.brushStrokeUpdateRecentPointCount++;
			};

			pushBrushPoint(currMousePos);

			bool touched = false;
			if (editor.brushStrokeUpdateRecentPointCount == 1)
			{
				touched = editor.touch(editor.brushStrokeUpdateRecentPoints[0], false, leftDown, rightDown);
			}
			else if (editor.brushStrokeUpdateRecentPointCount >= 4)
			{
				// Use Catmull-Rom -> Bezier conversion for the middle segment.
				// Points: [0]=newest, [3]=älteste.
				const bbe::Vector2 &p0 = editor.brushStrokeUpdateRecentPoints[3];
				const bbe::Vector2 &p1 = editor.brushStrokeUpdateRecentPoints[2];
				const bbe::Vector2 &p2 = editor.brushStrokeUpdateRecentPoints[1];
				const bbe::Vector2 &p3 = editor.brushStrokeUpdateRecentPoints[0];

				const bbe::Vector2 c1 = p1 + (p2 - p0) / 6.0f;
				const bbe::Vector2 c2 = p2 - (p3 - p1) / 6.0f;

				bbe::List<bbe::Vector2> splinePoints;
				splinePoints.add(p1);  // start on curve
				splinePoints.add(c1);  // control 1 (beeinflusst von p0)
				splinePoints.add(c2);  // control 2 (beeinflusst von p3)
				splinePoints.add(p2);  // end on curve

				editor.drawBezierSymmetry(splinePoints, editor.activeDrawColor(leftDown, rightDown));
				touched = true;
				editor.bumpNavigatorThumbnailDirty();
			}
			else if (editor.brushStrokeUpdateRecentPointCount >= 2)
			{
				touched = editor.touchLine(editor.brushStrokeUpdateRecentPoints[0], editor.brushStrokeUpdateRecentPoints[1], false, leftDown, rightDown);
			}
			if (drawMode)
			{
				editor.brushStrokeChangeRegistered |= touched;
				if (touched)
				{
					editor.bumpNavigatorThumbnailDirty();
					if (leftDown) editor.colorHistoryStrokeTouchedPrimary = true;
					if (rightDown && !leftDown) editor.colorHistoryStrokeTouchedSecondary = true;
				}
			}
		}
		else if (editor.mode == PaintEditor::MODE_ERASER)
		{
			const bool leftDown = g.isMouseDown(bbe::MouseButton::LEFT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool rightDown = g.isMouseDown(bbe::MouseButton::RIGHT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool eraseActive = leftDown || rightDown;

			if (!editor.brushStrokeUpdateLastDrawButtonDown && drawButtonDown)
			{
				editor.eraserStrokeHasPrev = false;
			}

			if (!eraseActive)
			{
				editor.eraserPreviewSegmentActive = false;
			}

			bool touched = false;
			if (eraseActive)
			{
				const bbe::Vector2 from = editor.eraserStrokePrevCanvasPos;
				if (editor.eraserStrokeHasPrev)
				{
					const float segLen = (currMousePos - from).getLength();
					if (segLen < 1e-5f)
					{
						touched = false;
						editor.eraserPreviewSegmentActive = false;
					}
					else
					{
						touched = editor.eraseLineOnWorkAreaWithSymmetry(currMousePos, from);
						editor.eraserPreviewSegmentPNew = currMousePos;
						editor.eraserPreviewSegmentPOld = from;
						editor.eraserPreviewSegmentActive = true;
					}
				}
				else
				{
					touched = editor.eraseStampAtCanvasWithSymmetry(currMousePos);
					editor.eraserPreviewSegmentActive = false;
				}
				editor.eraserStrokePrevCanvasPos = currMousePos;
				editor.eraserStrokeHasPrev = true;
			}
			if (drawMode)
			{
				editor.brushStrokeChangeRegistered |= touched;
				if (touched)
					editor.bumpNavigatorThumbnailDirty();
			}
		}
		else if (editor.mode == PaintEditor::MODE_SPRAY)
		{
			const bool leftDown = g.isMouseDown(bbe::MouseButton::LEFT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool rightDown = g.isMouseDown(bbe::MouseButton::RIGHT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool sprayActive = leftDown || rightDown;

			if (!editor.brushStrokeUpdateLastDrawButtonDown && drawButtonDown)
			{
				editor.sprayStrokeHasPrev = false;
			}

			bool touched = false;
			if (sprayActive)
			{
				if (editor.sprayStrokeHasPrev)
				{
					touched = editor.sprayLineOnWorkAreaWithSymmetry(currMousePos, editor.sprayStrokePrevCanvasPos, leftDown, rightDown);
				}
				else
				{
					touched = editor.sprayBurstAtCanvasWithSymmetry(currMousePos, leftDown, rightDown);
				}
				editor.sprayStrokePrevCanvasPos = currMousePos;
				editor.sprayStrokeHasPrev = true;
			}
			if (drawMode)
			{
				editor.brushStrokeChangeRegistered |= touched;
				if (touched)
				{
					editor.bumpNavigatorThumbnailDirty();
					if (leftDown) editor.colorHistoryStrokeTouchedPrimary = true;
					if (rightDown && !leftDown) editor.colorHistoryStrokeTouchedSecondary = true;
				}
			}
		}
		else if (editor.mode == PaintEditor::MODE_FLOOD_FILL)
		{
			const bool leftDown = g.isMouseDown(bbe::MouseButton::LEFT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool rightDown = g.isMouseDown(bbe::MouseButton::RIGHT) && !editor.suppressCanvasInputUntilMouseUp;
			bbe::Vector2 pos = editor.screenToCanvas(g.getMouse());
			if (editor.toTiledPos(pos))
			{
				const auto symPositions = editor.getSymmetryPositions(pos);
				for (size_t i = 0; i < symPositions.getLength(); i++)
				{
					bbe::Vector2 symPos = symPositions[i];
					if (editor.toTiledPos(symPos))
					{
						const int overflowTol = editor.floodFillSmartFill ? 255 : 0;
						const int overflowDepth = editor.floodFillSmartFill ? 1 : 0;
						editor.getActiveLayerImage().floodFill(symPos.as<int32_t>(),
							editor.activeDrawColor(leftDown, rightDown),
							false,
							editor.tiled,
							editor.floodFillTolerance,
							overflowTol,
							overflowDepth);
					}
				}
				editor.brushStrokeChangeRegistered = true;
				editor.bumpNavigatorThumbnailDirty();
				if (drawMode)
				{
					if (leftDown) editor.colorHistoryStrokeTouchedPrimary = true;
					if (rightDown && !leftDown) editor.colorHistoryStrokeTouchedSecondary = true;
				}
			}
		}
		else if (editor.mode == PaintEditor::MODE_PIPETTE)
		{
			auto pos = editor.screenToCanvas(g.getMouse());
			if (editor.toTiledPos(pos))
			{
				const size_t x = (size_t)pos.x;
				const size_t y = (size_t)pos.y;
				const bbe::Colori color = editor.getVisiblePixel(x, y);
				if (g.isMousePressed(bbe::MouseButton::LEFT))
				{
					if (editor.canvas.get().paletteMode) editor.applyPipetteSampleToPaletteSelection(color, false);
					else
					{
						editor.leftColor[0] = color.r / 255.f;
						editor.leftColor[1] = color.g / 255.f;
						editor.leftColor[2] = color.b / 255.f;
						editor.leftColor[3] = color.a / 255.f;
					}
					editor.mode = editor.pipetteReturnMode;
					editor.suppressCanvasInputUntilMouseUp = true;
					editor.pointerPrimaryDown = false;
					editor.pointerSecondaryDown = false;
					editor.brushStrokeUpdateRecentPointCount = 0;
				}
				if (g.isMousePressed(bbe::MouseButton::RIGHT))
				{
					if (editor.canvas.get().paletteMode) editor.applyPipetteSampleToPaletteSelection(color, true);
					else
					{
						editor.rightColor[0] = color.r / 255.f;
						editor.rightColor[1] = color.g / 255.f;
						editor.rightColor[2] = color.b / 255.f;
						editor.rightColor[3] = color.a / 255.f;
					}
					editor.mode = editor.pipetteReturnMode;
					editor.suppressCanvasInputUntilMouseUp = true;
					editor.pointerPrimaryDown = false;
					editor.pointerSecondaryDown = false;
					editor.brushStrokeUpdateRecentPointCount = 0;
				}
			}
		}
		else
		{
			bbe::Crash(bbe::Error::IllegalState);
		}
	}

	if (!drawButtonDown && editor.brushStrokeUpdateLastDrawButtonDown)
	{
		editor.brushStrokeUpdateRecentPointCount = 0;
		editor.eraserStrokeHasPrev = false;
		editor.eraserPreviewSegmentActive = false;
		editor.sprayStrokeHasPrev = false;
	}
	editor.brushStrokeUpdateLastDrawButtonDown = drawButtonDown;

	if (editor.canvas.get().paletteMode && editor.workArea.getWidth() > 0 && editor.workArea.getHeight() > 0 && editor.workArea.isLoadedCpu())
	{
		editor.quantizeWorkAreaPreviewIfPaletteMode();
	}
}

class MyGame : public bbe::Game
{
public:
	bbe::SerializableObject<ExamplePaintDigitHotkeysPersist> digitHotkeysPersist{ "ExamplePaintDigitHotkeys.dat" };
	bbe::SerializableObject<ExamplePaintFavoriteColorsPersist> favoriteColorsPersist{ "ExamplePaintFavoriteColors.dat" };
	bbe::SerializableObject<ExamplePaintColorHistoryPersist> colorHistoryPersist{ "ExamplePaintColorHistory.dat" };
	PaintEditor editor;
	/// If non-empty, opened on startup (CLI: first argument after the program name).
	std::string initialDocumentPath;

	void onStart() override
	{
		// Platform integration is configured by the host example, not by the editor core.
		PaintEditor::PlatformCallbacks callbacks;
		callbacks.showOpenDialog = [](bbe::String &inOutPath)
		{
			return bbe::simpleFile::showOpenDialog(inOutPath);
		};
		callbacks.showSaveDialog = [](bbe::String &inOutPath, const bbe::String &defaultExtension)
		{
			return bbe::simpleFile::showSaveDialog(inOutPath, defaultExtension);
		};
		callbacks.readBinaryFile = [](const bbe::String &path)
		{
			return bbe::simpleFile::readBinaryFile(path);
		};
		callbacks.writeBinaryFile = [](const bbe::String &path, const bbe::ByteBuffer &buffer)
		{
			return bbe::simpleFile::writeBinaryToFile(path, buffer);
		};
		callbacks.loadImageFile = [](const bbe::String &path)
		{
			return bbe::Image(path.getRaw());
		};
		callbacks.saveImageFile = [](const bbe::String &path, const bbe::Image &image)
		{
			return image.writeToFile(path);
		};
		callbacks.supportsClipboardImages = []() { return bbe::Image::supportsClipboardImages(); };
		callbacks.isClipboardImageAvailable = []() { return bbe::Image::isImageInClipbaord(); };
		callbacks.getClipboardImage = []() { return bbe::Image::getClipboardImage(); };
		callbacks.setClipboardImage = [](const bbe::Image &image)
		{
			image.copyToClipboard();
			return true;
		};
		callbacks.findSystemFonts = [](const bbe::String &purpose)
		{
			const bbe::List<bbe::FontFileEntry> fonts = bbe::Font::findUsableSystemFonts(purpose);
			bbe::List<FontEntry> out;
			for (size_t i = 0; i < fonts.getLength(); i++)
			{
				out.add({ fonts[i].displayName, fonts[i].path });
			}
			return out;
		};
		callbacks.getFont = []([[maybe_unused]] const bbe::String &path, [[maybe_unused]] int32_t size) -> const bbe::Font *
		{
			using FontKey = std::pair<std::string, int32_t>;
			static std::map<FontKey, bbe::Font> cache;

			const int32_t clampedSize = bbe::Math::max<int32_t>(size, 1);
			const FontKey key = { std::string(path.getRaw()), clampedSize };
			auto it = cache.find(key);
			if (it == cache.end())
			{
				it = cache.emplace(key, bbe::Font(path, (unsigned)clampedSize)).first;
			}
			return &it->second;
		};
		editor.setPlatformCallbacks(std::move(callbacks));

#ifndef NDEBUG
		if (!PaintEditor::debugRunPalettePersistenceRegressionChecks())
		{
			bbe::Crash(bbe::Error::IllegalState, "ExamplePaint palette regression checks failed");
		}
#endif

		applyPersistedDigitHotkeys(editor, *digitHotkeysPersist.operator->());
		editor.onDigitHotkeysChanged = [this]()
		{
			captureDigitHotkeysToPersist(editor, *digitHotkeysPersist.operator->());
			digitHotkeysPersist.writeToFile();
		};

		applyPersistedFavoriteColors(editor, *favoriteColorsPersist.operator->());
		editor.onFavoriteColorsChanged = [this]()
		{
			captureFavoriteColorsToPersist(editor, *favoriteColorsPersist.operator->());
			favoriteColorsPersist.writeToFile();
		};

		applyPersistedColorHistory(editor, *colorHistoryPersist.operator->());
		editor.onColorHistoryChanged = [this]()
		{
			captureColorHistoryToPersist(editor, *colorHistoryPersist.operator->());
			colorHistoryPersist.writeToFile();
		};

		PaintWindowMetrics w{};
		w.width = getWindowWidth();
		w.height = getWindowHeight();
		w.scale = getWindow()->getDpiScale();
		editor.onStart(w);

		if (!initialDocumentPath.empty())
		{
			if (!editor.tryOpenDocumentFromPath(initialDocumentPath.c_str()))
			{
				std::fprintf(stderr, "ExamplePaint: could not open \"%s\"\n", initialDocumentPath.c_str());
			}
		}

		editor.applicationExitRequested = [this]()
		{
			bbe::Window *w = getWindow();
			if (w != nullptr)
			{
				w->close();
			}
		};
	}

	bool onWindowCloseRequest() override
	{
		if (editor.allowWindowCloseWithoutPromptOnce)
		{
			editor.allowWindowCloseWithoutPromptOnce = false;
			return true;
		}
		if (!editor.hasUnsavedChanges())
		{
			return true;
		}
		editor.requestReplaceApplicationQuit();
		return false;
	}

	void onFilesDropped(const bbe::List<bbe::String> &paths) override
	{
		editor.onFilesDropped(paths);
	}

	void update(float timeSinceLastFrame) override
	{
		runPaintEditorUpdate(editor, *this, timeSinceLastFrame);
	}

	void draw3D(bbe::PrimitiveBrush3D & /*brush*/) override
	{
	}

	void draw2D(bbe::PrimitiveBrush2D &brush) override
	{
		drawExamplePaintGui(editor, brush, getMouse());
	}

	void onEnd() override
	{
		captureDigitHotkeysToPersist(editor, *digitHotkeysPersist.operator->());
		digitHotkeysPersist.writeToFile();
		editor.onDigitHotkeysChanged = nullptr;
		captureFavoriteColorsToPersist(editor, *favoriteColorsPersist.operator->());
		favoriteColorsPersist.writeToFile();
		editor.onFavoriteColorsChanged = nullptr;
		captureColorHistoryToPersist(editor, *colorHistoryPersist.operator->());
		colorHistoryPersist.writeToFile();
		editor.onColorHistoryChanged = nullptr;
		editor.applicationExitRequested = nullptr;
	}
};

int main(int argc, char **argv)
{
	MyGame *mg = new MyGame();
	if (argc >= 2)
	{
		mg->initialDocumentPath = argv[1];
	}
	mg->setMsaaSamples(0);
	mg->start(1280, 720, "ExamplePaint");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
