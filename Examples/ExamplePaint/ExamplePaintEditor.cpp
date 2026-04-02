#include "ExamplePaintEditor.h"
#include <algorithm>
#include <cstddef>
#include <filesystem>

#include <string>

bbe::Colori PaintEditor::getColor(bool useRight) const
{
	return bbe::Color(useRight ? rightColor : leftColor).asByteColor();
}

void PaintEditor::pointerDown(PointerButton button, const bbe::Vector2 &canvasPos)
{
	hasPointerPos = true;
	lastPointerCanvasPos = canvasPos;

	if (button == PointerButton::Primary) pointerPrimaryDown = true;
	if (button == PointerButton::Secondary) pointerSecondaryDown = true;

	const bbe::Vector2i mousePixel = toCanvasPixel(canvasPos);

	switch (mode)
	{
	case MODE_SELECTION:
	{
		if (button != PointerButton::Primary) break;
		const SelectionHitZone hitZone = getSelectionHitZone(mousePixel);
		if (hitZone == SelectionHitZone::ROTATION && selection.hasSelection)
		{
			beginRotationDrag(mousePixel);
		}
		else if (isSelectionResizeHit(hitZone))
		{
			beginSelectionResize(hitZone);
		}
		else if (hitZone == SelectionHitZone::INSIDE)
		{
			beginSelectionMove(mousePixel);
		}
		else
		{
			if (selection.floating)
			{
				commitFloatingSelection();
			}
			selection.dragActive = true;
			selection.dragStart = mousePixel;
			selection.hasSelection = false;
			selection.rect = {};
			selection.previewRect = {};
		}
		break;
	}
	case MODE_RECTANGLE:
	case MODE_CIRCLE:
	{
		ShapeDragState &shape = (mode == MODE_CIRCLE) ? circle : rectangle;
		const bool isCircle = (mode == MODE_CIRCLE);
		const bool handled = handleFloatingDraftInteraction(shape.draftActive, mousePixel, button);
		if (!handled && !shape.draftActive && (button == PointerButton::Primary || button == PointerButton::Secondary))
		{
			shape.dragActive = true;
			shape.dragUsesRightColor = (button == PointerButton::Secondary);
			shape.dragStart = mousePixel;
			shape.dragPreviewRect = {};
			shape.dragPreviewImage = {};
			(void)isCircle;
		}
		break;
	}
	case MODE_LINE:
		endpointPointerDown(line, false, button, canvasPos);
		break;
	case MODE_ARROW:
		endpointPointerDown(arrow, true, button, canvasPos);
		break;
	case MODE_BEZIER:
	{
		if (button == PointerButton::Primary)
		{
			const float handleRadius = 6.f / zoomLevel;
			int32_t hitIndex = -1;
			float bestDist = handleRadius;
			for (size_t i = 0; i < bezier.controlPoints.getLength(); i++)
			{
				const float dist = (canvasPos - bezier.controlPoints[i]).getLength();
				if (dist < bestDist)
				{
					bestDist = dist;
					hitIndex = (int32_t)i;
				}
			}
			if (hitIndex >= 0)
			{
				bezier.dragPointIndex = hitIndex;
			}
			else
			{
				if (bezier.controlPoints.isEmpty()) bezier.usesRightColor = false;
				bezier.controlPoints.add(canvasPos);
			}
			// Rebuild preview immediately
			pointerMove(canvasPos);
		}
		else if (button == PointerButton::Secondary)
		{
			finalizeBezierDraft();
		}
		break;
	}
	default:
		break;
	}
}

void PaintEditor::pointerMove(const bbe::Vector2 &canvasPos)
{
	hasPointerPos = true;
	lastPointerCanvasPos = canvasPos;

	const bbe::Vector2i mousePixel = toCanvasPixel(canvasPos);

	// Selection transform interactions can be active in multiple modes (selection, rectangle, circle).
	updateSelectionTransformInteraction(mousePixel, pointerPrimaryDown);

	switch (mode)
	{
	case MODE_SELECTION:
	{
		if (!pointerPrimaryDown) break;
		if (selection.rotationHandleActive)
		{
			updateRotationDrag(mousePixel);
		}
		if (selection.dragActive)
		{
			buildSelectionRect(selection.dragStart, mousePixel, selection.previewRect);
		}
		if (selection.moveActive)
		{
			updateSelectionMovePreview(mousePixel);
		}
		if (selection.resizeActive)
		{
			updateSelectionResizePreview(mousePixel);
		}
		break;
	}
	case MODE_RECTANGLE:
	case MODE_CIRCLE:
	{
		ShapeDragState &shape = (mode == MODE_CIRCLE) ? circle : rectangle;
		const bool isCircle = (mode == MODE_CIRCLE);
		if (!shape.dragActive) break;
		updateFloatingShapePreview(shape, isCircle, mousePixel);
		break;
	}
	case MODE_LINE:
		endpointPointerMove(line, false, canvasPos);
		break;
	case MODE_ARROW:
		endpointPointerMove(arrow, true, canvasPos);
		break;
	case MODE_BEZIER:
	{
		// Update dragged control point
		if (bezier.dragPointIndex >= 0 && pointerPrimaryDown)
		{
			bezier.controlPoints[(size_t)bezier.dragPointIndex] = canvasPos;
		}

		// Rebuild workArea preview
		clearWorkArea();
		if (bezier.controlPoints.getLength() >= 2)
		{
			const float handleRadius = 6.f / zoomLevel;
			bool nearExisting = false;
			for (size_t i = 0; i < bezier.controlPoints.getLength(); i++)
			{
				if ((canvasPos - bezier.controlPoints[i]).getLength() < handleRadius)
				{
					nearExisting = true;
					break;
				}
			}
			bbe::List<bbe::Vector2> previewPoints = bezier.controlPoints;
			if (!nearExisting && bezier.dragPointIndex < 0)
			{
				previewPoints.add(canvasPos);
			}
			drawBezierSymmetry(previewPoints, getBezierColor());
		}
		else if (bezier.controlPoints.getLength() == 1)
		{
			touchLineSymmetry(bezier.controlPoints[0], canvasPos, getBezierColor(), brushWidth);
		}
		break;
	}
	default:
		break;
	}
}

void PaintEditor::pointerUp(PointerButton button, const bbe::Vector2 &canvasPos)
{
	hasPointerPos = true;
	lastPointerCanvasPos = canvasPos;

	if (button == PointerButton::Primary) pointerPrimaryDown = false;
	if (button == PointerButton::Secondary) pointerSecondaryDown = false;

	const bbe::Vector2i mousePixel = toCanvasPixel(canvasPos);

	// Finish selection transforms on primary release.
	if (button == PointerButton::Primary)
	{
		if (selection.rotationHandleActive) selection.rotationHandleActive = false;
		if (selection.moveActive || selection.resizeActive) applySelectionTransform();
	}

	switch (mode)
	{
	case MODE_SELECTION:
	{
		if (button != PointerButton::Primary) break;
		if (selection.dragActive)
		{
			selection.hasSelection = buildSelectionRect(selection.dragStart, mousePixel, selection.rect);
			selection.dragActive = false;
			selection.previewRect = {};
		}
		break;
	}
	case MODE_RECTANGLE:
	case MODE_CIRCLE:
	{
		ShapeDragState &shape = (mode == MODE_CIRCLE) ? circle : rectangle;
		const bool isCircle = (mode == MODE_CIRCLE);
		if (!shape.dragActive) break;
		const bool releaseMatches = (shape.dragUsesRightColor && button == PointerButton::Secondary) || (!shape.dragUsesRightColor && button == PointerButton::Primary);
		if (releaseMatches)
		{
			finalizeFloatingShapeDrag(shape, isCircle, mousePixel);
		}
		break;
	}
	case MODE_LINE:
		endpointPointerUp(line, button, canvasPos);
		break;
	case MODE_ARROW:
		endpointPointerUp(arrow, button, canvasPos);
		break;
	case MODE_BEZIER:
		if (button == PointerButton::Primary) bezier.dragPointIndex = -1;
		break;
	default:
		break;
	}
}

void PaintEditor::bezierBackspace()
{
	if (bezier.controlPoints.isEmpty()) return;
	bezier.controlPoints.popBack();
	bezier.dragPointIndex = -1;
	if (hasPointerPos) pointerMove(lastPointerCanvasPos);
	else redrawBezierDraft();
}

void PaintEditor::finalizeEndpointDraft(bool &draftActive, int32_t &draftDragEndpoint)
{
	applyWorkArea();
	submitCanvas();
	draftActive = false;
	draftDragEndpoint = 0;
}

void PaintEditor::redrawEndpointDraft(EndpointDraftState &state, bool isArrow)
{
	if (!state.draftActive) return;
	clearWorkArea();
	if (isArrow) drawArrowSymmetry(state.start, state.end, getColor(state.draftUsesRightColor));
	else touchLineSymmetry(state.end, state.start, getColor(state.draftUsesRightColor), brushWidth);
}

