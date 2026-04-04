#pragma once

#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.
#include "BBE/Editor/RectSelectionGizmo2D.h"
#include "BBE/Symmetry2D.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <vector>

#include "ExamplePaintInput.h"

struct FontEntry
{
	bbe::String displayName;
	bbe::String path;
};

struct PaintLayer
{
	bbe::String name = "";
	bool visible = true;
	float opacity = 1.0f;
	bbe::BlendMode blendMode = bbe::BlendMode::Normal;
	bbe::Image image;
};

struct PaintDocument
{
	/// RGBA behind all layers (premultiplied-style source for the first blend). Default opaque white; alpha 0 = transparent document.
	float canvasFallbackRgba[4] = { 1.f, 1.f, 1.f, 1.f };
	bbe::List<PaintLayer> layers;
};

struct PaintEditor
{
	enum class PointerButton
	{
		Primary,
		Secondary,
		Middle,
	};

	PaintWindowMetrics viewport{};
	void setViewportMetrics(const PaintWindowMetrics &w) { viewport = w; }

	struct PlatformCallbacks
	{
		// File dialogs / IO
		std::function<bool(bbe::String &inOutPath)> showOpenDialog;
		std::function<bool(bbe::String &inOutPath, const bbe::String &defaultExtension)> showSaveDialog;
		std::function<bbe::ByteBuffer(const bbe::String &path)> readBinaryFile;
		std::function<bool(const bbe::String &path, const bbe::ByteBuffer &buffer)> writeBinaryFile;
		std::function<bbe::Image(const bbe::String &path)> loadImageFile;
		std::function<bool(const bbe::String &path, const bbe::Image &image)> saveImageFile;

		// Clipboard images
		std::function<bool()> supportsClipboardImages;
		std::function<bool()> isClipboardImageAvailable;
		std::function<bbe::Image()> getClipboardImage;
		std::function<bool(const bbe::Image &image)> setClipboardImage;

		// Fonts (system enumeration)
		std::function<bbe::List<FontEntry>(const bbe::String &purpose)> findSystemFonts;
		std::function<const bbe::Font *(const bbe::String &path, int32_t size)> getFont;
	};

	void setPlatformCallbacks(PlatformCallbacks callbacks) { platform = std::move(callbacks); }

	static constexpr int32_t MODE_BRUSH = 0;
	static constexpr int32_t MODE_FLOOD_FILL = 1;
	static constexpr int32_t MODE_LINE = 2;
	static constexpr int32_t MODE_RECTANGLE = 3;
	static constexpr int32_t MODE_SELECTION = 4;
	static constexpr int32_t MODE_TEXT = 5;
	static constexpr int32_t MODE_PIPETTE = 6;
	static constexpr int32_t MODE_CIRCLE = 7;
	static constexpr int32_t MODE_ARROW = 8;
	static constexpr int32_t MODE_BEZIER = 9;
	static constexpr int32_t MODE_MAGIC_WAND = 10;
	static constexpr int32_t MODE_LASSO = 11;
	static constexpr int32_t MODE_POLYGON_LASSO = 12;
	static constexpr int32_t MODE_ELLIPSE_SELECTION = 13;
	static constexpr int32_t MODE_ERASER = 14;
	static constexpr int32_t MODE_SPRAY = 15;

	/// Single code for what a top-row digit (1–9, 0) does: tool modes \c MODE_BRUSH…\c MODE_SPRAY are their own numeric values (0–15); other actions use distinct ids (see implementation).
	enum class DigitHotkeyAction : int32_t
	{
		SymmetryNone = 32,
		SymmetryHorizontal = 33,
		SymmetryVertical = 34,
		SymmetryFourWay = 35,
		SymmetryRadial = 36,
		Undo = 40,
		Redo = 41,
		SelectionCopy = 42,
		SelectionCut = 43,
		SelectionDelete = 44,
		ClipboardCopyCanvas = 45,
		ClipboardPasteNew = 46,
		LayerNew = 47,
		LayerDelete = 48,
		LayerMoveUp = 49,
		LayerMoveDown = 50,
		LayerDuplicate = 51,
		LayerMergeDown = 52,
	};

	static constexpr DigitHotkeyAction digitHotkeyActionForToolMode(int32_t toolMode) { return static_cast<DigitHotkeyAction>(toolMode); }
	static constexpr DigitHotkeyAction digitHotkeyActionForSymmetry(bbe::SymmetryMode m) { return static_cast<DigitHotkeyAction>(32 + static_cast<int32_t>(m)); }
	static bool digitHotkeyActionIsValid(DigitHotkeyAction a);

	/// Selection, lasso, polygon lasso, and wand share one marquee; leaving this set of tools applies the selection (see ExamplePaint mode changes).
	static bool isSelectionLikeTool(int32_t toolMode);

