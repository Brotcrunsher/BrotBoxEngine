#pragma once

#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.
#include "BBE/Editor/RectSelectionGizmo2D.h"
#include "BBE/Symmetry2D.h"
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <vector>

#include "ExamplePaintInput.h"

// TODO: Flood fill with edges of brush tool kinda bad.
// TODO: Bug: right click has weird behaviour with shadow

// TODO: Alpha eraser tool - not just recolering pixels but setting their alpha to 0.
// TODO: Scaling whole picture/selection up/down
// TODO: Pixel perfect manipulation with arrow keys
// TODO: "Filled with color" option for rectangle/circle tool
// TODO: Color history
// TODO: Selection via magic wand / color selection

// TODO: It's possible to enter negative numbers for new canvas size. Leads to a crash. Don't allow negative sizes.
// TODO: Saving an image always returns success, even if the file couldn't be written. Fix that.

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
	bbe::List<PaintLayer> layers;
};

struct PaintEditor
{
	PaintWindowMetrics viewport{};
	void setViewportMetrics(const PaintWindowMetrics &w) { viewport = w; }

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

	bool brushStrokeChangeRegistered = false;
	int32_t lastModeSnapshot = MODE_BRUSH;

	friend void drawExamplePaintGui(PaintEditor &editor, bbe::PrimitiveBrush2D &brush, bbe::Game &game);
	friend void drawTextPreviewForGui(bbe::PrimitiveBrush2D &brush, PaintEditor &editor, const bbe::Vector2i &topLeft);
	friend void drawSelectionOutlineForGui(bbe::PrimitiveBrush2D &brush, const PaintEditor &editor, const bbe::Rectanglei &rect);

	bbe::Vector2 offset;
	bbe::String path;
	bbe::UndoableObject<PaintDocument> canvas;
	int32_t activeLayerIndex = 0;
	bbe::Image workArea;
	float zoomLevel = 1.f;
	bool openSaveChoicePopup = false;
	bool openDropChoicePopup = false;
	bbe::List<bbe::String> pendingDroppedPaths;
	bool showHelpWindow = false;
	bool showNavigator = true;
	int64_t canvasGeneration = 0;
	int64_t savedGeneration = 0;

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

	static constexpr const char *LAYERED_FILE_MAGIC_V1 = "ExamplePaintLayeredV1";
	static constexpr const char *LAYERED_FILE_MAGIC = "ExamplePaintLayeredV2";
	static constexpr const char *LAYERED_FILE_EXTENSION = ".bbepaint";

	float leftColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	int32_t mode = MODE_BRUSH;