void PaintEditor::endpointPointerDown(EndpointDraftState &state, bool isArrow, PointerButton button, const bbe::Vector2 &mouseCanvas)
{
	// Adjust active draft by dragging endpoints or commit/cancel.
	if (state.draftActive)
	{
		if (button == PointerButton::Primary)
		{
			const float handleRadius = 6.f / zoomLevel;
			const float distToStart = (mouseCanvas - state.start).getLength();
			const float distToEnd = (mouseCanvas - state.end).getLength();
			if (distToStart <= handleRadius && distToStart <= distToEnd) state.dragEndpoint = 1;
			else if (distToEnd <= handleRadius) state.dragEndpoint = 2;
			else
			{
				if (isArrow) finalizeArrowDraft();
				else finalizeLineDraft();
				return;
			}
			redrawEndpointDraft(state, isArrow);
			return;
		}
		if (button == PointerButton::Secondary)
		{
			if (isArrow) finalizeArrowDraft();
			else finalizeLineDraft();
			return;
		}
		return;
	}

	// Begin new drag.
	if (!state.dragInProgress && (button == PointerButton::Primary || button == PointerButton::Secondary))
	{
		state.dragInProgress = true;
		state.dragUsesRightColor = (button == PointerButton::Secondary);
		state.start = mouseCanvas;
		clearWorkArea();
	}
}

void PaintEditor::endpointPointerMove(EndpointDraftState &state, bool isArrow, const bbe::Vector2 &mouseCanvas)
{
	if (state.draftActive)
	{
		if (state.dragEndpoint != 0 && pointerPrimaryDown)
		{
			(state.dragEndpoint == 1 ? state.start : state.end) = mouseCanvas;
		}
		redrawEndpointDraft(state, isArrow);
		return;
	}
	if (!state.dragInProgress) return;

	clearWorkArea();
	if (isArrow) drawArrowSymmetry(state.start, mouseCanvas, getColor(state.dragUsesRightColor));
	else touchLineSymmetry(mouseCanvas, state.start, getColor(state.dragUsesRightColor), brushWidth);
}

void PaintEditor::endpointPointerUp(EndpointDraftState &state, PointerButton button, const bbe::Vector2 &mouseCanvas)
{
	if (state.draftActive)
	{
		if (button == PointerButton::Primary) state.dragEndpoint = 0;
		return;
	}
	if (!state.dragInProgress) return;

	const bool releaseMatches = (state.dragUsesRightColor && button == PointerButton::Secondary) || (!state.dragUsesRightColor && button == PointerButton::Primary);
	if (!releaseMatches) return;

	state.draftActive = true;
	state.draftUsesRightColor = state.dragUsesRightColor;
	state.end = mouseCanvas;
	state.dragInProgress = false;
}

bool PaintEditor::handleFloatingDraftInteraction(bool draftActive, const bbe::Vector2i &mousePixel, PointerButton button)
{
	if (!draftActive) return false;
	if (button == PointerButton::Primary)
	{
		const SelectionHitZone hitZone = getSelectionHitZone(mousePixel);
		if (hitZone == SelectionHitZone::ROTATION) beginRotationDrag(mousePixel);
		else if (isSelectionResizeHit(hitZone)) beginSelectionResize(hitZone);
		else if (hitZone == SelectionHitZone::INSIDE) beginSelectionMove(mousePixel);
		else
		{
			commitFloatingSelection();
			clearSelectionState();
		}
		return true;
	}
	if (button == PointerButton::Secondary)
	{
		commitFloatingSelection();
		clearSelectionState();
		return true;
	}
	return false;
}

void PaintEditor::updateSelectionTransformInteraction(const bbe::Vector2i &mousePixel, bool primaryDown)
{
	if (selection.rotationHandleActive && primaryDown) updateRotationDrag(mousePixel);
	if (selection.moveActive && primaryDown) updateSelectionMovePreview(mousePixel);
	if (selection.resizeActive && primaryDown) updateSelectionResizePreview(mousePixel);
}

void PaintEditor::prepareImageForCanvas(bbe::Image &image) const
{
	if (image.getWidth() <= 0 || image.getHeight() <= 0) return;
	image.keepAfterUpload();
	// Preview images may already be uploaded when drawn via PrimitiveBrush2D; setFilterMode
	// is illegal after upload (bbe::Error::AlreadyUploaded).
	if (!image.isLoadedGpu())
	{
		image.setFilterMode(bbe::ImageFilterMode::NEAREST);
	}
}

void PaintEditor::clearSelectionState()
{
	selection = {};
	rectangle = {};
	circle = {};
	line = {};
	arrow = {};
	bezier.controlPoints.clear();
	bezier.usesRightColor = false;
	bezier.dragPointIndex = -1;
}

void PaintEditor::selectWholeLayer()
{
	if (selection.floating)
	{
		commitFloatingSelection();
	}
	if (getCanvasWidth() <= 0 || getCanvasHeight() <= 0) return;

	clearSelectionState();
	mode = MODE_SELECTION;
	selection.hasSelection = true;
	selection.rect = bbe::Rectanglei(0, 0, getCanvasWidth(), getCanvasHeight());
}

int32_t PaintEditor::getCanvasWidth() const { return canvas.get().layers.isEmpty() ? 0 : canvas.get().layers[0].image.getWidth(); }

int32_t PaintEditor::getCanvasHeight() const { return canvas.get().layers.isEmpty() ? 0 : canvas.get().layers[0].image.getHeight(); }

void PaintEditor::clampActiveLayerIndex()
{
	if (canvas.get().layers.isEmpty())
	{
		activeLayerIndex = 0;
		return;
	}
	activeLayerIndex = bbe::Math::clamp(activeLayerIndex, 0, (int32_t)canvas.get().layers.getLength() - 1);
}

void PaintEditor::undo()
{
	canvas.undo();
	canvasGeneration--;
	clampActiveLayerIndex();
	clearSelectionState();
	clearWorkArea();
}

void PaintEditor::redo()
{
	canvas.redo();
	canvasGeneration++;
	clampActiveLayerIndex();
	clearSelectionState();
	clearWorkArea();
}

PaintLayer &PaintEditor::getActiveLayer()
{
	clampActiveLayerIndex();
	return canvas.get().layers[(size_t)activeLayerIndex];
}

const PaintLayer &PaintEditor::getActiveLayer() const
{
	if (canvas.get().layers.isEmpty()) bbe::Crash(bbe::Error::IllegalState);
	const int32_t clampedIndex = bbe::Math::clamp(activeLayerIndex, 0, (int32_t)canvas.get().layers.getLength() - 1);
	return canvas.get().layers[(size_t)clampedIndex];
}

bbe::Image &PaintEditor::getActiveLayerImage() { return getActiveLayer().image; }

const bbe::Image &PaintEditor::getActiveLayerImage() const { return getActiveLayer().image; }

void PaintEditor::prepareLayer(PaintLayer &layer) const { prepareImageForCanvas(layer.image); }

PaintLayer PaintEditor::makeLayer(const bbe::String &name, int32_t width, int32_t height, const bbe::Color &color) const
{
	PaintLayer layer;
	layer.name = name;
	layer.visible = true;
	layer.image = bbe::Image(width, height, color);
	prepareLayer(layer);
	return layer;
}

void PaintEditor::prepareDocumentImages()
{
	for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
	{
		prepareLayer(canvas.get().layers[i]);
	}
}

void PaintEditor::prepareForLayerTargetChange()
{
	commitFloatingSelection();
	clearSelectionState();
	clearWorkArea();
}

bool PaintEditor::isLayeredDocumentPath(const bbe::String &filePath) const { return filePath.toLowerCase().endsWith(LAYERED_FILE_EXTENSION); }

bool PaintEditor::isSupportedDroppedDocumentPath(const bbe::String &filePath) const
{
	const bbe::String lowerPath = filePath.toLowerCase();
	return lowerPath.endsWith(".png") || lowerPath.endsWith(LAYERED_FILE_EXTENSION);
}