	bool brushStrokeChangeRegistered = false;
	int32_t lastModeSnapshot = MODE_BRUSH;
	/// Tool to return to after sampling with the pipette (set when entering pipette).
	int32_t pipetteReturnMode = MODE_BRUSH;
	/// After pipette sampling, ignore tool mouse input until all buttons are released (avoids brush stroke on same click).
	bool suppressCanvasInputUntilMouseUp = false;

	/// Per-instance brush stroke sampling for \c runPaintEditorUpdate (was file-static; index 0 = newest sample).
	bbe::Vector2 brushStrokeUpdateRecentPoints[4]{};
	int32_t brushStrokeUpdateRecentPointCount = 0;
	bool brushStrokeUpdateLastDrawButtonDown = false;
	/// Frames spent in shadow brush/eraser/spray preview while the active tool is not drawing (clears workArea after the first shadow frame).
	uint32_t brushStrokeUpdateShadowCounter = 0;

	/// Top-row digits 1–9 then 0: `digitHotkeys[0]`…`[8]` = keys 1–9, `[9]` = key 0. Reassign with Ctrl+digit while hovering a bound control in Tools or Layers.
	/// When set, invoked after a digit binding changes (e.g. to persist settings).
	std::function<void()> onDigitHotkeysChanged;

	DigitHotkeyAction digitHotkeys[10] = {
		digitHotkeyActionForToolMode(MODE_BRUSH),
		digitHotkeyActionForToolMode(MODE_FLOOD_FILL),
		digitHotkeyActionForToolMode(MODE_LINE),
		digitHotkeyActionForToolMode(MODE_RECTANGLE),
		digitHotkeyActionForToolMode(MODE_SELECTION),
		digitHotkeyActionForToolMode(MODE_TEXT),
		digitHotkeyActionForToolMode(MODE_PIPETTE),
		digitHotkeyActionForToolMode(MODE_CIRCLE),
		digitHotkeyActionForToolMode(MODE_ARROW),
		digitHotkeyActionForToolMode(MODE_BEZIER),
	};

	friend void drawTextPreviewForGui(bbe::PrimitiveBrush2D &brush, PaintEditor &editor, const bbe::Vector2i &topLeft);
	friend void drawSelectionOutlineForGui(bbe::PrimitiveBrush2D &brush, const PaintEditor &editor, const bbe::Rectanglei &rect, bool alwaysDrawOutline);
	friend void drawSelectionPixelMaskOverlayForGui(bbe::PrimitiveBrush2D &brush, const PaintEditor &editor, const bbe::Rectanglei &canvasRect);

	bbe::Vector2 offset;
	bbe::String path;
	bbe::UndoableObject<PaintDocument> canvas;
	int32_t activeLayerIndex = 0;
	bbe::Image workArea;
	float zoomLevel = 1.f;
	bool openSaveChoicePopup = false;
	bool openSaveFailedPopup = false;
	bool openDropChoicePopup = false;
	/// Pending full document replace (new/open/paste-as-new/quit) while \c hasUnsavedChanges(); resolved via \c openUnsavedChangesPopup.
	struct PendingDocumentReplace
	{
		enum class Kind
		{
			None,
			NewBlank,
			OpenPath,
			PasteClipboard,
			QuitApplication,
		} kind = Kind::None;
		uint32_t blankW = 0;
		uint32_t blankH = 0;
		bbe::String path;
	};
	PendingDocumentReplace pendingDocumentReplace{};
	bool openUnsavedChangesPopup = false;
	/// After choosing Save in the unsaved dialog with no path yet: complete save via \c saveDocumentAs, then \c runPendingDocumentReplace().
	bool runPendingReplaceAfterSuccessfulSave = false;
	/// Re-open the unsaved dialog after a failed save that was part of that chain.
	bool reshowUnsavedAfterSaveFail = false;
	/// When set, \c saveDocumentAs must not assign \c path (document was replaced by the save-then-replace chain).
	bool suppressNextSaveAsPathAssign = false;
	/// After Save / Don't save on the quit confirmation, allow one close without showing the dialog again (see \c MyGame::onWindowCloseRequest).
	bool allowWindowCloseWithoutPromptOnce = false;
	/// Request application exit via window close (e.g. \c Window::close); set by the host game so quit uses the same GLFW path as the title-bar close button.
	std::function<void()> applicationExitRequested;
	bbe::List<bbe::String> pendingDroppedPaths;
	bool showHelpWindow = false;
	bool showNavigator = true;
	/// Screen-space minimap bounds from the last completed ImGui frame (see \c drawExamplePaintGui); drives \c getNavigatorRect().
	bbe::Rectangle navigatorMinimapHitRect{};
	bool navigatorMinimapHitRectValid = false;
	/// Bumped when workArea or other navigator-visible pixels change without a new \c canvasGeneration (live stroke, clearWorkArea, flood fill).
	uint64_t navigatorThumbnailCacheRevision = 0;
	uint64_t navigatorContentHashStored = ~(uint64_t)0;
	uint64_t navigatorThumbPixelHashStored = ~(uint64_t)0;
	/// Flattened visible layers (excluding workArea); valid while \c navigatorContentHashStored matches current content hash and any non-normal blend is active.
	bbe::Image navigatorCachedFlattenVisible;
	/// Docked tool-options window (View menu); can be closed and re-opened from the menu.
	bool showToolOptionsPanel = true;
	/// Colors window (primary/secondary, pipette, favorite swatches); View menu.
	bool showColorsPanel = true;