	int32_t brushWidth = 1;
	int32_t textFontSize = 20;
	int32_t textFontIndex = 0;
	char textBuffer[512] = "Text";
	bbe::List<FontEntry> availableFonts;
	bbe::Vector2 startMousePos;
	int32_t cornerRadius = 0;

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
	};
	SelectionState selection;

	ShapeDragState rectangle;
	ShapeDragState circle;

	int32_t arrowHeadSize = 15;
	int32_t arrowHeadWidth = 12;
	bool arrowDoubleHeaded = false;
	bool arrowFilledHead = true;

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

	bbe::Colori getColor(bool useRight) const;

	template<typename CreateImage>
	void refreshActiveShapeDraftImage(bool draftActive, CreateImage createImage)
	{
		if (!draftActive) return;
		if (selection.moveActive || selection.resizeActive)
		{
			if (selection.previewRect.width > 0 && selection.previewRect.height > 0) selection.previewImage = createImage(selection.previewRect.width, selection.previewRect.height);
			return;
		}
		if (selection.rect.width > 0 && selection.rect.height > 0) selection.floatingImage = createImage(selection.rect.width, selection.rect.height);
	}

	void finalizeEndpointDraft(bool &draftActive, int32_t &draftDragEndpoint);

	template<typename Draw, typename Finalize>
	void updateEndpointDraftTool(bbe::Game &g, bool &draftActive, bool &draftUsesRightColor, bbe::Vector2 &draftStart, bbe::Vector2 &draftEnd, int32_t &draftDragEndpoint, bool &dragInProgress, bool &dragUsesRightColor, Draw draw, Finalize finalize)
	{
		const bbe::Vector2 mouseCanvas = screenToCanvas(g.getMouse());
		if (draftActive)
		{
			bool handledMousePress = false;
			if (g.isMousePressed(bbe::MouseButton::LEFT))
			{
				const float handleRadius = 6.f / zoomLevel;
				const float distToStart = (mouseCanvas - draftStart).getLength();
				const float distToEnd = (mouseCanvas - draftEnd).getLength();
				if (distToStart <= handleRadius && distToStart <= distToEnd) draftDragEndpoint = 1;
				else if (distToEnd <= handleRadius) draftDragEndpoint = 2;
				else
				{
					finalize();
					return;
				}
				handledMousePress = true;
			}
			if (!handledMousePress && g.isMousePressed(bbe::MouseButton::RIGHT))
			{
				finalize();
				return;
			}
			if (draftDragEndpoint != 0 && g.isMouseDown(bbe::MouseButton::LEFT)) (draftDragEndpoint == 1 ? draftStart : draftEnd) = mouseCanvas;
			if (draftDragEndpoint != 0 && g.isMouseReleased(bbe::MouseButton::LEFT)) draftDragEndpoint = 0;
			clearWorkArea();
			draw(draftStart, draftEnd, getColor(draftUsesRightColor));
			return;
		}
		if (!dragInProgress)
		{
			if (g.isMousePressed(bbe::MouseButton::LEFT))
			{
				dragInProgress = true;
				dragUsesRightColor = false;
				draftStart = mouseCanvas;
			}
			else if (g.isMousePressed(bbe::MouseButton::RIGHT))
			{
				dragInProgress = true;
				dragUsesRightColor = true;
				draftStart = mouseCanvas;
			}
		}
		if (!dragInProgress) return;
		clearWorkArea();
		draw(draftStart, mouseCanvas, getColor(dragUsesRightColor));
		if ((dragUsesRightColor ? g.isMouseReleased(bbe::MouseButton::RIGHT) : g.isMouseReleased(bbe::MouseButton::LEFT)))
		{
			draftActive = true;
			draftUsesRightColor = dragUsesRightColor;
			draftEnd = mouseCanvas;
			dragInProgress = false;
		}
	}

	bool handleFloatingDraftInteraction(bool draftActive, const bbe::Vector2i &mousePixel, bbe::Game &g);

	void updateSelectionTransformInteraction(const bbe::Vector2i &mousePixel, bbe::Game &g);

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

	template<typename BeginDrag, typename UpdatePreview, typename FinalizeDrag>
	void updateFloatingShapeTool(const bbe::Vector2 &currMousePos, bool draftActive, bool &dragActive, bool dragUsesRightColor, bbe::Game &g, bool shiftDown, BeginDrag beginDrag, UpdatePreview updatePreview, FinalizeDrag finalizeDrag)
	{
		const bbe::Vector2i mousePixel = toCanvasPixel(currMousePos);
		const bool handledMousePress = handleFloatingDraftInteraction(draftActive, mousePixel, g);
		if (!handledMousePress && !draftActive)
		{
			if (g.isMousePressed(bbe::MouseButton::LEFT)) beginDrag(mousePixel, false);
			else if (g.isMousePressed(bbe::MouseButton::RIGHT)) beginDrag(mousePixel, true);
		}
		updateSelectionTransformInteraction(mousePixel, g);
		if (!dragActive) return;
		updatePreview(mousePixel);
		if (dragUsesRightColor ? g.isMouseReleased(bbe::MouseButton::RIGHT) : g.isMouseReleased(bbe::MouseButton::LEFT)) finalizeDrag(mousePixel);
	}

	void prepareImageForCanvas(bbe::Image &image) const;

	void clearSelectionState();

	void selectWholeLayer();

	int32_t getCanvasWidth() const;

	int32_t getCanvasHeight() const;

	void clampActiveLayerIndex();

	void undo();

	void redo();

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

	void importFileAsLayers(const bbe::List<bbe::String> &paths);

	void deleteActiveLayer();

	void moveActiveLayerUp();

	void moveActiveLayerDown();

	void duplicateActiveLayer();

	void mergeActiveLayerDown();

	void setActiveLayerIndex(int32_t newIndex);

	bbe::Vector2i toCanvasPixel(const bbe::Vector2 &pos) const;

	bbe::Vector2i toTiledCanvasPixel(const bbe::Vector2 &pos);

	bool clampRectToCanvas(const bbe::Rectanglei &rect, bbe::Rectanglei &outRect) const;

	bbe::Vector2i constrainToSquare(const bbe::Vector2i &start, const bbe::Vector2i &end) const;

	bool buildSelectionRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const;

	bbe::Rectanglei buildRawRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2) const;

	bool isPointInSelection(const bbe::Vector2i &point) const;

	bool isSelectionResizeHit(const SelectionHitZone hitZone) const;

	SelectionHitZone getSelectionHitZone(const bbe::Vector2i &point) const;

	bool isWholeLayerSelection(const bbe::Rectanglei &rect) const;

	bool shouldClearWholeLayerSelectionToTransparency() const;

	bbe::Image copyCanvasRect(const bbe::Rectanglei &rect) const;

	void clearCanvasRect(const bbe::Rectanglei &rect);

	void storeSelectionInClipboard();

	void deleteSelection();

	void cutSelection();

	bool getPasteImage(bbe::Image &image);

	void pasteSelectionAt(const bbe::Vector2i &pos);

	void commitFloatingSelection();

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

	void updateLineTool(bbe::Game &g);

	void updateArrowTool(bbe::Game &g);

	bbe::Colori getBezierColor() const;

	void drawBezierToWorkArea(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color);

	void finalizeBezierDraft();

	void updateBezierTool(bbe::Game &g);

	bbe::Colori getRectangleDraftColor() const;

	bbe::Colori getRectangleDragColor() const;

	int32_t getRectangleDraftPadding() const;

	bbe::Rectanglei expandRectangleRect(const bbe::Rectanglei &rect) const;

	bool buildRectangleDraftRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const;

	bbe::Image createRectangleImage(int32_t width, int32_t height, const bbe::Colori &color, float rotation = 0.f) const;

	bbe::Image createRectangleDraftImage(int32_t width, int32_t height) const;

	bbe::Image createRectangleDragPreviewImage(int32_t width, int32_t height) const;

	void refreshActiveRectangleDraftImage();

	bbe::Colori getCircleDraftColor() const;

	bbe::Colori getCircleDragColor() const;

	bbe::Image createCircleImage(int32_t width, int32_t height, const bbe::Colori &color, float rotation = 0.f) const;

	bbe::Image createCircleDraftImage(int32_t width, int32_t height) const;

	bbe::Image createCircleDragPreviewImage(int32_t width, int32_t height) const;

	void refreshActiveCircleDraftImage();

	void finalizeCircleDrag(const bbe::Vector2i &mousePixel, bool shiftDown);

	void beginCircleDrag(const bbe::Vector2i &mousePixel, bool useRightColor);

	void updateCircleDragPreview(const bbe::Vector2i &mousePixel, bool shiftDown);

	void updateCircleTool(const bbe::Vector2 &currMousePos, bbe::Game &g, bool shiftDown);

	void beginSelectionMove(const bbe::Vector2i &mousePixel);

	void beginRotationDrag(const bbe::Vector2i &mousePixel);

	void updateRotationDrag(const bbe::Vector2i &mousePixel);

	void updateSelectionMovePreview(const bbe::Vector2i &mousePixel);

	void beginSelectionResize(const SelectionHitZone hitZone);

	void updateSelectionResizePreview(const bbe::Vector2i &mousePixel);

	bbe::Image buildSelectionPreviewResultImage() const;

	void clearSelectionInteractionState();

	void applySelectionTransform();

	void updateSelectionTool(const bbe::Vector2 &currMousePos, bbe::Game &g);

	void finalizeRectangleDrag(const bbe::Vector2i &mousePixel, bool shiftDown);

	void beginRectangleDrag(const bbe::Vector2i &mousePixel, bool useRightColor);

	void updateRectangleDragPreview(const bbe::Vector2i &mousePixel, bool shiftDown);

	void updateRectangleTool(const bbe::Vector2 &currMousePos, bbe::Game &g, bool shiftDown);

	bbe::Rectangle selectionRectToScreen(const bbe::Rectanglei &rect) const;

	// Returns the screen-space position of canvas resize handle i (0=TL,1=T,2=TR,3=R,4=BR,5=B,6=BL,7=L).
	bbe::Vector2 getCanvasHandleScreenPos(int32_t i) const;

	// Returns the index (0-7) of the canvas resize handle the screen-space point is near, or -1.
	int32_t getCanvasResizeHitHandle(const bbe::Vector2 &screenPos) const;

	void updateCanvasResizePreview(const bbe::Vector2 &canvasMousePos);

	void applyCanvasResize(const bbe::Rectanglei &previewRect);

	void clampBrushWidth();

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

	bool saveLayeredDocument(const bbe::String &filePath);

	bool loadLayeredDocument(const bbe::String &filePath);

	bool saveFlattenedPng(const bbe::String &filePath);

	bool saveDocumentToPath(const bbe::String &filePath);

	void saveDocumentAs(SaveFormat format);

	void requestSave();

	void saveCanvas();

	void resetCamera();

	void clearWorkArea();

	void submitCanvas();

	void applyWorkArea();

	void setupCanvas(bool clearHistory = true);

	void newCanvas(uint32_t width, uint32_t height);

	bool newCanvas(const char *path);

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

	void touchLineSymmetry(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, const bbe::Colori &color, int32_t width, bool rectShape = false);

	void drawArrowSymmetry(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color);

	void drawBezierSymmetry(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color);

	void onStart(const PaintWindowMetrics &window);

	void onFilesDropped(const bbe::List<bbe::String> &paths);
};