bbe::Image PaintEditor::flattenVisibleLayers() const
{
	bbe::Image flattened(getCanvasWidth(), getCanvasHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
	prepareImageForCanvas(flattened);
	for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
	{
		const PaintLayer &layer = canvas.get().layers[i];
		if (!layer.visible) continue;
		flattened.blend(layer.image, layer.opacity, layer.blendMode);
	}
	return flattened;
}

bbe::Colori PaintEditor::getVisiblePixel(size_t x, size_t y) const
{
	bbe::Colori color(0, 0, 0, 0);
	for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
	{
		const PaintLayer &layer = canvas.get().layers[i];
		if (!layer.visible) continue;
		const bbe::Colori src = layer.image.getPixel(x, y);
		color = color.blendTo(src, layer.opacity, layer.blendMode);
	}
	return color;
}

bbe::String PaintEditor::makeLayerName() const
{
	return bbe::String("Layer ") + (canvas.get().layers.getLength() + 1);
}

void PaintEditor::addLayer()
{
	prepareForLayerTargetChange();
	canvas.get().layers.add(makeLayer(makeLayerName(), getCanvasWidth(), getCanvasHeight()));
	activeLayerIndex = (int32_t)canvas.get().layers.getLength() - 1;
	submitCanvas();
}

void PaintEditor::importFileAsLayers(const bbe::List<bbe::String> &paths)
{
	for (size_t i = 0; i < paths.getLength(); i++)
	{
		const bbe::String &path = paths[i];
		if (!isSupportedDroppedDocumentPath(path)) continue;

		if (isLayeredDocumentPath(path))
		{
			// Import every layer from the .bbepaint file
			bbe::ByteBuffer buffer;
			if (platform.readBinaryFile) buffer = platform.readBinaryFile(path);
			if (buffer.getLength() == 0) continue;
			bbe::ByteBufferSpan span = buffer.getSpan();
			const bbe::String magic = span.readNullString();
			const bool importIsV2 = (magic == LAYERED_FILE_MAGIC);
			const bool importIsV1 = (magic == LAYERED_FILE_MAGIC_V1);
			if (!importIsV2 && !importIsV1) continue;
			int32_t width = 0, height = 0;
			uint32_t layerCount = 0;
			int32_t storedActiveLayerIndex = 0;
			span.read(width);
			span.read(height);
			span.read(layerCount);
			span.read(storedActiveLayerIndex);
			if (width <= 0 || height <= 0 || layerCount == 0) continue;
			for (uint32_t k = 0; k < layerCount; k++)
			{
				PaintLayer layer;
				span.read(layer.visible);
				span.read(layer.name);
				if (importIsV2)
				{
					span.read(layer.opacity);
					uint8_t blendModeRaw = 0;
					span.read(blendModeRaw);
					layer.blendMode = (bbe::BlendMode)blendModeRaw;
				}
				if (!deserializeLayerImage(span, width, height, layer.image)) break;
				prepareForLayerTargetChange();
				canvas.get().layers.add(std::move(layer));
				activeLayerIndex = (int32_t)canvas.get().layers.getLength() - 1;
			}
		}
		else
		{
			// Import single PNG as a new layer
			bbe::Image img;
			if (platform.loadImageFile) img = platform.loadImageFile(path);
			if (img.getWidth() <= 0 || img.getHeight() <= 0) continue;
			prepareImageForCanvas(img);
			bbe::String name = std::filesystem::path(path.getRaw()).stem().string().c_str();
			prepareForLayerTargetChange();
			canvas.get().layers.add(PaintLayer{ name, true, 1.0f, bbe::BlendMode::Normal, std::move(img) });
			activeLayerIndex = (int32_t)canvas.get().layers.getLength() - 1;
		}
	}
	submitCanvas();
}

void PaintEditor::deleteActiveLayer()
{
	if (canvas.get().layers.getLength() <= 1) return;
	prepareForLayerTargetChange();
	canvas.get().layers.removeIndex((size_t)activeLayerIndex);
	clampActiveLayerIndex();
	submitCanvas();
}

void PaintEditor::moveActiveLayerUp()
{
	if ((size_t)activeLayerIndex + 1 >= canvas.get().layers.getLength()) return;
	prepareForLayerTargetChange();
	canvas.get().layers.swap((size_t)activeLayerIndex, (size_t)activeLayerIndex + 1);
	activeLayerIndex++;
	submitCanvas();
}

void PaintEditor::moveActiveLayerDown()
{
	if (activeLayerIndex <= 0) return;
	prepareForLayerTargetChange();
	canvas.get().layers.swap((size_t)activeLayerIndex, (size_t)activeLayerIndex - 1);
	activeLayerIndex--;
	submitCanvas();
}

void PaintEditor::duplicateActiveLayer()
{
	prepareForLayerTargetChange();
	PaintLayer dup = getActiveLayer();
	dup.name = dup.name + " Copy";
	canvas.get().layers.addAt((size_t)activeLayerIndex + 1, dup);
	activeLayerIndex++;
	submitCanvas();
}

void PaintEditor::mergeActiveLayerDown()
{
	if (activeLayerIndex <= 0) return;
	prepareForLayerTargetChange();
	PaintLayer &above = canvas.get().layers[(size_t)activeLayerIndex];
	PaintLayer &below = canvas.get().layers[(size_t)(activeLayerIndex - 1)];
	below.image.blend(above.image, above.opacity, above.blendMode);
	canvas.get().layers.removeIndex((size_t)activeLayerIndex);
	activeLayerIndex--;
	submitCanvas();
}

void PaintEditor::setActiveLayerIndex(int32_t newIndex)
{
	if (newIndex == activeLayerIndex) return;
	prepareForLayerTargetChange();
	activeLayerIndex = newIndex;
	clampActiveLayerIndex();
}

bbe::Vector2i PaintEditor::toCanvasPixel(const bbe::Vector2 &pos) const { return bbe::Vector2i((int32_t)bbe::Math::floor(pos.x), (int32_t)bbe::Math::floor(pos.y)); }

bbe::Vector2i PaintEditor::toTiledCanvasPixel(const bbe::Vector2 &pos)
{
	bbe::Vector2 p = pos;
	toTiledPos(p);
	return toCanvasPixel(p);
}

bool PaintEditor::clampRectToCanvas(const bbe::Rectanglei &rect, bbe::Rectanglei &outRect) const
{
	const int32_t left = bbe::Math::max<int32_t>(rect.x, 0);
	const int32_t top = bbe::Math::max<int32_t>(rect.y, 0);
	const int32_t right = bbe::Math::min<int32_t>(rect.x + rect.width - 1, getCanvasWidth() - 1);
	const int32_t bottom = bbe::Math::min<int32_t>(rect.y + rect.height - 1, getCanvasHeight() - 1);

	if (left > right || top > bottom)
	{
		outRect = {};
		return false;
	}

	outRect = bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1);
	return true;
}

bbe::Vector2i PaintEditor::constrainToSquare(const bbe::Vector2i &start, const bbe::Vector2i &end) const
{
	const int32_t dx = end.x - start.x;
	const int32_t dy = end.y - start.y;
	const int32_t size = bbe::Math::min(bbe::Math::abs(dx), bbe::Math::abs(dy));
	return bbe::Vector2i(
		start.x + (dx >= 0 ? size : -size),
		start.y + (dy >= 0 ? size : -size));
}

bool PaintEditor::buildSelectionRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
{
	const int32_t left = bbe::Math::min(pos1.x, pos2.x);
	const int32_t top = bbe::Math::min(pos1.y, pos2.y);
	const int32_t right = bbe::Math::max(pos1.x, pos2.x);
	const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
	return clampRectToCanvas(bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1), outRect);
}

bbe::Rectanglei PaintEditor::buildRawRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2) const
{
	const int32_t left = bbe::Math::min(pos1.x, pos2.x);
	const int32_t top = bbe::Math::min(pos1.y, pos2.y);
	const int32_t right = bbe::Math::max(pos1.x, pos2.x);
	const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
	return bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1);
}

bool PaintEditor::isPointInSelection(const bbe::Vector2i &point) const { return selection.hasSelection && selection.rect.isPointInRectangle(point, true); }

bool PaintEditor::isSelectionResizeHit(const SelectionHitZone hitZone) const { return bbe::editor::isResizeZone((bbe::editor::RectSelectionHitZone)hitZone); }

PaintEditor::SelectionHitZone PaintEditor::getSelectionHitZone(const bbe::Vector2i &point) const
{
	if (!selection.hasSelection)
	{
		return SelectionHitZone::NONE;
	}
	const bool allowRotationHandle = !selection.dragActive;
	const int32_t padding = bbe::Math::max<int32_t>(1, (int32_t)bbe::Math::ceil(6.0f / zoomLevel));
	const float rotationStemLenCanvas = 30.f / zoomLevel;
	const float rotationHitRadiusCanvas = 8.f / zoomLevel;
	return (SelectionHitZone)bbe::editor::hitTest(selection.rect, point, padding, allowRotationHandle, rotationStemLenCanvas, rotationHitRadiusCanvas);
}

bool PaintEditor::isWholeLayerSelection(const bbe::Rectanglei &rect) const { return rect.x == 0 && rect.y == 0 && rect.width == getCanvasWidth() && rect.height == getCanvasHeight(); }

bool PaintEditor::shouldClearWholeLayerSelectionToTransparency() const { return canvas.get().layers.getLength() > 1; }