	static constexpr size_t favoriteColorSlotCount = 16;
	/// RGBA favorite swatches (default filled white on load from settings).
	float favoriteColorRgba[favoriteColorSlotCount][4]{};
	/// Called when a favorite swatch is edited (persist).
	std::function<void()> onFavoriteColorsChanged;

	/// Most recently used draw colors (MRU), shown in the Colors window; defaults to all white, persisted.
	static constexpr size_t colorHistoryCapacity = 16;
	float colorHistoryRgba[colorHistoryCapacity][4]{};
	size_t colorHistoryCount = colorHistoryCapacity;
	/// Called when a recent swatch is edited in the Colors window (persist).
	std::function<void()> onColorHistoryChanged;
	/// Set during brush/spray/flood strokes; consumed when the stroke commits (see \c resetColorHistoryStrokeTouched).
	bool colorHistoryStrokeTouchedPrimary = false;
	bool colorHistoryStrokeTouchedSecondary = false;

	/// Top colors by frequency on the flattened visible canvas (alpha = 0 pixels skipped); see \c ensureMostUsedOnCanvasColorsUpdated.
	static constexpr size_t mostUsedOnCanvasCapacity = 16;
	float mostUsedOnCanvasRgba[mostUsedOnCanvasCapacity][4]{};
	size_t mostUsedOnCanvasCount = 0;
	void ensureMostUsedOnCanvasColorsUpdated();

	int64_t canvasGeneration = 0;
	int64_t savedGeneration = 0;
	/// Bumped when document identity resets while \c canvasGeneration returns to 0 (e.g. new canvas); invalidates derived caches.
	uint32_t canvasVisualEpoch = 0;

	uint32_t mostUsedOnCanvasCacheKeyStored = 0;
	bool mostUsedOnCanvasCacheValid = false;

	struct EndpointDraftState
	{
		bool draftActive = false;
		bool draftUsesRightColor = false;
		bbe::Vector2 start;
		bbe::Vector2 end;
		int32_t dragEndpoint = 0; // 0=none, 1=start, 2=end
		bool dragInProgress = false;
		bool dragUsesRightColor = false;
	};

	struct ShapeDragState
	{
		bool draftActive = false;
		bool draftUsesRightColor = false;
		bool dragActive = false;
		bool dragUsesRightColor = false;
		bbe::Vector2i dragStart;
		bbe::Rectanglei dragPreviewRect;
		bbe::Image dragPreviewImage;
	};

	struct BezierState
	{
		bbe::List<bbe::Vector2> controlPoints;
		bool usesRightColor = false;
		int32_t dragPointIndex = -1;
	};

	enum class SaveFormat
	{
		PNG,
		LAYERED,
	};

	/// Interior fill for rectangle and ellipse shape tools (outline still uses the active mouse color).
	enum class ShapeFillMode : int32_t
	{
		None = 0,
		Primary,
		Secondary,
		Checkerboard,
		HorizontalStripes,
		VerticalStripes,
		DiagonalStripes,
		DiagonalStripesAlt,
		DotGrid,
		ConcentricRings,
		RadialGradient,
		Crosshatch,
		Brick,
		NoiseDither,
		COUNT,
	};

	static constexpr const char *LAYERED_FILE_MAGIC_V1 = "ExamplePaintLayeredV1";
	static constexpr const char *LAYERED_FILE_MAGIC = "ExamplePaintLayeredV2";
	static constexpr const char *LAYERED_FILE_MAGIC_V3 = "ExamplePaintLayeredV3";
	static constexpr const char *LAYERED_FILE_EXTENSION = ".bbepaint";

	float leftColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	int32_t mode = MODE_BRUSH;