bbe::Image PaintEditor::copyCanvasRect(const bbe::Rectanglei &rect) const
{
	bbe::Image copied(rect.width, rect.height, bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
	prepareImageForCanvas(copied);
	for (int32_t x = 0; x < rect.width; x++)
	{
		for (int32_t y = 0; y < rect.height; y++)
		{
			copied.setPixel((size_t)x, (size_t)y, getActiveLayerImage().getPixel((size_t)(rect.x + x), (size_t)(rect.y + y)));
		}
	}
	return copied;
}

void PaintEditor::clearCanvasRect(const bbe::Rectanglei &rect)
{
	const bool clearToTransparency = isWholeLayerSelection(rect) && shouldClearWholeLayerSelectionToTransparency();
	const bbe::Colori backgroundColor = clearToTransparency ? bbe::Colori(0, 0, 0, 0) : bbe::Color(rightColor).asByteColor();
	for (int32_t x = 0; x < rect.width; x++)
	{
		for (int32_t y = 0; y < rect.height; y++)
		{
			getActiveLayerImage().setPixel((size_t)(rect.x + x), (size_t)(rect.y + y), backgroundColor);
		}
	}
}

void PaintEditor::storeSelectionInClipboard()
{
	if (!selection.hasSelection) return;
	selection.clipboard = selection.floating ? selection.floatingImage : copyCanvasRect(selection.rect);
	prepareImageForCanvas(selection.clipboard);
	if (platform.supportsClipboardImages && platform.setClipboardImage && platform.supportsClipboardImages())
	{
		platform.setClipboardImage(selection.clipboard);
	}
}

void PaintEditor::deleteSelection()
{
	if (!selection.hasSelection) return;
	if (selection.floating)
	{
		clearSelectionState();
		return;
	}
	clearCanvasRect(selection.rect);
	submitCanvas();
	clearSelectionState();
}

void PaintEditor::cutSelection()
{
	if (!selection.hasSelection) return;
	storeSelectionInClipboard();
	if (selection.floating)
	{
		clearSelectionState();
		return;
	}
	deleteSelection();
}

bool PaintEditor::getPasteImage(bbe::Image &image)
{
	if (platform.supportsClipboardImages && platform.isClipboardImageAvailable && platform.getClipboardImage &&
		platform.supportsClipboardImages() && platform.isClipboardImageAvailable())
	{
		image = platform.getClipboardImage();
		prepareImageForCanvas(image);
		return image.getWidth() > 0 && image.getHeight() > 0;
	}

	if (selection.clipboard.getWidth() > 0 && selection.clipboard.getHeight() > 0)
	{
		image = selection.clipboard;
		prepareImageForCanvas(image);
		return true;
	}

	return false;
}

void PaintEditor::pasteSelectionAt(const bbe::Vector2i &pos)
{
	bbe::Image image;
	if (!getPasteImage(image)) return;

	if (selection.floating)
	{
		commitFloatingSelection();
	}

	const bbe::Vector2i clampedPos(
		bbe::Math::clamp(pos.x, 0, bbe::Math::max(getCanvasWidth()  - image.getWidth(),  0)),
		bbe::Math::clamp(pos.y, 0, bbe::Math::max(getCanvasHeight() - image.getHeight(), 0))
	);

	const int32_t neededW = clampedPos.x + image.getWidth();
	const int32_t neededH = clampedPos.y + image.getHeight();
	const int32_t newW = bbe::Math::max(getCanvasWidth(), neededW);
	const int32_t newH = bbe::Math::max(getCanvasHeight(), neededH);
	if (newW > getCanvasWidth() || newH > getCanvasHeight())
	{
		const bbe::Color fillColor(rightColor[0], rightColor[1], rightColor[2], rightColor[3]);
		for (size_t li = 0; li < canvas.get().layers.getLength(); li++)
		{
			canvas.get().layers[li].image = canvas.get().layers[li].image.resizedCanvas(
				newW, newH, bbe::Vector2i(0, 0), fillColor);
			prepareImageForCanvas(canvas.get().layers[li].image);
		}
		clearWorkArea();
		submitCanvas();
	}

	mode = MODE_SELECTION;
	selection.hasSelection = true;
	selection.floating = true;
	selection.floatingImage = image;
	selection.pastedFromClipboard = true;
	selection.rect = bbe::Rectanglei(clampedPos.x, clampedPos.y, image.getWidth(), image.getHeight());
	rectangle.draftActive = false;
	rectangle.draftUsesRightColor = false;
	selection.moveActive = false;
	selection.resizeActive = false;
	selection.dragActive = false;
	selection.previewRect = {};
	selection.previewImage = {};
}

void PaintEditor::commitFloatingSelection()
{
	if (!selection.floating) return;

	const bool applySymmetry = rectangle.draftActive || circle.draftActive || selection.pastedFromClipboard;

	if (std::abs(selection.rotation) > 0.0001f)
	{
		// AA-off shapes: re-render directly from SDF with rotation baked in.
		// This avoids both gaps and thickness changes that image-rotation sampling produces.
		if (!antiAliasingEnabled && (rectangle.draftActive || circle.draftActive) && std::abs(selection.rotation) > 0.01f)
		{
			const bbe::Colori color = rectangle.draftActive ? getRectangleDraftColor() : getCircleDraftColor();
			const bbe::Vector2 center = {
				selection.rect.x + selection.rect.width * 0.5f,
				selection.rect.y + selection.rect.height * 0.5f
			};

			auto blitRotated = [&](float rot, const bbe::Vector2 &c)
			{
				const bbe::Image img = rectangle.draftActive
										   ? createRectangleImage(selection.rect.width, selection.rect.height, color, rot, rectangle.draftUsesRightColor)
										   : createCircleImage(selection.rect.width, selection.rect.height, color, rot, circle.draftUsesRightColor);
				const bbe::Vector2i pos(
					(int32_t)std::floor(c.x - img.getWidth() * 0.5f),
					(int32_t)std::floor(c.y - img.getHeight() * 0.5f));
				getActiveLayerImage().blendOver(img, pos, tiled);
			};

			blitRotated(selection.rotation, center);
			const auto symCenters = getSymmetryPositions(center);
			const auto symAngles = getSymmetryRotationAngles();
			for (size_t i = 1; i < symCenters.getLength(); i++)
				blitRotated(selection.rotation + symAngles[i], symCenters[i]);

			submitCanvas();
			clearSelectionState();
			return;
		}

		getActiveLayerImage().blendOverRotated(selection.floatingImage, selection.rect, selection.rotation, tiled, antiAliasingEnabled);
		if (applySymmetry)
		{
			const bbe::Vector2 center = {
				selection.rect.x + selection.rect.width * 0.5f,
				selection.rect.y + selection.rect.height * 0.5f
			};
			const auto symCenters = getSymmetryPositions(center);
			const auto symAngles = getSymmetryRotationAngles();
			for (size_t i = 1; i < symCenters.getLength(); i++)
			{
				const bbe::Rectanglei symRect = {
					(int32_t)std::round(symCenters[i].x - selection.rect.width * 0.5f),
					(int32_t)std::round(symCenters[i].y - selection.rect.height * 0.5f),
					selection.rect.width,
					selection.rect.height
				};
				getActiveLayerImage().blendOverRotated(selection.floatingImage, symRect, selection.rotation + symAngles[i], tiled, antiAliasingEnabled);
			}
		}
		submitCanvas();
		clearSelectionState();
		return;
	}

	getActiveLayerImage().blendOver(selection.floatingImage, selection.rect.getPos(), tiled);
	if (applySymmetry)
	{
		const bbe::Vector2 center = {
			selection.rect.x + selection.rect.width * 0.5f,
			selection.rect.y + selection.rect.height * 0.5f
		};
		const auto symCenters = getSymmetryPositions(center);
		const auto symAngles = getSymmetryRotationAngles();
		for (size_t i = 1; i < symCenters.getLength(); i++)
		{
			const bbe::Rectanglei symRect = {
				(int32_t)std::round(symCenters[i].x - selection.rect.width * 0.5f),
				(int32_t)std::round(symCenters[i].y - selection.rect.height * 0.5f),
				selection.rect.width,
				selection.rect.height
			};
			if (std::abs(symAngles[i]) > 0.0001f)
				getActiveLayerImage().blendOverRotated(selection.floatingImage, symRect, symAngles[i], tiled, antiAliasingEnabled);
			else
				getActiveLayerImage().blendOver(selection.floatingImage, symRect.getPos(), tiled);
		}
	}
	submitCanvas();

	bbe::Vector2i displayPos = selection.rect.getPos();
	if (tiled)
	{
		displayPos.x = bbe::Math::mod<int32_t>(displayPos.x, getCanvasWidth());
		displayPos.y = bbe::Math::mod<int32_t>(displayPos.y, getCanvasHeight());
	}

	bbe::Rectanglei clampedRect;
	if (!clampRectToCanvas(bbe::Rectanglei(displayPos.x, displayPos.y, selection.floatingImage.getWidth(), selection.floatingImage.getHeight()), clampedRect))
	{
		clearSelectionState();
		return;
	}

	selection.rect = clampedRect;
	selection.floating = false;
	selection.floatingImage = {};
	rectangle.draftActive = false;
	rectangle.draftUsesRightColor = false;
	circle.draftActive = false;
	circle.draftUsesRightColor = false;
}

void PaintEditor::applySelectionWhenLeavingTool()
{
	bbe::Image savedClipboard = std::move(selection.clipboard);

	if (selection.rotationHandleActive)
		selection.rotationHandleActive = false;
	if (selection.moveActive || selection.resizeActive)
		applySelectionTransform();

	if (selection.dragActive)
	{
		if (hasPointerPos)
			selection.hasSelection = buildSelectionRect(selection.dragStart, toCanvasPixel(lastPointerCanvasPos), selection.rect);
		selection.dragActive = false;
		selection.previewRect = {};
	}

	if (selection.floating)
		commitFloatingSelection();

	selection = SelectionState{};
	selection.clipboard = std::move(savedClipboard);
}

bool PaintEditor::toImagePos(bbe::Vector2 &pos, int32_t width, int32_t height, bool repeated) const
{
	if (repeated)
	{
		pos.x = bbe::Math::mod<float>(pos.x, width);
		pos.y = bbe::Math::mod<float>(pos.y, height);
		return true;
	}

	return pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height;
}

void PaintEditor::drawArrowToWorkArea(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color)
{
	workArea.drawArrow(from, to, color, brushWidth, arrowHeadSize, arrowHeadWidth, arrowDoubleHeaded, arrowFilledHead, tiled, antiAliasingEnabled);
}

bbe::Colori PaintEditor::getLineDraftColor() const { return getColor(line.draftUsesRightColor); }

bbe::Colori PaintEditor::getArrowDraftColor() const { return getColor(arrow.draftUsesRightColor); }

void PaintEditor::redrawLineDraft()
{
	if (!line.draftActive) return;
	clearWorkArea();
	touchLineSymmetry(line.end, line.start, getLineDraftColor(), brushWidth);
}

void PaintEditor::redrawArrowDraft()
{
	if (!arrow.draftActive) return;
	clearWorkArea();
	drawArrowSymmetry(arrow.start, arrow.end, getArrowDraftColor());
}

void PaintEditor::redrawBezierDraft()
{
	if (bezier.controlPoints.isEmpty()) return;
	clearWorkArea();
	drawBezierSymmetry(bezier.controlPoints, getBezierColor());
}

void PaintEditor::refreshBrushBasedDrafts()
{
	if (rectangle.draftActive) refreshActiveRectangleDraftImage();
	if (circle.draftActive) refreshActiveCircleDraftImage();
	redrawLineDraft();
	redrawArrowDraft();
	redrawBezierDraft();
}

void PaintEditor::finalizeLineDraft()
{
	finalizeEndpointDraft(line.draftActive, line.dragEndpoint);
}

void PaintEditor::finalizeArrowDraft()
{
	finalizeEndpointDraft(arrow.draftActive, arrow.dragEndpoint);
}

bbe::Colori PaintEditor::getBezierColor() const { return getColor(bezier.usesRightColor); }

void PaintEditor::drawBezierToWorkArea(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color)
{
	workArea.drawBezier(points, color, brushWidth, tiled, antiAliasingEnabled);
}

void PaintEditor::finalizeBezierDraft()
{
	if (bezier.controlPoints.getLength() >= 2)
	{
		clearWorkArea();
		drawBezierSymmetry(bezier.controlPoints, getBezierColor());
		applyWorkArea();
		submitCanvas();
	}
	else
	{
		clearWorkArea();
	}
	bezier.controlPoints.clear();
	bezier.dragPointIndex = -1;
}

bbe::Colori PaintEditor::getRectangleDraftColor() const { return getColor(rectangle.draftUsesRightColor); }

bbe::Colori PaintEditor::getRectangleDragColor() const { return getColor(rectangle.dragUsesRightColor); }

int32_t PaintEditor::getRectangleDraftPadding() const { return 0; }

bbe::Rectanglei PaintEditor::expandRectangleRect(const bbe::Rectanglei &rect) const
{
	const int32_t padding = getRectangleDraftPadding();
	return bbe::Rectanglei(
		rect.x - padding,
		rect.y - padding,
		rect.width + padding * 2,
		rect.height + padding * 2);
}

bool PaintEditor::buildRectangleDraftRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
{
	bbe::Rectanglei baseRect;
	if (tiled)
	{
		baseRect = buildRawRect(pos1, pos2);
		if (baseRect.width <= 0 || baseRect.height <= 0)
		{
			outRect = {};
			return false;
		}
	}
	else if (!buildSelectionRect(pos1, pos2, baseRect))
	{
		outRect = {};
		return false;
	}

	outRect = expandRectangleRect(baseRect);
	return true;
}

bbe::Image PaintEditor::createRectangleImage(int32_t width, int32_t height, const bbe::Colori &strokeColor, float rotation, bool strokeUsesRightColor) const
{
	if (shapeFillWithSecondary)
	{
		const bbe::Colori fillColor = getColor(!strokeUsesRightColor);
		bbe::Image img = bbe::Image::strokedRoundedRect(width, height, fillColor, 0, cornerRadius, rotation, antiAliasingEnabled);
		bbe::Image stroke = bbe::Image::strokedRoundedRect(width, height, strokeColor, brushWidth, cornerRadius, rotation, antiAliasingEnabled);
		img.blend(stroke, 1.0f, bbe::BlendMode::Normal);
		return img;
	}
	return bbe::Image::strokedRoundedRect(width, height, strokeColor, brushWidth, cornerRadius, rotation, antiAliasingEnabled);
}

bbe::Image PaintEditor::createRectangleDraftImage(int32_t width, int32_t height) const { return createRectangleImage(width, height, getRectangleDraftColor(), 0.f, rectangle.draftUsesRightColor); }

bbe::Image PaintEditor::createRectangleDragPreviewImage(int32_t width, int32_t height) const { return createRectangleImage(width, height, getRectangleDragColor(), 0.f, rectangle.dragUsesRightColor); }

void PaintEditor::refreshActiveRectangleDraftImage()
{
	refreshActiveShapeDraftImage(rectangle.draftActive, [&](int32_t width, int32_t height)
								 { return createRectangleDraftImage(width, height); });
}

bbe::Colori PaintEditor::getCircleDraftColor() const { return getColor(circle.draftUsesRightColor); }

bbe::Colori PaintEditor::getCircleDragColor() const { return getColor(circle.dragUsesRightColor); }

bbe::Image PaintEditor::createCircleImage(int32_t width, int32_t height, const bbe::Colori &strokeColor, float rotation, bool strokeUsesRightColor) const
{
	if (shapeFillWithSecondary)
	{
		const bbe::Colori fillColor = getColor(!strokeUsesRightColor);
		bbe::Image img = bbe::Image::strokedEllipse(width, height, fillColor, 0, rotation, antiAliasingEnabled);
		bbe::Image stroke = bbe::Image::strokedEllipse(width, height, strokeColor, brushWidth, rotation, antiAliasingEnabled);
		img.blend(stroke, 1.0f, bbe::BlendMode::Normal);
		return img;
	}
	return bbe::Image::strokedEllipse(width, height, strokeColor, brushWidth, rotation, antiAliasingEnabled);
}

bbe::Image PaintEditor::createCircleDraftImage(int32_t width, int32_t height) const { return createCircleImage(width, height, getCircleDraftColor(), 0.f, circle.draftUsesRightColor); }

bbe::Image PaintEditor::createCircleDragPreviewImage(int32_t width, int32_t height) const { return createCircleImage(width, height, getCircleDragColor(), 0.f, circle.dragUsesRightColor); }

void PaintEditor::refreshActiveCircleDraftImage()
{
	refreshActiveShapeDraftImage(circle.draftActive, [&](int32_t width, int32_t height)
								 { return createCircleDraftImage(width, height); });
}

void PaintEditor::finalizeCircleDrag(const bbe::Vector2i &mousePixel, bool shiftDown)
{
	finalizeShapeDrag(circle.dragActive, circle.draftActive, circle.draftUsesRightColor, circle.dragUsesRightColor, circle.dragStart, mousePixel, circle.dragPreviewRect, circle.dragPreviewImage, shiftDown, [&](const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect)
					  {
			if (tiled)
			{
				outRect = buildRawRect(pos1, pos2);
				return outRect.width > 0 && outRect.height > 0;
			}
			return buildSelectionRect(pos1, pos2, outRect) && outRect.width > 0 && outRect.height > 0; }, [&](int32_t width, int32_t height, const bbe::Colori &color)
					  { return createCircleImage(width, height, color, 0.f, circle.dragUsesRightColor); });
}

void PaintEditor::beginCircleDrag(const bbe::Vector2i &mousePixel, bool useRightColor)
{
	circle.dragActive = true;
	circle.dragUsesRightColor = useRightColor;
	circle.dragStart = mousePixel;
	circle.dragPreviewRect = {};
	circle.dragPreviewImage = {};
}

void PaintEditor::updateCircleDragPreview(const bbe::Vector2i &mousePixel, bool shiftDown)
{
	updateShapeDragPreview(circle.dragStart, mousePixel, circle.dragPreviewRect, circle.dragPreviewImage, shiftDown, [&](const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect)
						   {
			if (tiled)
			{
				outRect = buildRawRect(pos1, pos2);
				return outRect.width > 0 && outRect.height > 0;
			}
			return buildSelectionRect(pos1, pos2, outRect); }, [&](int32_t width, int32_t height)
						   { return createCircleDragPreviewImage(width, height); });
}

void PaintEditor::beginSelectionMove(const bbe::Vector2i &mousePixel)
{
	selection.moveActive = true;
	selection.moveOffset = mousePixel - selection.rect.getPos();
	selection.interactionStartRect = selection.rect;
	selection.previewRect = selection.rect;
	selection.previewImage = selection.floating ? selection.floatingImage : copyCanvasRect(selection.rect);
	// Must retain CPU pixels after GPU draw (see OpenGLImage ctor); blendOver needs isLoadedCpu().
	prepareImageForCanvas(selection.previewImage);
}

void PaintEditor::beginRotationDrag(const bbe::Vector2i &mousePixel)
{
	if (!selection.floating && selection.hasSelection)
	{
		selection.floatingImage = copyCanvasRect(selection.rect);
		prepareImageForCanvas(selection.floatingImage);
		clearCanvasRect(selection.rect);
		selection.floating = true;
		submitCanvas();
	}

	selection.rotationHandleActive = true;
	selection.rotationDragPivot = {
		selection.rect.x + selection.rect.width / 2.f,
		selection.rect.y + selection.rect.height / 2.f
	};
	const bbe::Vector2 toMouse((float)mousePixel.x - selection.rotationDragPivot.x, (float)mousePixel.y - selection.rotationDragPivot.y);
	selection.rotationDragStartAngle = toMouse.getLength() > 0.001f ? toMouse.getAngle() : 0.f;
	selection.rotationDragBaseAngle = selection.rotation;
}

void PaintEditor::updateRotationDrag(const bbe::Vector2i &mousePixel)
{
	const bbe::Vector2 toMouse((float)mousePixel.x - selection.rotationDragPivot.x, (float)mousePixel.y - selection.rotationDragPivot.y);
	if (toMouse.getLength() > 0.001f)
	{
		selection.rotation = selection.rotationDragBaseAngle + (toMouse.getAngle() - selection.rotationDragStartAngle);
	}
}

void PaintEditor::updateSelectionMovePreview(const bbe::Vector2i &mousePixel)
{
	selection.previewRect = bbe::Rectanglei(
		mousePixel.x - selection.moveOffset.x,
		mousePixel.y - selection.moveOffset.y,
		selection.previewImage.getWidth(),
		selection.previewImage.getHeight());
}

void PaintEditor::beginSelectionResize(const SelectionHitZone hitZone)
{
	selection.resizeActive = true;
	selection.resizeZone = hitZone;
	selection.interactionStartRect = selection.rect;
	selection.previewRect = selection.rect;
	selection.previewImage = selection.floating ? selection.floatingImage : copyCanvasRect(selection.rect);
}

void PaintEditor::updateSelectionResizePreview(const bbe::Vector2i &mousePixel)
{
	const int32_t originalLeft = selection.interactionStartRect.x;
	const int32_t originalTop = selection.interactionStartRect.y;
	const int32_t originalRight = selection.interactionStartRect.x + selection.interactionStartRect.width - 1;
	const int32_t originalBottom = selection.interactionStartRect.y + selection.interactionStartRect.height - 1;

	int32_t left = originalLeft;
	int32_t top = originalTop;
	int32_t right = originalRight;
	int32_t bottom = originalBottom;

	switch (selection.resizeZone)
	{
	case SelectionHitZone::LEFT:
	case SelectionHitZone::TOP_LEFT:
	case SelectionHitZone::BOTTOM_LEFT:
		left = mousePixel.x;
		break;
	default:
		break;
	}
	switch (selection.resizeZone)
	{
	case SelectionHitZone::RIGHT:
	case SelectionHitZone::TOP_RIGHT:
	case SelectionHitZone::BOTTOM_RIGHT:
		right = mousePixel.x;
		break;
	default:
		break;
	}
	switch (selection.resizeZone)
	{
	case SelectionHitZone::TOP:
	case SelectionHitZone::TOP_LEFT:
	case SelectionHitZone::TOP_RIGHT:
		top = mousePixel.y;
		break;
	default:
		break;
	}
	switch (selection.resizeZone)
	{
	case SelectionHitZone::BOTTOM:
	case SelectionHitZone::BOTTOM_LEFT:
	case SelectionHitZone::BOTTOM_RIGHT:
		bottom = mousePixel.y;
		break;
	default:
		break;
	}

	selection.previewRect = buildRawRect({ left, top }, { right, bottom });
	if (rectangle.draftActive)
	{
		refreshActiveRectangleDraftImage();
	}
}

bbe::Image PaintEditor::buildSelectionPreviewResultImage() const
{
	if (rectangle.draftActive)
	{
		return createRectangleDraftImage(selection.previewRect.width, selection.previewRect.height);
	}

	if (circle.draftActive)
	{
		return createCircleDraftImage(selection.previewRect.width, selection.previewRect.height);
	}

	if (selection.previewRect.width != selection.previewImage.getWidth() || selection.previewRect.height != selection.previewImage.getHeight())
	{
		return selection.previewImage.scaledNearest(selection.previewRect.width, selection.previewRect.height);
	}

	return selection.previewImage;
}

void PaintEditor::clearSelectionInteractionState()
{
	selection.moveActive = false;
	selection.moveOffset = {};
	selection.resizeActive = false;
	selection.resizeZone = SelectionHitZone::NONE;
	selection.interactionStartRect = {};
	selection.previewRect = {};
	selection.previewImage = {};
}

void PaintEditor::applySelectionTransform()
{
	if (!selection.moveActive && !selection.resizeActive) return;

	const bool rectChanged = selection.previewRect.x != selection.rect.x || selection.previewRect.y != selection.rect.y || selection.previewRect.width != selection.rect.width || selection.previewRect.height != selection.rect.height;

	if (selection.floating)
	{
		if (rectChanged)
		{
			selection.rect = selection.previewRect;
			selection.floatingImage = buildSelectionPreviewResultImage();
			prepareImageForCanvas(selection.floatingImage);
		}
		clearSelectionInteractionState();
		return;
	}

	if (rectChanged)
	{
		clearCanvasRect(selection.rect);
		selection.rect = selection.previewRect;
		selection.floating = true;
		selection.floatingImage = buildSelectionPreviewResultImage();
		prepareImageForCanvas(selection.floatingImage);
		rectangle.draftActive = false;
		rectangle.draftUsesRightColor = false;
		circle.draftActive = false;
		circle.draftUsesRightColor = false;
		selection.hasSelection = true;
	}

	clearSelectionInteractionState();
}


void PaintEditor::finalizeRectangleDrag(const bbe::Vector2i &mousePixel, bool shiftDown)
{
	finalizeShapeDrag(rectangle.dragActive, rectangle.draftActive, rectangle.draftUsesRightColor, rectangle.dragUsesRightColor, rectangle.dragStart, mousePixel, rectangle.dragPreviewRect, rectangle.dragPreviewImage, shiftDown, [&](const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect)
					  { return buildRectangleDraftRect(pos1, pos2, outRect); }, [&](int32_t width, int32_t height, const bbe::Colori &color)
					  { return createRectangleImage(width, height, color, 0.f, rectangle.dragUsesRightColor); });
}

void PaintEditor::beginRectangleDrag(const bbe::Vector2i &mousePixel, bool useRightColor)
{
	rectangle.dragActive = true;
	rectangle.dragUsesRightColor = useRightColor;
	rectangle.dragStart = mousePixel;
	rectangle.dragPreviewRect = {};
	rectangle.dragPreviewImage = {};
}

void PaintEditor::updateRectangleDragPreview(const bbe::Vector2i &mousePixel, bool shiftDown)
{
	updateShapeDragPreview(rectangle.dragStart, mousePixel, rectangle.dragPreviewRect, rectangle.dragPreviewImage, shiftDown, [&](const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect)
						   { return buildRectangleDraftRect(pos1, pos2, outRect); }, [&](int32_t width, int32_t height)
						   { return createRectangleDragPreviewImage(width, height); });
}

void PaintEditor::updateFloatingShapePreview(ShapeDragState &shape, bool isCircle, const bbe::Vector2i &mousePixel)
{
	const bool shiftDown = constrainSquareEnabled;
	if (isCircle)
		updateCircleDragPreview(mousePixel, shiftDown);
	else
		updateRectangleDragPreview(mousePixel, shiftDown);
}

void PaintEditor::finalizeFloatingShapeDrag(ShapeDragState &shape, bool isCircle, const bbe::Vector2i &mousePixel)
{
	const bool shiftDown = constrainSquareEnabled;
	if (isCircle)
		finalizeCircleDrag(mousePixel, shiftDown);
	else
		finalizeRectangleDrag(mousePixel, shiftDown);
}

bbe::Rectangle PaintEditor::selectionRectToScreen(const bbe::Rectanglei &rect) const
{
	return bbe::Rectangle(
		offset.x + rect.x * zoomLevel,
		offset.y + rect.y * zoomLevel,
		rect.width * zoomLevel,
		rect.height * zoomLevel);
}

bbe::Vector2 PaintEditor::getCanvasHandleScreenPos(int32_t i) const
{
	const float W = getCanvasWidth() * zoomLevel;
	const float H = getCanvasHeight() * zoomLevel;
	switch (i)
	{
	case 0:
		return { offset.x, offset.y };
	case 1:
		return { offset.x + W / 2.f, offset.y };
	case 2:
		return { offset.x + W, offset.y };
	case 3:
		return { offset.x + W, offset.y + H / 2.f };
	case 4:
		return { offset.x + W, offset.y + H };
	case 5:
		return { offset.x + W / 2.f, offset.y + H };
	case 6:
		return { offset.x, offset.y + H };
	case 7:
		return { offset.x, offset.y + H / 2.f };
	default:
		return offset;
	}
}

int32_t PaintEditor::getCanvasResizeHitHandle(const bbe::Vector2 &screenPos) const
{
	if (getCanvasWidth() <= 0 || getCanvasHeight() <= 0) return -1;
	constexpr float hitRadius = 6.f;
	for (int32_t i = 0; i < 8; i++)
	{
		const bbe::Vector2 hp = getCanvasHandleScreenPos(i);
		const float dx = screenPos.x - hp.x;
		const float dy = screenPos.y - hp.y;
		if (dx * dx + dy * dy <= hitRadius * hitRadius) return i;
	}
	return -1;
}

void PaintEditor::updateCanvasResizePreview(const bbe::Vector2 &canvasMousePos)
{
	const int32_t W = getCanvasWidth();
	const int32_t H = getCanvasHeight();
	const int32_t mx = (int32_t)std::round(canvasMousePos.x);
	const int32_t my = (int32_t)std::round(canvasMousePos.y);
	switch (canvasResizeHandleIndex)
	{
	case 0:
		canvasResizePreviewRect = { mx, my, std::max(1, W - mx), std::max(1, H - my) };
		break;
	case 1:
		canvasResizePreviewRect = { 0, my, W, std::max(1, H - my) };
		break;
	case 2:
		canvasResizePreviewRect = { 0, my, std::max(1, mx), std::max(1, H - my) };
		break;
	case 3:
		canvasResizePreviewRect = { 0, 0, std::max(1, mx), H };
		break;
	case 4:
		canvasResizePreviewRect = { 0, 0, std::max(1, mx), std::max(1, my) };
		break;
	case 5:
		canvasResizePreviewRect = { 0, 0, W, std::max(1, my) };
		break;
	case 6:
		canvasResizePreviewRect = { mx, 0, std::max(1, W - mx), std::max(1, my) };
		break;
	case 7:
		canvasResizePreviewRect = { mx, 0, std::max(1, W - mx), H };
		break;
	default:
		break;
	}
}

void PaintEditor::applyCanvasResize(const bbe::Rectanglei &previewRect)
{
	if (previewRect.width <= 0 || previewRect.height <= 0) return;
	if (canvas.get().layers.isEmpty()) return;

	const bbe::Color fillColor(rightColor[0], rightColor[1], rightColor[2], rightColor[3]);
	const int32_t oldW = getCanvasWidth();
	const int32_t oldH = getCanvasHeight();

	for (size_t li = 0; li < canvas.get().layers.getLength(); li++)
	{
		canvas.get().layers[li].image = canvas.get().layers[li].image.resizedCanvas(
			previewRect.width,
			previewRect.height,
			bbe::Vector2i(-previewRect.x, -previewRect.y),
			fillColor);
		prepareImageForCanvas(canvas.get().layers[li].image);
	}

	// Shift offset so visual position of original content is preserved.
	offset.x += previewRect.x * zoomLevel;
	offset.y += previewRect.y * zoomLevel;

	clearSelectionState();
	clearWorkArea();
	submitCanvas();
}

void PaintEditor::clampBrushWidth()
{
	if (brushWidth < 1) brushWidth = 1;
}

void PaintEditor::clampTextFontSize()
{
	if (textFontSize < 1) textFontSize = 1;
}

void PaintEditor::clampTextFontIndex()
{
	if (availableFonts.isEmpty())
	{
		textFontIndex = 0;
		return;
	}
	textFontIndex = bbe::Math::clamp(textFontIndex, 0, (int32_t)availableFonts.getLength() - 1);
}

void PaintEditor::buildAvailableFontList()
{
	availableFonts.clear();
	if (!platform.findSystemFonts) return;
	availableFonts = platform.findSystemFonts("Text");
}

const bbe::Image &PaintEditor::getTextGlyphImage(const bbe::Font &font, int32_t codePoint) const
{
	bbe::Image &glyph = const_cast<bbe::Image &>(font.getImage(codePoint, 1.0f));
	glyph.keepAfterUpload();
	return glyph;
}

bbe::String PaintEditor::getTextBufferString() const
{
	return bbe::String(textBuffer);
}

bool PaintEditor::getTextOriginAndBounds(const bbe::Vector2i &topLeft, bbe::Vector2 &outOrigin, bbe::Rectangle &outBounds) const
{
	const bbe::String text = getTextBufferString();
	if (text.isEmpty()) return false;
	return getTextToolFont().getRasterOriginAndBounds(text, topLeft, outOrigin, outBounds);
}

bbe::Image PaintEditor::renderTextToImage(const bbe::Vector2i &topLeft, const bbe::Colori &color) const
{
	const bbe::String text = getTextBufferString();
	const bbe::Font &font = getTextToolFont();
	return bbe::Image::renderTextToImage(font, text, topLeft, color, antiAliasingEnabled);
}

bool PaintEditor::drawTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color)
{
	const bbe::String text = getTextBufferString();
	if (text.isEmpty()) return false;
	const bbe::Font &font = getTextToolFont();
	getActiveLayerImage().blendText(font, text, topLeft, color, tiled, antiAliasingEnabled);
	return true;
}

void PaintEditor::placeTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color)
{
	bool changed = drawTextAt(topLeft, color);
	if (changed)
	{
		submitCanvas();
	}
}

void PaintEditor::swapColors()
{
	for (size_t i = 0; i < std::size(leftColor); i++)
	{
		std::swap(leftColor[i], rightColor[i]);
	}
}

void PaintEditor::resetColorsToDefault()
{
	leftColor[0] = 0.0f;
	leftColor[1] = 0.0f;
	leftColor[2] = 0.0f;
	leftColor[3] = 1.0f;

	rightColor[0] = 1.0f;
	rightColor[1] = 1.0f;
	rightColor[2] = 1.0f;
	rightColor[3] = 1.0f;
}

void PaintEditor::serializeLayerImage(const bbe::Image &image, bbe::ByteBuffer &buffer) const
{
	for (int32_t y = 0; y < image.getHeight(); y++)
	{
		for (int32_t x = 0; x < image.getWidth(); x++)
		{
			const bbe::Colori color = image.getPixel((size_t)x, (size_t)y);
			uint8_t r = color.r;
			uint8_t g = color.g;
			uint8_t b = color.b;
			uint8_t a = color.a;
			buffer.write(r);
			buffer.write(g);
			buffer.write(b);
			buffer.write(a);
		}
	}
}