	int32_t brushWidth = 1;
	/// Hard rectangular eraser: N×N pixels, alpha cleared to 0 (size 1 = 1×1).
	int32_t eraserSize = 1;
	/// Previous canvas mouse position for the current eraser drag (frame-to-frame stroke, not brush spline history).
	bbe::Vector2 eraserStrokePrevCanvasPos{};
	bool eraserStrokeHasPrev = false;
	/// Spray tool: previous canvas position for interpolating bursts while dragging.
	bbe::Vector2 sprayStrokePrevCanvasPos{};
	bool sprayStrokeHasPrev = false;
	/// Spray disk radius in pixels (independent of \c brushWidth).
	int32_t sprayWidth = 12;
	/// Droplets placed per burst (per symmetry copy); each frame samples the spray disk this many times.
	int32_t sprayDensity = 1;
	/// Last in-frame segment actually rasterized (for UI preview); same endpoints as eraseLineOnWorkAreaWithSymmetry(pNew, pOld).
	bool eraserPreviewSegmentActive = false;
	bbe::Vector2 eraserPreviewSegmentPNew{};
	bbe::Vector2 eraserPreviewSegmentPOld{};
	int32_t textFontSize = 20;
	int32_t textFontIndex = 0;
	char textBuffer[512] = "Text";
	bbe::List<FontEntry> availableFonts;
	bbe::Vector2 startMousePos;
	int32_t cornerRadius = 0;
	ShapeFillMode shapeFillMode = ShapeFillMode::None;
	/// When true, rectangle/circle outline is drawn as dashes whose period fits the closed perimeter (no seam).
	bool shapeStripedStroke = false;
	/// Target dash+gap length in pixels before snapping to a seamless divisor of the outline length.
	int32_t shapeStripePeriodPx = 16;
	/// Scale in pixels for repeating shape fills (checkerboard cell, stripe period, dot spacing, ring thickness, brick width, noise patch size, etc.). Unused for \ref ShapeFillMode::RadialGradient.
	int32_t shapeFillPatternCellPx = 8;
	/// Mixed into \ref ShapeFillMode::NoiseDither; change or reroll for a different random layout.
	uint32_t shapeFillNoiseSeed = 0xC001D00Du;

	bool drawGridLines = true;
	bool tiled = false;
	enum class SelectionHitZone
	{
		NONE,
		INSIDE,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		TOP_LEFT,
		TOP_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_RIGHT,
		ROTATION,
	};
	struct SelectionState
	{
		bool hasSelection = false;
		bbe::Rectanglei rect;
		bbe::Image clipboard;

		bool floating = false;
		bbe::Image floatingImage;
		bool pastedFromClipboard = false;

		bool dragActive = false;
		bbe::Vector2i dragStart;

		bool moveActive = false;
		bbe::Vector2i moveOffset;

		bool resizeActive = false;
		SelectionHitZone resizeZone = SelectionHitZone::NONE;

		bbe::Rectanglei interactionStartRect;
		bbe::Rectanglei previewRect;
		bbe::Image previewImage;

		float rotation = 0.f;
		bool rotationHandleActive = false;
		bbe::Vector2 rotationDragPivot;
		float rotationDragStartAngle = 0.f;
		float rotationDragBaseAngle = 0.f;

		/// When non-empty and matching `rect` dimensions, only pixels with alpha >= 128 are selected; otherwise the whole rectangle is selected.
		bbe::Image mask;

		/// Rectangular marquee + Ctrl: preserved while a new drag is in progress.
		bool mergeBackupHadSelection = false;
		bbe::Rectanglei mergeBackupRect{};
		bbe::Image mergeBackupMask;

		/// Freehand lasso drag (MODE_LASSO): path in continuous canvas space, clamped to [0,W]×[0,H] so outlines can sit on the outer pixel edges.
		bool lassoDragActive = false;
		std::vector<bbe::Vector2> lassoPath;

		/// Polygon lasso (MODE_POLYGON_LASSO): click to add vertices; close via first point, Enter, or right-click.
		std::vector<bbe::Vector2i> polygonLassoVertices;
	};
	SelectionState selection;

	/// Set each frame before pointer events: Ctrl adds to the current selection (magic wand and rectangular marquee).
	void setSelectionAdditiveModifier(bool ctrlDown) { selectionAdditiveModifier = ctrlDown; }
	bool selectionAdditiveModifier = false;

	int32_t magicWandTolerance = 32;
	void clampMagicWandTolerance();
	void applyMagicWandAt(const bbe::Vector2i &pixel, bool additive);

	/// Per-channel color tolerance for flood fill on the active layer (same rule as Magic Wand).
	int32_t floodFillTolerance = 0;
	/// When true, flood fill uses overflow (gap-bridging): tolerance 255 and Manhattan depth 1 past the strict fill boundary.
	bool floodFillSmartFill = false;
	void clampFloodFillTolerance();

	/// After clearing the selection with a click, suppress one wand sample on the same frame.
	bool consumeMagicWandSuppressedPick();

	/// True when `selection.mask` matches `selection.rect` and defines a non-rectangular (or sub-rect) selection.
	bool hasSelectionPixelMask() const;

	bool skipMagicWandSampleOnce = false;

	ShapeDragState rectangle;
	ShapeDragState circle;

	int32_t arrowHeadSize = 15;
	int32_t arrowHeadWidth = 12;
	bool arrowDoubleHeaded = false;
	bool arrowFilledHead = true;
	/// When set, finishing a line or arrow drag applies the stroke immediately (no endpoint gizmo).
	bool endpointApplyStrokeOnRelease = false;

	EndpointDraftState line;
	EndpointDraftState arrow;
	BezierState bezier;

	bool canvasResizeActive = false;
	int32_t canvasResizeHandleIndex = -1;
	bbe::Rectanglei canvasResizePreviewRect;

	bbe::SymmetryMode symmetryMode = bbe::SymmetryMode::None;
	int32_t radialSymmetryCount = 6;
	bool symmetryOffsetCustom = false;
	bbe::Vector2 symmetryOffset;

	bool antiAliasingEnabled = true;
	bool constrainSquareEnabled = false;
	bool pointerPrimaryDown = false;
	bool pointerSecondaryDown = false;
	bool hasPointerPos = false;
	bbe::Vector2 lastPointerCanvasPos;

	PlatformCallbacks platform{};

	// --- Command API (input-agnostic; driven by ExamplePaint.cpp controller) ---
	void setMode(int32_t newMode) { mode = newMode; }
	void setConstrainSquare(bool enabled) { constrainSquareEnabled = enabled; }
	void pointerDown(PointerButton button, const bbe::Vector2 &canvasPos);
	void pointerMove(const bbe::Vector2 &canvasPos);
	void pointerUp(PointerButton button, const bbe::Vector2 &canvasPos);
	void bezierBackspace();

	bbe::Colori getColor(bool useRight) const;

	/// Insert or promote \p rgba in MRU history (compares quantized 8-bit channels).
	void recordColorHistory(const float rgba[4]);
	void recordColorHistoryFromColori(const bbe::Colori &c);
	void resetColorHistoryStrokeTouched();

	template<typename CreateImage>
	void refreshActiveShapeDraftImage(bool draftActive, CreateImage createImage)
	{
		if (!draftActive) return;
		if (selection.moveActive || selection.resizeActive)
		{
			if (selection.previewRect.width > 0 && selection.previewRect.height > 0)
			{
				selection.previewImage = createImage(selection.previewRect.width, selection.previewRect.height);
				prepareImageForCanvas(selection.previewImage);
			}
			return;
		}
		if (selection.rect.width > 0 && selection.rect.height > 0)
		{
			selection.floatingImage = createImage(selection.rect.width, selection.rect.height);
			prepareImageForCanvas(selection.floatingImage);
		}
	}

	void finalizeEndpointDraft(bool &draftActive, int32_t &draftDragEndpoint);
	void redrawEndpointDraft(EndpointDraftState &state);
	void endpointPointerDown(EndpointDraftState &state, PointerButton button, const bbe::Vector2 &mouseCanvas);
	void endpointPointerMove(EndpointDraftState &state, const bbe::Vector2 &mouseCanvas);
	void endpointPointerUp(EndpointDraftState &state, PointerButton button, const bbe::Vector2 &mouseCanvas);

	bool handleFloatingDraftInteraction(bool draftActive, const bbe::Vector2 &canvasPos, PointerButton button);
	void updateSelectionTransformInteraction(const bbe::Vector2i &mousePixel, bool primaryDown);

	template<typename BuildRect, typename CreatePreview>
	bool updateShapeDragPreview(const bbe::Vector2i &dragStart, const bbe::Vector2i &mousePixel, bbe::Rectanglei &previewRect, bbe::Image &previewImage, bool shiftDown, BuildRect buildRect, CreatePreview createPreview)
	{
		const bbe::Vector2i constrainedPixel = shiftDown ? constrainToSquare(dragStart, mousePixel) : mousePixel;
		if (!buildRect(dragStart, constrainedPixel, previewRect))
		{
			previewRect = {};
			previewImage = {};
			return false;
		}
		previewImage = createPreview(previewRect.width, previewRect.height);
		prepareImageForCanvas(previewImage);
		return true;
	}

	template<typename BuildRect, typename CreateImage>
	void finalizeShapeDrag(bool &dragActive, bool &draftActive, bool &draftUsesRightColor, bool useRightColor, const bbe::Vector2i &dragStart, const bbe::Vector2i &mousePixel, bbe::Rectanglei &previewRect, bbe::Image &previewImage, bool shiftDown, BuildRect buildRect, CreateImage createImage)
	{
		dragActive = false;
		const bbe::Vector2i constrainedPixel = shiftDown ? constrainToSquare(dragStart, mousePixel) : mousePixel;
		bbe::Rectanglei draftRect;
		if (!buildRect(dragStart, constrainedPixel, draftRect))
		{
			previewRect = {};
			previewImage = {};
			return;
		}
		const bbe::Image draftImage = previewImage.getWidth() > 0 && previewImage.getHeight() > 0 ? previewImage : createImage(draftRect.width, draftRect.height, getColor(useRightColor));
		clearSelectionState();
		selection.hasSelection = true;
		selection.floating = true;
		selection.rect = draftRect;
		draftActive = true;
		draftUsesRightColor = useRightColor;
		selection.floatingImage = draftImage;
		prepareImageForCanvas(selection.floatingImage);
		previewRect = {};
		previewImage = {};
	}