bool PaintEditor::deserializeLayerImage(bbe::ByteBufferSpan &span, int32_t width, int32_t height, bbe::Image &outImage) const
{
	if (width <= 0 || height <= 0) return false;
	outImage = bbe::Image(width, height, bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
	prepareImageForCanvas(outImage);

	for (int32_t y = 0; y < height; y++)
	{
		for (int32_t x = 0; x < width; x++)
		{
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;
			uint8_t a = 0;
			span.read(r);
			span.read(g);
			span.read(b);
			span.read(a);
			outImage.setPixel((size_t)x, (size_t)y, bbe::Colori(r, g, b, a));
		}
	}

	return true;
}

bbe::ByteBuffer PaintEditor::serializeLayeredDocumentBytes() const
{
	bbe::ByteBuffer buffer;
	buffer.writeNullString(LAYERED_FILE_MAGIC);

	int32_t width = getCanvasWidth();
	int32_t height = getCanvasHeight();
	uint32_t layerCount = (uint32_t)canvas.get().layers.getLength();
	int32_t storedActiveLayerIndex = activeLayerIndex;
	buffer.write(width);
	buffer.write(height);
	buffer.write(layerCount);
	buffer.write(storedActiveLayerIndex);

	for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
	{
		const PaintLayer &layer = canvas.get().layers[i];
		bool visible = layer.visible;
		bbe::String name = layer.name;
		float opacity = layer.opacity;
		uint8_t blendModeRaw = (uint8_t)layer.blendMode;
		buffer.write(visible);
		buffer.write(name);
		buffer.write(opacity);
		buffer.write(blendModeRaw);
		serializeLayerImage(layer.image, buffer);
	}
	return buffer;
}

bool PaintEditor::deserializeLayeredDocumentBytes(bbe::ByteBufferSpan span, PaintDocument &outDocument, int32_t &outStoredActiveLayerIndex) const
{
	const bbe::String magic = span.readNullString();
	const bool isV2 = (magic == LAYERED_FILE_MAGIC);
	const bool isV1 = (magic == LAYERED_FILE_MAGIC_V1);
	if (!isV2 && !isV1) return false;

	int32_t width = 0;
	int32_t height = 0;
	uint32_t layerCount = 0;
	int32_t storedActiveLayerIndex = 0;
	span.read(width);
	span.read(height);
	span.read(layerCount);
	span.read(storedActiveLayerIndex);
	if (width <= 0 || height <= 0 || layerCount == 0) return false;

	PaintDocument document;
	for (uint32_t i = 0; i < layerCount; i++)
	{
		PaintLayer layer;
		span.read(layer.visible);
		span.read(layer.name);
		if (isV2)
		{
			span.read(layer.opacity);
			uint8_t blendModeRaw = 0;
			span.read(blendModeRaw);
			layer.blendMode = (bbe::BlendMode)blendModeRaw;
		}
		if (!deserializeLayerImage(span, width, height, layer.image))
		{
			return false;
		}
		document.layers.add(std::move(layer));
	}

	outDocument = std::move(document);
	outStoredActiveLayerIndex = storedActiveLayerIndex;
	return true;
}

bool PaintEditor::saveLayeredDocument(const bbe::String &filePath)
{
	commitFloatingSelection();

	const bbe::ByteBuffer buffer = serializeLayeredDocumentBytes();
	if (platform.writeBinaryFile) return platform.writeBinaryFile(filePath, buffer);
	return false;
}

bool PaintEditor::loadLayeredDocument(const bbe::String &filePath)
{
	bbe::ByteBuffer buffer;
	if (platform.readBinaryFile) buffer = platform.readBinaryFile(filePath);
	if (buffer.getLength() == 0) return false;

	bbe::ByteBufferSpan span = buffer.getSpan();
	PaintDocument document;
	int32_t storedActiveLayerIndex = 0;
	if (!deserializeLayeredDocumentBytes(span, document, storedActiveLayerIndex)) return false;

	canvas.get() = std::move(document);
	activeLayerIndex = storedActiveLayerIndex;
	this->path = filePath;
	setupCanvas();
	clampActiveLayerIndex();
	return true;
}

bool PaintEditor::saveFlattenedImage(const bbe::String &filePath)
{
	commitFloatingSelection();
	if (!platform.saveImageFile) return false;
	bbe::Image flattened = flattenVisibleLayers();

	const bbe::String lowerPath = filePath.toLowerCase();
	if (lowerPath.endsWith(".ico"))
	{
		// ICO writer expects a 256x256 source image and will generate
		// the other sizes (128/64/32/16) from it. If the canvas size
		// is different, rescale the flattened result accordingly.
		if (flattened.getWidth() != 256 || flattened.getHeight() != 256)
		{
			flattened = flattened.scaledNearest(256, 256);
		}
	}

	return platform.saveImageFile(filePath, flattened);
}

bool PaintEditor::saveDocumentToPath(const bbe::String &filePath)
{
	bool ok;
	if (isLayeredDocumentPath(filePath))
	{
		ok = saveLayeredDocument(filePath);
	}
	else
	{
		ok = saveFlattenedImage(filePath);
	}
	if (ok) savedGeneration = canvasGeneration;
	return ok;
}

void PaintEditor::saveDocumentAs(SaveFormat format)
{
	bbe::String newPath = path;
	const bbe::String defaultExtension = format == SaveFormat::PNG ? "png" : "bbepaint";
	if (platform.showSaveDialog && platform.showSaveDialog(newPath, defaultExtension))
	{
		path = newPath;
		saveDocumentToPath(path);
	}
}

void PaintEditor::requestSave()
{
	if (!path.isEmpty())
	{
		saveDocumentToPath(path);
		return;
	}

	openSaveChoicePopup = true;
}

void PaintEditor::saveCanvas()
{
	requestSave();
}

void PaintEditor::resetCamera()
{
	offset = bbe::Vector2(
		viewport.width * 0.5f - getCanvasWidth() * 0.5f,
		viewport.height * 0.5f - getCanvasHeight() * 0.5f);
	zoomLevel = 1.f;
}

void PaintEditor::clearWorkArea()
{
	workArea = bbe::Image(getCanvasWidth(), getCanvasHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
	workArea.keepAfterUpload();
	workArea.setFilterMode(bbe::ImageFilterMode::NEAREST);
}

void PaintEditor::submitCanvas()
{
	canvas.submit();
	canvasGeneration++;
}

void PaintEditor::applyWorkArea()
{
	getActiveLayerImage().blendOver(workArea, { 0, 0 }, false);
	clearWorkArea();
}

void PaintEditor::setupCanvas(bool clearHistory)
{
	prepareDocumentImages();
	clearWorkArea();
	resetCamera();
	clearSelectionState();
	clampActiveLayerIndex();
	symmetryOffsetCustom = false;
	if (clearHistory)
	{
		canvas.clearHistory();
		canvasGeneration = 0;
		savedGeneration = 0;
	}
}

void PaintEditor::newCanvas(uint32_t width, uint32_t height)
{
	canvas.get().layers.clear();
	canvas.get().layers.add(makeLayer("Layer 1", (int32_t)width, (int32_t)height, bbe::Color::white()));
	activeLayerIndex = 0;
	this->path = "";
	setupCanvas();
}

bool PaintEditor::newCanvas(const char *path)
{
	if (isLayeredDocumentPath(path))
	{
		if (loadLayeredDocument(path))
		{
			return true;
		}
		return false;
	}

	canvas.get().layers.clear();
	bbe::Image img;
	if (platform.loadImageFile) img = platform.loadImageFile(path);
	canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, bbe::BlendMode::Normal, std::move(img) });
	activeLayerIndex = 0;
	this->path = path;
	setupCanvas();
	return true;
}