	void updateFloatingShapePreview(ShapeDragState &shape, bool isCircle, const bbe::Vector2i &mousePixel);
	void finalizeFloatingShapeDrag(ShapeDragState &shape, bool isCircle, const bbe::Vector2i &mousePixel);

	void prepareImageForCanvas(bbe::Image &image) const;

	void clearSelectionState();

	/// Drop the marquee (non-floating or after caller handles floating) but keep the image clipboard.
	void clearMarqueePreservingClipboard();

	/// Commit floating/move/resize if needed, then clear the selection (clipboard preserved).
	void deselectAll();

	void selectWholeLayer();

	int32_t getCanvasWidth() const;

	int32_t getCanvasHeight() const;

	void clampActiveLayerIndex();

	void undo();

	void redo();

	void applyDigitHotkeyBinding(DigitHotkeyAction action);
	void copyFlattenedCanvasToClipboard();

	PaintLayer &getActiveLayer();

	const PaintLayer &getActiveLayer() const;

	bbe::Image &getActiveLayerImage();

	const bbe::Image &getActiveLayerImage() const;

	void prepareLayer(PaintLayer &layer) const;

	PaintLayer makeLayer(const bbe::String &name, int32_t width, int32_t height, const bbe::Color &color = bbe::Color(0.0f, 0.0f, 0.0f, 0.0f)) const;

	void prepareDocumentImages();

	void prepareForLayerTargetChange();

	bool isLayeredDocumentPath(const bbe::String &filePath) const;

	bool isSupportedDroppedDocumentPath(const bbe::String &filePath) const;

	bbe::Image flattenVisibleLayers() const;

	bbe::Colori getVisiblePixel(size_t x, size_t y) const;

	bbe::String makeLayerName() const;

	void addLayer();

	void mirrorAllLayersHorizontally();
	void mirrorAllLayersVertically();

	void rotateAllLayers90Clockwise();
	void rotateAllLayers90CounterClockwise();

	void importFileAsLayers(const bbe::List<bbe::String> &paths);

	void deleteActiveLayer();

	void moveActiveLayerUp();

	void moveActiveLayerDown();

	void duplicateActiveLayer();

	void mergeActiveLayerDown();

	void setActiveLayerIndex(int32_t newIndex);

	bbe::Vector2i toCanvasPixel(const bbe::Vector2 &pos) const;
	/// Line tool only: with AA off, snap to pixel via toCanvasPixel then use pixel centers (+0.5).
	bbe::Vector2 lineEndpointCanvasPos(const EndpointDraftState &state, const bbe::Vector2 &canvas) const;

	bbe::Vector2i toTiledCanvasPixel(const bbe::Vector2 &pos);

	bool clampRectToCanvas(const bbe::Rectanglei &rect, bbe::Rectanglei &outRect) const;

	bbe::Vector2i constrainToSquare(const bbe::Vector2i &start, const bbe::Vector2i &end) const;

	bool buildSelectionRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const;

	bool buildEllipseMarqueeRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const;

	void finishEllipseMarqueeDrag(const bbe::Vector2i &mousePixel);

	bbe::Rectanglei buildRawRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2) const;

	bool isPointInSelection(const bbe::Vector2i &point) const;

	bool isSelectionResizeHit(const SelectionHitZone hitZone) const;

	SelectionHitZone getSelectionHitZone(const bbe::Vector2 &pointCanvas) const;

	bbe::Image copyCanvasRect(const bbe::Rectanglei &rect) const;

	void clearCanvasRect(const bbe::Rectanglei &rect);

	void storeSelectionInClipboard();

	void deleteSelection();

	void cutSelection();

	bool getPasteImage(bbe::Image &image);

	void pasteSelectionAt(const bbe::Vector2i &pos);

	void commitFloatingSelection();

	/// Finalize move/resize/drag, commit floating pixels to the layer, then drop the marquee. Clipboard contents are kept.
	void applySelectionWhenLeavingTool();

	bool toImagePos(bbe::Vector2 &pos, int32_t width, int32_t height, bool repeated) const;

	// (removed drawing indirection helpers; call Image APIs directly)
	void drawArrowToWorkArea(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color);

	bbe::Colori getLineDraftColor() const;

	bbe::Colori getArrowDraftColor() const;

	void redrawLineDraft();

	void redrawArrowDraft();

	void redrawBezierDraft();

	void refreshBrushBasedDrafts();

	void finalizeLineDraft();
	void finalizeArrowDraft();

	bbe::Colori getBezierColor() const;

	void drawBezierToWorkArea(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color);

	void finalizeBezierDraft();

	bbe::Colori getRectangleDraftColor() const;

	bbe::Colori getRectangleDragColor() const;

	int32_t getRectangleDraftPadding() const;

	bbe::Rectanglei expandRectangleRect(const bbe::Rectanglei &rect) const;

	bool buildRectangleDraftRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const;

	bbe::Image createRectangleImage(int32_t width, int32_t height, const bbe::Colori &strokeColor, float rotation = 0.f) const;

	bbe::Image createRectangleDraftImage(int32_t width, int32_t height) const;

	bbe::Image createRectangleDragPreviewImage(int32_t width, int32_t height) const;

	void refreshActiveRectangleDraftImage();

	bbe::Colori getCircleDraftColor() const;

	bbe::Colori getCircleDragColor() const;

	bbe::Image createCircleImage(int32_t width, int32_t height, const bbe::Colori &strokeColor, float rotation = 0.f) const;

	bbe::Image createCircleDraftImage(int32_t width, int32_t height) const;

	bbe::Image createCircleDragPreviewImage(int32_t width, int32_t height) const;

	void refreshActiveCircleDraftImage();

	void finalizeCircleDrag(const bbe::Vector2i &mousePixel, bool shiftDown);

	void beginCircleDrag(const bbe::Vector2i &mousePixel, bool useRightColor);

	void updateCircleDragPreview(const bbe::Vector2i &mousePixel, bool shiftDown);


	/// Cut selected pixels to a floating layer if not already floating (same as starting a move/resize/rotate drag).
	void liftSelectionToFloatingIfNeeded();

	void beginSelectionMove(const bbe::Vector2i &mousePixel);

	/// Outside click, Ctrl+inside, or Ctrl on resize handles: clear / merge / start rect drag.
	void pointerDownSelectionDefaultMarqueePath(const bbe::Vector2i &mousePixel);
	void pointerDownLassoMarqueePath(const bbe::Vector2 &canvasPos);
	void appendLassoPoint(const bbe::Vector2 &canvasPos);
	void finishLassoDrag(const bbe::Vector2 &canvasPos);
	void commitSelectionFromClosedLassoPath(std::vector<bbe::Vector2> path);

	void pointerDownPolygonLassoMarqueePath(const bbe::Vector2i &mousePixel);
	void appendPolygonLassoVertex(const bbe::Vector2i &p);
	bool isClickClosePolygonLasso(const bbe::Vector2 &canvasPos) const;
	void finishPolygonLassoSelection();
	void cancelPolygonLassoDraft();
	void polygonLassoBackspace();
	void confirmPolygonLassoIfReady();

	void beginRotationDrag(const bbe::Vector2i &mousePixel);

	void updateRotationDrag(const bbe::Vector2i &mousePixel);

	void updateSelectionMovePreview(const bbe::Vector2i &mousePixel);

	void beginSelectionResize(const SelectionHitZone hitZone);

	void updateSelectionResizePreview(const bbe::Vector2i &mousePixel);

	bbe::Image buildSelectionPreviewResultImage() const;

	void clearSelectionInteractionState();

	void applySelectionTransform();

	/// Move the selection like a one-pixel drag: lifts to floating if needed, then offsets the rect (may go past canvas edges like mouse move; commit crops).
	void nudgeSelectionByPixels(int32_t dx, int32_t dy);

	void finalizeRectangleDrag(const bbe::Vector2i &mousePixel, bool shiftDown);

	void beginRectangleDrag(const bbe::Vector2i &mousePixel, bool useRightColor);

	void updateRectangleDragPreview(const bbe::Vector2i &mousePixel, bool shiftDown);


	bbe::Rectangle selectionRectToScreen(const bbe::Rectanglei &rect) const;

	// Returns the screen-space position of canvas resize handle i (0=TL,1=T,2=TR,3=R,4=BR,5=B,6=BL,7=L).
	bbe::Vector2 getCanvasHandleScreenPos(int32_t i) const;

	// Returns the index (0-7) of the canvas resize handle the screen-space point is near, or -1.
	int32_t getCanvasResizeHitHandle(const bbe::Vector2 &screenPos) const;

	void updateCanvasResizePreview(const bbe::Vector2 &canvasMousePos);

	void applyCanvasResize(const bbe::Rectanglei &previewRect);

	void clampBrushWidth();
	void clampEraserSize();
	void clampSprayWidth();
	void clampSprayDensity();
	void clampShapeStripePeriod();
	void clampShapeFillPatternCellPx();
	void rerollShapeFillNoiseSeed();

	void clampTextFontSize();

	void clampTextFontIndex();

	void buildAvailableFontList();

	const bbe::Font &getTextToolFont() const;

	const bbe::Image &getTextGlyphImage(const bbe::Font &font, int32_t codePoint) const;

	bbe::String getTextBufferString() const;

	bool getTextOriginAndBounds(const bbe::Vector2i &topLeft, bbe::Vector2 &outOrigin, bbe::Rectangle &outBounds) const;

	// Renders the text (as it would appear at topLeft) into a standalone image.
	bbe::Image renderTextToImage(const bbe::Vector2i &topLeft, const bbe::Colori &color) const;

	bool drawTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color);

	void placeTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color);

	void swapColors();

	void resetColorsToDefault();

	void serializeLayerImage(const bbe::Image &image, bbe::ByteBuffer &buffer) const;

	bool deserializeLayerImage(bbe::ByteBufferSpan &span, int32_t width, int32_t height, bbe::Image &outImage) const;

	// Memory-only layered-document serialization (unit-test friendly).
	bbe::ByteBuffer serializeLayeredDocumentBytes() const;
	bool deserializeLayeredDocumentBytes(bbe::ByteBufferSpan span, PaintDocument &outDocument, int32_t &outStoredActiveLayerIndex) const;

	bool saveLayeredDocument(const bbe::String &filePath);

	bool loadLayeredDocument(const bbe::String &filePath);
	
	bool saveFlattenedImage(const bbe::String &filePath);

	bool saveDocumentToPath(const bbe::String &filePath);

	void saveDocumentAs(SaveFormat format);

	void requestSave();

	void saveCanvas();

	void resetCamera();

	void clearWorkArea();

	void bumpNavigatorThumbnailDirty();

	void submitCanvas();

	void applyWorkArea();
	/// Commits erase marks from workArea: sets active layer alpha to 0 where the workArea mask is non-transparent.
	void applyEraserWorkArea();

	void setupCanvas(bool clearHistory = true);

	bool hasUnsavedChanges() const { return canvasGeneration != savedGeneration; }

	/// If the document is clean, replaces immediately; otherwise opens the unsaved-changes dialog.
	void requestReplaceDocumentNewBlank(uint32_t width, uint32_t height);
	void requestReplaceDocumentOpen(const bbe::String &filePath);
	void requestReplaceDocumentPasteFromClipboard();
	void requestReplaceApplicationQuit();

	void runPendingDocumentReplace();
	void clearPendingDocumentReplace();

	/// After loading a flat image: opaque white backdrop if every pixel has alpha 255, else transparent backdrop.
	void setCanvasFallbackFromImageAlpha(const bbe::Image &image);

	bbe::Vector2 screenToCanvas(const bbe::Vector2 &pos);

	bbe::Rectangle getNavigatorRect();

	bool toTiledPos(bbe::Vector2 &pos);

	void changeZoom(float val, const bbe::Vector2 &mouseScreenPos);

	bbe::Colori activeDrawColor(bool leftDown, bool rightDown) const;

	bbe::Vector2 getSymmetryCenter() const;

	bbe::List<bbe::Vector2> getSymmetryPositions(const bbe::Vector2 &pos) const;

	bbe::List<float> getSymmetryRotationAngles() const;

	bool touch(const bbe::Vector2 &touchPos, bool rectangleShape, bool leftDown, bool rightDown);

	bool touchLine(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, bool rectangleShape, bool leftDown, bool rightDown);

	/// N×N canvas pixel rectangle (N = eraserSize) centered with the same rule as the erase stamp.
	bbe::Rectanglei getEraserPixelRect(const bbe::Vector2 &canvasPos) const;

	/// Preview rects for the lerped segment (same sampling as eraseLineOnWorkAreaWithSymmetry).
	void appendEraserStampRectsAlongSegment(const bbe::Vector2 &pNew, const bbe::Vector2 &pOld, bbe::List<bbe::Rectanglei> &out) const;

	/// Marks an N×N erase region on workArea (all symmetry copies). N = eraserSize.
	bool eraseStampAtCanvasWithSymmetry(const bbe::Vector2 &canvasPos);
	/// Lerps from pOld to pNew (ceil distance steps); each sample uses getEraserPixelRect like the stamp tool.
	bool eraseLineOnWorkAreaWithSymmetry(const bbe::Vector2 &pNew, const bbe::Vector2 &pOld);

	/// Random droplets in a disk of radius \c sprayWidth at each symmetry copy of \c canvasPos.
	bool sprayBurstAtCanvasWithSymmetry(const bbe::Vector2 &canvasPos, bool leftDown, bool rightDown);
	/// Like eraseLineOnWorkAreaWithSymmetry: one burst per interpolated step along the segment.
	bool sprayLineOnWorkAreaWithSymmetry(const bbe::Vector2 &pNew, const bbe::Vector2 &pOld, bool leftDown, bool rightDown);

	void touchLineSymmetry(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, const bbe::Colori &color, int32_t width, bool rectShape = false);

	void drawArrowSymmetry(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color);

	void drawBezierSymmetry(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color);

	void onStart(const PaintWindowMetrics &window);

	void onFilesDropped(const bbe::List<bbe::String> &paths);

private:
	void resetBrushStrokeUpdateState();

	void newCanvas(uint32_t width, uint32_t height);
	bool newCanvas(const char *path);
	bool pasteClipboardAsNewDocument();
};