bbe::Vector2 PaintEditor::screenToCanvas(const bbe::Vector2 &pos)
{
	return (pos - offset) / zoomLevel;
}

bbe::Rectangle PaintEditor::getNavigatorRect()
{
	const float canvasW = (float)getCanvasWidth();
	const float canvasH = (float)getCanvasHeight();
	if (canvasW <= 0.f || canvasH <= 0.f) return {};
	const float navMaxSize = 160.f * viewport.scale;
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
	const float margin = 8.f;
	return bbe::Rectangle(viewport.width - navW - margin, viewport.height - navH - margin, navW, navH);
}

bool PaintEditor::toTiledPos(bbe::Vector2 &pos)
{
	if (tiled)
	{
		pos.x = bbe::Math::mod<float>(pos.x, getCanvasWidth());
		pos.y = bbe::Math::mod<float>(pos.y, getCanvasHeight());
		return true; // If we are tiled, then any position is always within the canvas.
	}

	// If we are not tiled, then we have to check if the pos is actually part of the canvas.
	return pos.x >= 0 && pos.y >= 0 && pos.x < getCanvasWidth() && pos.y < getCanvasHeight();
}

void PaintEditor::changeZoom(float val, const bbe::Vector2 &mouseScreenPos)
{
	auto mouseBeforeZoom = screenToCanvas(mouseScreenPos);
	zoomLevel *= val;
	auto mouseAfterZoom = screenToCanvas(mouseScreenPos);
	offset += (mouseAfterZoom - mouseBeforeZoom) * zoomLevel;
}

bbe::Colori PaintEditor::activeDrawColor(bool leftDown, bool rightDown) const
{
	if (!leftDown && !rightDown) return bbe::Color(leftColor).asByteColor();
	return leftDown ? bbe::Color(leftColor).asByteColor() : bbe::Color(rightColor).asByteColor();
}

bbe::Vector2 PaintEditor::getSymmetryCenter() const
{
	if (symmetryOffsetCustom) return symmetryOffset;
	return { getCanvasWidth() * 0.5f, getCanvasHeight() * 0.5f };
}

bbe::List<bbe::Vector2> PaintEditor::getSymmetryPositions(const bbe::Vector2 &pos) const
{
	return bbe::getSymmetryPositions(pos, getSymmetryCenter(), symmetryMode, radialSymmetryCount);
}

bbe::List<float> PaintEditor::getSymmetryRotationAngles() const
{
	return bbe::getSymmetryRotationAngles(symmetryMode, radialSymmetryCount);
}

bool PaintEditor::touch(const bbe::Vector2 &touchPos, bool rectangleShape, bool leftDown, bool rightDown)
{
	bool changed = false;
	const auto positions = getSymmetryPositions(touchPos);
	for (size_t i = 0; i < positions.getLength(); i++)
		changed |= workArea.drawBrushStamp(positions[i], activeDrawColor(leftDown, rightDown), brushWidth, rectangleShape ? bbe::ImageBrushShape::Square : bbe::ImageBrushShape::Circle, tiled, antiAliasingEnabled);
	return changed;
}

bool PaintEditor::touchLine(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, bool rectangleShape, bool leftDown, bool rightDown)
{
	bool changed = false;
	const auto starts = getSymmetryPositions(pos1);
	const auto ends = getSymmetryPositions(pos2);
	for (size_t i = 0; i < starts.getLength(); i++)
		changed |= workArea.drawLineCapsule(starts[i], ends[i], activeDrawColor(leftDown, rightDown), brushWidth, rectangleShape ? bbe::ImageBrushShape::Square : bbe::ImageBrushShape::Circle, tiled, antiAliasingEnabled);
	return changed;
}

void PaintEditor::touchLineSymmetry(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, const bbe::Colori &color, int32_t width, bool rectShape)
{
	const auto starts = getSymmetryPositions(pos1);
	const auto ends = getSymmetryPositions(pos2);
	for (size_t i = 0; i < starts.getLength(); i++)
		workArea.drawLineCapsule(starts[i], ends[i], color, width, rectShape ? bbe::ImageBrushShape::Square : bbe::ImageBrushShape::Circle, tiled, antiAliasingEnabled);
}

void PaintEditor::drawArrowSymmetry(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color)
{
	const auto froms = getSymmetryPositions(from);
	const auto tos = getSymmetryPositions(to);
	for (size_t i = 0; i < froms.getLength(); i++)
		drawArrowToWorkArea(froms[i], tos[i], color);
}

void PaintEditor::drawBezierSymmetry(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color)
{
	if (points.isEmpty()) return;
	const size_t symCount = getSymmetryPositions(points[0]).getLength();
	for (size_t s = 0; s < symCount; s++)
	{
		bbe::List<bbe::Vector2> symPoints;
		for (size_t p = 0; p < points.getLength(); p++)
			symPoints.add(getSymmetryPositions(points[p])[s]);
		drawBezierToWorkArea(symPoints, color);
	}
}

void PaintEditor::onStart(const PaintWindowMetrics &window)
{
	viewport = window;
	lastModeSnapshot = mode;
	pipetteReturnMode = mode;
	buildAvailableFontList();
	newCanvas(400, 300);
}

void PaintEditor::onFilesDropped(const bbe::List<bbe::String> &paths)
{
	pendingDroppedPaths.clear();
	for (size_t i = 0; i < paths.getLength(); i++)
	{
		if (isSupportedDroppedDocumentPath(paths[i]))
		{
			pendingDroppedPaths.add(paths[i]);
		}
	}
	if (!pendingDroppedPaths.isEmpty())
	{
		openDropChoicePopup = true;
	}
}

const bbe::Font &PaintEditor::getTextToolFont() const
{
	const int32_t clampedSize = bbe::Math::max<int32_t>(textFontSize, 1);
	const bbe::String &fontPath = (textFontIndex >= 0 && textFontIndex < (int32_t)availableFonts.getLength())
									  ? availableFonts[(size_t)textFontIndex].path
									  : bbe::String("OpenSansRegular.ttf");
	if (platform.getFont)
	{
		if (const bbe::Font *font = platform.getFont(fontPath, clampedSize))
		{
			return *font;
		}
	}

	// Fallback (should not be used in the ExamplePaint app; it installs a platform font provider).
	static bbe::Font fallback("OpenSansRegular.ttf", (unsigned)20);
	return fallback;
}
