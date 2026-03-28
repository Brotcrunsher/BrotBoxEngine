
#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.

// TODO: Flood fill with edges of brush tool kinda bad.
// TODO: Bug: right click has weird behaviour with shadow

// Todo: Arrow tool
// TODO: configurable roundness of edges in rectangle tool
struct PaintLayer
{
	bbe::String name = "";
	bool visible = true;
	bbe::Image image;
};

struct PaintDocument
{
	bbe::List<PaintLayer> layers;
};

class MyGame : public bbe::Game
{
	bbe::Vector2 offset;
	bbe::String path;
	bbe::UndoableObject<PaintDocument> canvas;
	int32_t activeLayerIndex = 0;
	bbe::Image workArea;
	float zoomLevel = 1.f;
	bool openSaveChoicePopup = false;
	bool showHelpWindow = false;

	enum class SaveFormat
	{
		PNG,
		LAYERED,
	};

	static constexpr const char *LAYERED_FILE_MAGIC = "ExamplePaintLayeredV1";
	static constexpr const char *LAYERED_FILE_EXTENSION = ".bbepaint";

	float leftColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr static int32_t MODE_BRUSH = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	constexpr static int32_t MODE_LINE = 2;
	constexpr static int32_t MODE_RECTANGLE = 3;
	constexpr static int32_t MODE_SELECTION = 4;
	constexpr static int32_t MODE_TEXT = 5;
	constexpr static int32_t MODE_PIPETTE = 6;
	constexpr static int32_t MODE_CIRCLE  = 7;
	int32_t mode = MODE_BRUSH;

	int32_t brushWidth = 1;
	int32_t textFontSize = 20;
	char textBuffer[512] = "Text";
	bbe::Vector2 startMousePos;
	bool roundEdges = false;

	bool drawGridLines = true;
	bool tiled = false;
	bool hasSelection = false;
	bbe::Rectanglei selectionRect;
	bbe::Image selectionClipboard;
	bool selectionFloating = false;
	bbe::Image selectionFloatingImage;
	bool selectionDragActive = false;
	bbe::Vector2i selectionDragStart;
	bool selectionMoveActive = false;
	bbe::Vector2i selectionMoveOffset;
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
	};
	bool selectionResizeActive = false;
	SelectionHitZone selectionResizeZone = SelectionHitZone::NONE;
	bbe::Rectanglei selectionInteractionStartRect;
	bbe::Rectanglei selectionPreviewRect;
	bbe::Image selectionPreviewImage;
	bool rectangleDraftActive = false;
	bool rectangleDraftUsesRightColor = false;
	bool rectangleDragActive = false;
	bool rectangleDragUsesRightColor = false;
	bbe::Vector2i rectangleDragStart;
	bbe::Rectanglei rectangleDragPreviewRect;
	bbe::Image rectangleDragPreviewImage;

	bool circleDraftActive = false;
	bool circleDraftUsesRightColor = false;
	bool circleDragActive = false;
	bool circleDragUsesRightColor = false;
	bbe::Vector2i circleDragStart;
	bbe::Rectanglei circleDragPreviewRect;
	bbe::Image circleDragPreviewImage;

	void prepareImageForCanvas(bbe::Image &image) const
	{
		if (image.getWidth() <= 0 || image.getHeight() <= 0) return;
		image.keepAfterUpload();
		image.setFilterMode(bbe::ImageFilterMode::NEAREST);
	}

	void clearSelectionState()
	{
		hasSelection = false;
		selectionRect = {};
		selectionFloating = false;
		selectionFloatingImage = {};
		selectionDragActive = false;
		selectionDragStart = {};
		selectionMoveActive = false;
		selectionMoveOffset = {};
		selectionResizeActive = false;
		selectionResizeZone = SelectionHitZone::NONE;
		selectionInteractionStartRect = {};
		selectionPreviewRect = {};
		selectionPreviewImage = {};
		rectangleDraftActive = false;
		rectangleDraftUsesRightColor = false;
		rectangleDragActive = false;
		rectangleDragUsesRightColor = false;
		rectangleDragStart = {};
		rectangleDragPreviewRect = {};
		rectangleDragPreviewImage = {};
		circleDraftActive = false;
		circleDraftUsesRightColor = false;
		circleDragActive = false;
		circleDragUsesRightColor = false;
		circleDragStart = {};
		circleDragPreviewRect = {};
		circleDragPreviewImage = {};
	}

	void selectWholeLayer()
	{
		if (selectionFloating)
		{
			commitFloatingSelection();
		}
		if (getCanvasWidth() <= 0 || getCanvasHeight() <= 0) return;

		clearSelectionState();
		mode = MODE_SELECTION;
		hasSelection = true;
		selectionRect = bbe::Rectanglei(0, 0, getCanvasWidth(), getCanvasHeight());
	}

	int32_t getCanvasWidth() const
	{
		if (canvas.get().layers.isEmpty()) return 0;
		return canvas.get().layers[0].image.getWidth();
	}

	int32_t getCanvasHeight() const
	{
		if (canvas.get().layers.isEmpty()) return 0;
		return canvas.get().layers[0].image.getHeight();
	}

	void clampActiveLayerIndex()
	{
		if (canvas.get().layers.isEmpty())
		{
			activeLayerIndex = 0;
			return;
		}
		activeLayerIndex = bbe::Math::clamp(activeLayerIndex, 0, (int32_t)canvas.get().layers.getLength() - 1);
	}

	PaintLayer &getActiveLayer()
	{
		clampActiveLayerIndex();
		return canvas.get().layers[(size_t)activeLayerIndex];
	}

	const PaintLayer &getActiveLayer() const
	{
		if (canvas.get().layers.isEmpty()) bbe::Crash(bbe::Error::IllegalState);
		const int32_t clampedIndex = bbe::Math::clamp(activeLayerIndex, 0, (int32_t)canvas.get().layers.getLength() - 1);
		return canvas.get().layers[(size_t)clampedIndex];
	}

	bbe::Image &getActiveLayerImage()
	{
		return getActiveLayer().image;
	}

	const bbe::Image &getActiveLayerImage() const
	{
		return getActiveLayer().image;
	}

	void prepareLayer(PaintLayer &layer) const
	{
		prepareImageForCanvas(layer.image);
	}

	PaintLayer makeLayer(const bbe::String &name, int32_t width, int32_t height, const bbe::Color &color = bbe::Color(0.0f, 0.0f, 0.0f, 0.0f)) const
	{
		PaintLayer layer;
		layer.name = name;
		layer.visible = true;
		layer.image = bbe::Image(width, height, color);
		prepareLayer(layer);
		return layer;
	}

	void prepareDocumentImages()
	{
		for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
		{
			prepareLayer(canvas.get().layers[i]);
		}
	}

	void prepareForLayerTargetChange()
	{
		commitFloatingSelection();
		clearSelectionState();
		clearWorkArea();
	}

	bool isLayeredDocumentPath(const bbe::String &filePath) const
	{
		return filePath.toLowerCase().endsWith(LAYERED_FILE_EXTENSION);
	}

	bool isSupportedDroppedDocumentPath(const bbe::String &filePath) const
	{
		const bbe::String lowerPath = filePath.toLowerCase();
		return lowerPath.endsWith(".png") || lowerPath.endsWith(LAYERED_FILE_EXTENSION);
	}

	void blendImageOntoImage(bbe::Image &target, const bbe::Image &image, const bbe::Vector2i &pos, bool repeated = false) const
	{
		for (int32_t x = 0; x < image.getWidth(); x++)
		{
			for (int32_t y = 0; y < image.getHeight(); y++)
			{
				int32_t targetX = pos.x + x;
				int32_t targetY = pos.y + y;
				if (repeated)
				{
					targetX = bbe::Math::mod<int32_t>(targetX, target.getWidth());
					targetY = bbe::Math::mod<int32_t>(targetY, target.getHeight());
				}
				else
				{
					if (targetX < 0 || targetY < 0 || targetX >= target.getWidth() || targetY >= target.getHeight()) continue;
				}

				const bbe::Colori sourceColor = image.getPixel((size_t)x, (size_t)y);
				if (sourceColor.a == 0) continue;

				const bbe::Colori oldColor = target.getPixel((size_t)targetX, (size_t)targetY);
				target.setPixel((size_t)targetX, (size_t)targetY, oldColor.blendTo(sourceColor));
			}
		}
	}

	bbe::Image flattenVisibleLayers() const
	{
		bbe::Image flattened(getCanvasWidth(), getCanvasHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		prepareImageForCanvas(flattened);
		for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
		{
			const PaintLayer &layer = canvas.get().layers[i];
			if (!layer.visible) continue;
			blendImageOntoImage(flattened, layer.image, { 0, 0 });
		}
		return flattened;
	}

	bbe::Colori getVisiblePixel(size_t x, size_t y) const
	{
		bbe::Colori color(0, 0, 0, 0);
		bool hasColor = false;
		for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
		{
			const PaintLayer &layer = canvas.get().layers[i];
			if (!layer.visible) continue;

			const bbe::Colori sourceColor = layer.image.getPixel(x, y);
			if (sourceColor.a == 0) continue;

			color = hasColor ? color.blendTo(sourceColor) : sourceColor;
			hasColor = true;
		}
		return color;
	}

	bbe::String makeLayerName() const
	{
		return bbe::String("Layer ") + (canvas.get().layers.getLength() + 1);
	}

	void addLayer()
	{
		prepareForLayerTargetChange();
		canvas.get().layers.add(makeLayer(makeLayerName(), getCanvasWidth(), getCanvasHeight()));
		activeLayerIndex = (int32_t)canvas.get().layers.getLength() - 1;
		canvas.submit();
	}

	void deleteActiveLayer()
	{
		if (canvas.get().layers.getLength() <= 1) return;
		prepareForLayerTargetChange();
		canvas.get().layers.removeIndex((size_t)activeLayerIndex);
		clampActiveLayerIndex();
		canvas.submit();
	}

	void moveActiveLayerUp()
	{
		if ((size_t)activeLayerIndex + 1 >= canvas.get().layers.getLength()) return;
		prepareForLayerTargetChange();
		canvas.get().layers.swap((size_t)activeLayerIndex, (size_t)activeLayerIndex + 1);
		activeLayerIndex++;
		canvas.submit();
	}

	void moveActiveLayerDown()
	{
		if (activeLayerIndex <= 0) return;
		prepareForLayerTargetChange();
		canvas.get().layers.swap((size_t)activeLayerIndex, (size_t)activeLayerIndex - 1);
		activeLayerIndex--;
		canvas.submit();
	}

	void setActiveLayerIndex(int32_t newIndex)
	{
		if (newIndex == activeLayerIndex) return;
		prepareForLayerTargetChange();
		activeLayerIndex = newIndex;
		clampActiveLayerIndex();
	}

	bbe::Vector2i toCanvasPixel(const bbe::Vector2 &pos) const
	{
		return bbe::Vector2i((int32_t)bbe::Math::floor(pos.x), (int32_t)bbe::Math::floor(pos.y));
	}

	bbe::Vector2i toTiledCanvasPixel(const bbe::Vector2 &pos)
	{
		bbe::Vector2 p = pos;
		toTiledPos(p);
		return toCanvasPixel(p);
	}

	bool clampRectToCanvas(const bbe::Rectanglei &rect, bbe::Rectanglei &outRect) const
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

	bbe::Vector2i constrainToSquare(const bbe::Vector2i &start, const bbe::Vector2i &end) const
	{
		const int32_t dx = end.x - start.x;
		const int32_t dy = end.y - start.y;
		const int32_t size = bbe::Math::min(bbe::Math::abs(dx), bbe::Math::abs(dy));
		return bbe::Vector2i(
			start.x + (dx >= 0 ? size : -size),
			start.y + (dy >= 0 ? size : -size));
	}

	bool buildSelectionRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
	{
		const int32_t left = bbe::Math::min(pos1.x, pos2.x);
		const int32_t top = bbe::Math::min(pos1.y, pos2.y);
		const int32_t right = bbe::Math::max(pos1.x, pos2.x);
		const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
		return clampRectToCanvas(bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1), outRect);
	}

	bbe::Rectanglei buildRawRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2) const
	{
		const int32_t left = bbe::Math::min(pos1.x, pos2.x);
		const int32_t top = bbe::Math::min(pos1.y, pos2.y);
		const int32_t right = bbe::Math::max(pos1.x, pos2.x);
		const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
		return bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1);
	}

	bool isPointInSelection(const bbe::Vector2i &point) const
	{
		return hasSelection && selectionRect.isPointInRectangle(point, true);
	}

	bool isSelectionResizeHit(const SelectionHitZone hitZone) const
	{
		return hitZone != SelectionHitZone::NONE && hitZone != SelectionHitZone::INSIDE;
	}

	SelectionHitZone getSelectionHitZone(const bbe::Vector2i &point) const
	{
		if (!hasSelection || selectionRect.width <= 0 || selectionRect.height <= 0)
		{
			return SelectionHitZone::NONE;
		}

		const int32_t left = selectionRect.x;
		const int32_t top = selectionRect.y;
		const int32_t right = selectionRect.x + selectionRect.width - 1;
		const int32_t bottom = selectionRect.y + selectionRect.height - 1;
		const int32_t padding = bbe::Math::max<int32_t>(1, (int32_t)bbe::Math::ceil(6.0f / zoomLevel));

		const bool nearLeft = point.x >= left - padding && point.x <= left + padding && point.y >= top - padding && point.y <= bottom + padding;
		const bool nearRight = point.x >= right - padding && point.x <= right + padding && point.y >= top - padding && point.y <= bottom + padding;
		const bool nearTop = point.y >= top - padding && point.y <= top + padding && point.x >= left - padding && point.x <= right + padding;
		const bool nearBottom = point.y >= bottom - padding && point.y <= bottom + padding && point.x >= left - padding && point.x <= right + padding;

		if (nearLeft && nearTop) return SelectionHitZone::TOP_LEFT;
		if (nearRight && nearTop) return SelectionHitZone::TOP_RIGHT;
		if (nearLeft && nearBottom) return SelectionHitZone::BOTTOM_LEFT;
		if (nearRight && nearBottom) return SelectionHitZone::BOTTOM_RIGHT;
		if (nearLeft) return SelectionHitZone::LEFT;
		if (nearRight) return SelectionHitZone::RIGHT;
		if (nearTop) return SelectionHitZone::TOP;
		if (nearBottom) return SelectionHitZone::BOTTOM;
		if (isPointInSelection(point)) return SelectionHitZone::INSIDE;
		return SelectionHitZone::NONE;
	}

		bool isWholeLayerSelection(const bbe::Rectanglei &rect) const
		{
			return rect.x == 0
				&& rect.y == 0
				&& rect.width == getCanvasWidth()
				&& rect.height == getCanvasHeight();
		}

		bool shouldClearWholeLayerSelectionToTransparency() const
		{
			return canvas.get().layers.getLength() > 1;
		}

	bbe::Image copyCanvasRect(const bbe::Rectanglei &rect) const
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

		void clearCanvasRect(const bbe::Rectanglei &rect)
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

	void blendImageOntoCanvas(const bbe::Image &image, const bbe::Vector2i &pos)
	{
		blendImageOntoImage(getActiveLayerImage(), image, pos, tiled);
	}

	void storeSelectionInClipboard()
	{
		if (!hasSelection) return;
		selectionClipboard = selectionFloating ? selectionFloatingImage : copyCanvasRect(selectionRect);
		prepareImageForCanvas(selectionClipboard);
		if (bbe::Image::supportsClipboardImages())
		{
			selectionClipboard.copyToClipboard();
		}
	}

	void deleteSelection()
	{
		if (!hasSelection) return;
		if (selectionFloating)
		{
			clearSelectionState();
			return;
		}
		clearCanvasRect(selectionRect);
		canvas.submit();
		clearSelectionState();
	}

	void cutSelection()
	{
		if (!hasSelection) return;
		storeSelectionInClipboard();
		if (selectionFloating)
		{
			clearSelectionState();
			return;
		}
		deleteSelection();
	}

	bool getPasteImage(bbe::Image &image)
	{
		if (bbe::Image::supportsClipboardImages() && bbe::Image::isImageInClipbaord())
		{
			image = bbe::Image::getClipboardImage();
			prepareImageForCanvas(image);
			return image.getWidth() > 0 && image.getHeight() > 0;
		}

		if (selectionClipboard.getWidth() > 0 && selectionClipboard.getHeight() > 0)
		{
			image = selectionClipboard;
			prepareImageForCanvas(image);
			return true;
		}

		return false;
	}

	void pasteSelectionAt(const bbe::Vector2i &pos)
	{
		bbe::Image image;
		if (!getPasteImage(image)) return;

		if (selectionFloating)
		{
			commitFloatingSelection();
		}

		mode = MODE_SELECTION;
		hasSelection = true;
		selectionFloating = true;
		selectionFloatingImage = image;
		selectionRect = bbe::Rectanglei(pos.x, pos.y, image.getWidth(), image.getHeight());
		rectangleDraftActive = false;
		rectangleDraftUsesRightColor = false;
		selectionMoveActive = false;
		selectionResizeActive = false;
		selectionDragActive = false;
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	void commitFloatingSelection()
	{
		if (!selectionFloating) return;

		blendImageOntoCanvas(selectionFloatingImage, selectionRect.getPos());
		canvas.submit();

		bbe::Vector2i displayPos = selectionRect.getPos();
		if (tiled)
		{
			displayPos.x = bbe::Math::mod<int32_t>(displayPos.x, getCanvasWidth());
			displayPos.y = bbe::Math::mod<int32_t>(displayPos.y, getCanvasHeight());
		}

		bbe::Rectanglei clampedRect;
		if (!clampRectToCanvas(bbe::Rectanglei(displayPos.x, displayPos.y, selectionFloatingImage.getWidth(), selectionFloatingImage.getHeight()), clampedRect))
		{
			clearSelectionState();
			return;
		}

		selectionRect = clampedRect;
		selectionFloating = false;
		selectionFloatingImage = {};
		rectangleDraftActive = false;
		rectangleDraftUsesRightColor = false;
		circleDraftActive = false;
		circleDraftUsesRightColor = false;
	}

	bool toImagePos(bbe::Vector2 &pos, int32_t width, int32_t height, bool repeated) const
	{
		if (repeated)
		{
			pos.x = bbe::Math::mod<float>(pos.x, width);
			pos.y = bbe::Math::mod<float>(pos.y, height);
			return true;
		}

		return pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height;
	}

	bool touchImage(bbe::Image &image, const bbe::Vector2 &touchPos, const bbe::Colori &color, int32_t toolBrushWidth, bool rectangleShape, bool repeated) const
	{
		bool changeRegistered = false;
		for (int32_t i = -toolBrushWidth; i <= toolBrushWidth; i++)
		{
			for (int32_t k = -toolBrushWidth; k <= toolBrushWidth; k++)
			{
				float pencilStrength = bbe::Math::clamp01(toolBrushWidth - bbe::Math::sqrt(i * i + k * k));
				if (rectangleShape)
				{
					if (i != -toolBrushWidth && i != toolBrushWidth && k != -toolBrushWidth && k != toolBrushWidth)
					{
						pencilStrength = 1.0f;
					}
					else
					{
						pencilStrength = 0.0f;
					}
				}
				if (pencilStrength <= 0.f) continue;

				bbe::Vector2 coord = touchPos + bbe::Vector2(i, k);
				if (toImagePos(coord, image.getWidth(), image.getHeight(), repeated))
				{
					bbe::Colori newColor = color;
					newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
					const int32_t pixelX = (int32_t)coord.x;
					const int32_t pixelY = (int32_t)coord.y;
					bbe::Colori oldColor = image.getPixel((size_t)pixelX, (size_t)pixelY);
					if (newColor.a > oldColor.a)
					{
						image.setPixel((size_t)pixelX, (size_t)pixelY, newColor);
						changeRegistered = true;
					}
				}
			}
		}
		return changeRegistered;
	}

	bool touchLineImage(bbe::Image &image, const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, const bbe::Colori &color, int32_t toolBrushWidth, bool rectangleShape, bool repeated) const
	{
		bool changeRegistered = false;
		bbe::GridIterator gi(pos1, pos2);
		while (gi.hasNext())
		{
			const bbe::Vector2 coordBase = gi.next().as<float>();
			changeRegistered |= touchImage(image, coordBase, color, toolBrushWidth, rectangleShape, repeated);
		}
		return changeRegistered;
	}

	bbe::Image scaleImageNearest(const bbe::Image &image, int32_t width, int32_t height) const
	{
		if (width <= 0 || height <= 0) return {};
		if (image.getWidth() == width && image.getHeight() == height) return image;

		bbe::Image scaled(width, height, bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		prepareImageForCanvas(scaled);
		for (int32_t x = 0; x < width; x++)
		{
			for (int32_t y = 0; y < height; y++)
			{
				int32_t sourceX = (int32_t)((int64_t)x * image.getWidth() / width);
				int32_t sourceY = (int32_t)((int64_t)y * image.getHeight() / height);
				if (sourceX >= image.getWidth()) sourceX = image.getWidth() - 1;
				if (sourceY >= image.getHeight()) sourceY = image.getHeight() - 1;
				scaled.setPixel((size_t)x, (size_t)y, image.getPixel((size_t)sourceX, (size_t)sourceY));
			}
		}
		return scaled;
	}

	bbe::Colori getRectangleDraftColor() const
	{
		return rectangleDraftUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	bbe::Colori getRectangleDragColor() const
	{
		return rectangleDragUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	int32_t getRectangleDraftPadding() const
	{
		return 0;
	}

	bbe::Rectanglei expandRectangleRect(const bbe::Rectanglei &rect) const
	{
		const int32_t padding = getRectangleDraftPadding();
		return bbe::Rectanglei(
			rect.x - padding,
			rect.y - padding,
			rect.width + padding * 2,
			rect.height + padding * 2);
	}

	bool buildRectangleDraftRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
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

	bbe::Image createRectangleImage(int32_t width, int32_t height, const bbe::Colori &color) const
	{
		if (width <= 0 || height <= 0) return {};

		bbe::Image image(width, height, bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		prepareImageForCanvas(image);

		for (int32_t y = 0; y < height; y++)
		{
			for (int32_t x = 0; x < width; x++)
			{
				const bool inTopBorder    = y < brushWidth;
				const bool inBottomBorder = y >= height - brushWidth;
				const bool inLeftBorder   = x < brushWidth;
				const bool inRightBorder  = x >= width - brushWidth;

				if (!inTopBorder && !inBottomBorder && !inLeftBorder && !inRightBorder) continue;

				if (roundEdges)
				{
					const bool inCorner = (inTopBorder || inBottomBorder) && (inLeftBorder || inRightBorder);
					if (inCorner)
					{
						const float cx = inLeftBorder ? (float)(brushWidth - 1 - x) : (float)(x - (width - brushWidth));
						const float cy = inTopBorder  ? (float)(brushWidth - 1 - y) : (float)(y - (height - brushWidth));
						const float strength = bbe::Math::clamp01((float)brushWidth - 0.5f - bbe::Math::sqrt(cx * cx + cy * cy));
						if (strength <= 0.f) continue;
						bbe::Colori c = color;
						c.a = (bbe::byte)(c.a * strength);
						image.setPixel((size_t)x, (size_t)y, c);
						continue;
					}
				}

				image.setPixel((size_t)x, (size_t)y, color);
			}
		}

		return image;
	}

	bbe::Image createRectangleDraftImage(int32_t width, int32_t height) const
	{
		return createRectangleImage(width, height, getRectangleDraftColor());
	}

	bbe::Image createRectangleDragPreviewImage(int32_t width, int32_t height) const
	{
		return createRectangleImage(width, height, getRectangleDragColor());
	}

	void refreshActiveRectangleDraftImage()
	{
		if (!rectangleDraftActive) return;

		if (selectionMoveActive || selectionResizeActive)
		{
			if (selectionPreviewRect.width > 0 && selectionPreviewRect.height > 0)
			{
				selectionPreviewImage = createRectangleDraftImage(selectionPreviewRect.width, selectionPreviewRect.height);
			}
			return;
		}

		if (selectionRect.width > 0 && selectionRect.height > 0)
		{
			selectionFloatingImage = createRectangleDraftImage(selectionRect.width, selectionRect.height);
		}
	}

	bbe::Colori getCircleDraftColor() const
	{
		return circleDraftUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	bbe::Colori getCircleDragColor() const
	{
		return circleDragUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	bbe::Image createCircleImage(int32_t width, int32_t height, const bbe::Colori &color) const
	{
		if (width <= 0 || height <= 0) return {};

		bbe::Image image(width, height, bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		prepareImageForCanvas(image);

		const float rx_outer = width  * 0.5f;
		const float ry_outer = height * 0.5f;
		const float rx_inner = rx_outer - (float)brushWidth;
		const float ry_inner = ry_outer - (float)brushWidth;
		const float minRadius_outer = rx_outer < ry_outer ? rx_outer : ry_outer;

		for (int32_t y = 0; y < height; y++)
		{
			for (int32_t x = 0; x < width; x++)
			{
				const float px = x - rx_outer + 0.5f;
				const float py = y - ry_outer + 0.5f;

				const float nx_o = px / rx_outer;
				const float ny_o = py / ry_outer;
				const float d_outer = bbe::Math::sqrt(nx_o * nx_o + ny_o * ny_o);
				const float alpha_outer = bbe::Math::clamp01((1.f - d_outer) * minRadius_outer + 0.5f);
				if (alpha_outer <= 0.f) continue;

				float alpha_inner = 1.f;
				if (rx_inner > 0.f && ry_inner > 0.f)
				{
					const float nx_i = px / rx_inner;
					const float ny_i = py / ry_inner;
					const float d_inner = bbe::Math::sqrt(nx_i * nx_i + ny_i * ny_i);
					const float minRadius_inner = rx_inner < ry_inner ? rx_inner : ry_inner;
					alpha_inner = bbe::Math::clamp01((d_inner - 1.f) * minRadius_inner + 0.5f);
				}

				const float alpha = alpha_outer < alpha_inner ? alpha_outer : alpha_inner;
				if (alpha <= 0.f) continue;

				bbe::Colori c = color;
				c.a = (bbe::byte)(c.a * alpha);
				image.setPixel((size_t)x, (size_t)y, c);
			}
		}

		return image;
	}

	bbe::Image createCircleDraftImage(int32_t width, int32_t height) const
	{
		return createCircleImage(width, height, getCircleDraftColor());
	}

	bbe::Image createCircleDragPreviewImage(int32_t width, int32_t height) const
	{
		return createCircleImage(width, height, getCircleDragColor());
	}

	void refreshActiveCircleDraftImage()
	{
		if (!circleDraftActive) return;

		if (selectionMoveActive || selectionResizeActive)
		{
			if (selectionPreviewRect.width > 0 && selectionPreviewRect.height > 0)
			{
				selectionPreviewImage = createCircleDraftImage(selectionPreviewRect.width, selectionPreviewRect.height);
			}
			return;
		}

		if (selectionRect.width > 0 && selectionRect.height > 0)
		{
			selectionFloatingImage = createCircleDraftImage(selectionRect.width, selectionRect.height);
		}
	}

	void finalizeCircleDrag(const bbe::Vector2i &mousePixel)
	{
		const bool useRightColor = circleDragUsesRightColor;
		circleDragActive = false;

		const bool shiftDown = isKeyDown(bbe::Key::LEFT_SHIFT) || isKeyDown(bbe::Key::RIGHT_SHIFT);
		const bbe::Vector2i constrainedPixel = shiftDown ? constrainToSquare(circleDragStart, mousePixel) : mousePixel;
		bbe::Rectanglei draftRect;
		if (tiled)
		{
			draftRect = buildRawRect(circleDragStart, constrainedPixel);
		}
		if ((!tiled && !buildSelectionRect(circleDragStart, constrainedPixel, draftRect))
			|| draftRect.width <= 0 || draftRect.height <= 0)
		{
			circleDragPreviewRect = {};
			circleDragPreviewImage = {};
			return;
		}

		const bbe::Image draftImage = circleDragPreviewImage.getWidth() > 0 && circleDragPreviewImage.getHeight() > 0
			? circleDragPreviewImage
			: createCircleImage(draftRect.width, draftRect.height, useRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor());

		clearSelectionState();
		hasSelection = true;
		selectionFloating = true;
		selectionRect = draftRect;
		circleDraftActive = true;
		circleDraftUsesRightColor = useRightColor;
		selectionFloatingImage = draftImage;
		circleDragPreviewRect = {};
		circleDragPreviewImage = {};
	}

	void beginCircleDrag(const bbe::Vector2i &mousePixel, bool useRightColor)
	{
		circleDragActive = true;
		circleDragUsesRightColor = useRightColor;
		circleDragStart = mousePixel;
		circleDragPreviewRect = {};
		circleDragPreviewImage = {};
	}

	void updateCircleDragPreview(const bbe::Vector2i &mousePixel)
	{
		const bool shiftDown = isKeyDown(bbe::Key::LEFT_SHIFT) || isKeyDown(bbe::Key::RIGHT_SHIFT);
		const bbe::Vector2i constrainedPixel = shiftDown ? constrainToSquare(circleDragStart, mousePixel) : mousePixel;
		if (tiled)
		{
			circleDragPreviewRect = buildRawRect(circleDragStart, constrainedPixel);
			if (circleDragPreviewRect.width <= 0 || circleDragPreviewRect.height <= 0)
			{
				circleDragPreviewRect = {};
				circleDragPreviewImage = {};
				return;
			}
		}
		else if (!buildSelectionRect(circleDragStart, constrainedPixel, circleDragPreviewRect))
		{
			circleDragPreviewRect = {};
			circleDragPreviewImage = {};
			return;
		}

		circleDragPreviewImage = createCircleDragPreviewImage(circleDragPreviewRect.width, circleDragPreviewRect.height);
	}

	void updateCircleTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2i mousePixel = toCanvasPixel(currMousePos);
		bool handledMousePress = false;

		if (circleDraftActive)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				const SelectionHitZone hitZone = getSelectionHitZone(mousePixel);
				if (isSelectionResizeHit(hitZone))
				{
					beginSelectionResize(hitZone);
				}
				else if (hitZone == SelectionHitZone::INSIDE)
				{
					beginSelectionMove(mousePixel);
				}
				else
				{
					commitFloatingSelection();
					clearSelectionState();
				}
				handledMousePress = true;
			}
			if (!handledMousePress && isMousePressed(bbe::MouseButton::RIGHT))
			{
				commitFloatingSelection();
				clearSelectionState();
				handledMousePress = true;
			}
		}

		if (!handledMousePress && !circleDraftActive)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				beginCircleDrag(mousePixel, false);
			}
			else if (isMousePressed(bbe::MouseButton::RIGHT))
			{
				beginCircleDrag(mousePixel, true);
			}
		}

		if (selectionMoveActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionMovePreview(mousePixel);
		}
		if (selectionResizeActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionResizePreview(mousePixel);
		}
		if ((selectionMoveActive || selectionResizeActive) && isMouseReleased(bbe::MouseButton::LEFT))
		{
			applySelectionTransform();
		}

		if (!circleDragActive) return;
		updateCircleDragPreview(mousePixel);

		const bool dragReleased = circleDragUsesRightColor
			? isMouseReleased(bbe::MouseButton::RIGHT)
			: isMouseReleased(bbe::MouseButton::LEFT);
		if (dragReleased)
		{
			finalizeCircleDrag(mousePixel);
		}
	}

	void beginSelectionMove(const bbe::Vector2i &mousePixel)
	{
		selectionMoveActive = true;
		selectionMoveOffset = mousePixel - selectionRect.getPos();
		selectionInteractionStartRect = selectionRect;
		selectionPreviewRect = selectionRect;
		selectionPreviewImage = selectionFloating ? selectionFloatingImage : copyCanvasRect(selectionRect);
	}

	void updateSelectionMovePreview(const bbe::Vector2i &mousePixel)
	{
		selectionPreviewRect = bbe::Rectanglei(
			mousePixel.x - selectionMoveOffset.x,
			mousePixel.y - selectionMoveOffset.y,
			selectionPreviewImage.getWidth(),
			selectionPreviewImage.getHeight());
	}

	void beginSelectionResize(const SelectionHitZone hitZone)
	{
		selectionResizeActive = true;
		selectionResizeZone = hitZone;
		selectionInteractionStartRect = selectionRect;
		selectionPreviewRect = selectionRect;
		selectionPreviewImage = selectionFloating ? selectionFloatingImage : copyCanvasRect(selectionRect);
	}

	void updateSelectionResizePreview(const bbe::Vector2i &mousePixel)
	{
		const int32_t originalLeft = selectionInteractionStartRect.x;
		const int32_t originalTop = selectionInteractionStartRect.y;
		const int32_t originalRight = selectionInteractionStartRect.x + selectionInteractionStartRect.width - 1;
		const int32_t originalBottom = selectionInteractionStartRect.y + selectionInteractionStartRect.height - 1;

		int32_t left = originalLeft;
		int32_t top = originalTop;
		int32_t right = originalRight;
		int32_t bottom = originalBottom;

		switch (selectionResizeZone)
		{
		case SelectionHitZone::LEFT:
		case SelectionHitZone::TOP_LEFT:
		case SelectionHitZone::BOTTOM_LEFT:
			left = mousePixel.x;
			break;
		default:
			break;
		}
		switch (selectionResizeZone)
		{
		case SelectionHitZone::RIGHT:
		case SelectionHitZone::TOP_RIGHT:
		case SelectionHitZone::BOTTOM_RIGHT:
			right = mousePixel.x;
			break;
		default:
			break;
		}
		switch (selectionResizeZone)
		{
		case SelectionHitZone::TOP:
		case SelectionHitZone::TOP_LEFT:
		case SelectionHitZone::TOP_RIGHT:
			top = mousePixel.y;
			break;
		default:
			break;
		}
		switch (selectionResizeZone)
		{
		case SelectionHitZone::BOTTOM:
		case SelectionHitZone::BOTTOM_LEFT:
		case SelectionHitZone::BOTTOM_RIGHT:
			bottom = mousePixel.y;
			break;
		default:
			break;
		}

		selectionPreviewRect = buildRawRect({ left, top }, { right, bottom });
		if (rectangleDraftActive)
		{
			refreshActiveRectangleDraftImage();
		}
	}

	bbe::Image buildSelectionPreviewResultImage() const
	{
		if (rectangleDraftActive)
		{
			return createRectangleDraftImage(selectionPreviewRect.width, selectionPreviewRect.height);
		}

		if (circleDraftActive)
		{
			return createCircleDraftImage(selectionPreviewRect.width, selectionPreviewRect.height);
		}

		if (selectionPreviewRect.width != selectionPreviewImage.getWidth() || selectionPreviewRect.height != selectionPreviewImage.getHeight())
		{
			return scaleImageNearest(selectionPreviewImage, selectionPreviewRect.width, selectionPreviewRect.height);
		}

		return selectionPreviewImage;
	}

	void clearSelectionInteractionState()
	{
		selectionMoveActive = false;
		selectionMoveOffset = {};
		selectionResizeActive = false;
		selectionResizeZone = SelectionHitZone::NONE;
		selectionInteractionStartRect = {};
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	void applySelectionTransform()
	{
		if (!selectionMoveActive && !selectionResizeActive) return;

		const bool rectChanged = selectionPreviewRect.x != selectionRect.x
			|| selectionPreviewRect.y != selectionRect.y
			|| selectionPreviewRect.width != selectionRect.width
			|| selectionPreviewRect.height != selectionRect.height;

		if (selectionFloating)
		{
			if (rectChanged)
			{
				selectionRect = selectionPreviewRect;
				selectionFloatingImage = buildSelectionPreviewResultImage();
			}
			clearSelectionInteractionState();
			return;
		}

		if (rectChanged)
		{
			clearCanvasRect(selectionRect);
			selectionRect = selectionPreviewRect;
			selectionFloating = true;
			selectionFloatingImage = buildSelectionPreviewResultImage();
			rectangleDraftActive = false;
			rectangleDraftUsesRightColor = false;
			circleDraftActive = false;
			circleDraftUsesRightColor = false;
			hasSelection = true;
		}

		clearSelectionInteractionState();
	}

	void updateSelectionTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2i mousePixel = toCanvasPixel(currMousePos);

		if (isMousePressed(bbe::MouseButton::LEFT))
		{
			const SelectionHitZone hitZone = getSelectionHitZone(mousePixel);
			if (isSelectionResizeHit(hitZone))
			{
				beginSelectionResize(hitZone);
			}
			else if (hitZone == SelectionHitZone::INSIDE)
			{
				beginSelectionMove(mousePixel);
			}
			else
			{
				if (selectionFloating)
				{
					commitFloatingSelection();
				}
				selectionDragActive = true;
				selectionDragStart = mousePixel;
				hasSelection = false;
				selectionRect = {};
				selectionPreviewRect = {};
			}
		}

		if (isMouseDown(bbe::MouseButton::LEFT))
		{
			if (selectionDragActive)
			{
				buildSelectionRect(selectionDragStart, mousePixel, selectionPreviewRect);
			}
			if (selectionMoveActive)
			{
				updateSelectionMovePreview(mousePixel);
			}
			if (selectionResizeActive)
			{
				updateSelectionResizePreview(mousePixel);
			}
		}

		if (isMouseReleased(bbe::MouseButton::LEFT))
		{
			if (selectionDragActive)
			{
				hasSelection = buildSelectionRect(selectionDragStart, mousePixel, selectionRect);
				selectionDragActive = false;
				selectionPreviewRect = {};
			}

			if (selectionMoveActive || selectionResizeActive)
			{
				applySelectionTransform();
			}
		}
	}

	void finalizeRectangleDrag(const bbe::Vector2i &mousePixel)
	{
		const bool useRightColor = rectangleDragUsesRightColor;
		rectangleDragActive = false;

		const bool shiftDown = isKeyDown(bbe::Key::LEFT_SHIFT) || isKeyDown(bbe::Key::RIGHT_SHIFT);
		const bbe::Vector2i constrainedPixel = shiftDown ? constrainToSquare(rectangleDragStart, mousePixel) : mousePixel;
		if (!buildRectangleDraftRect(rectangleDragStart, constrainedPixel, rectangleDragPreviewRect))
		{
			rectangleDragPreviewRect = {};
			rectangleDragPreviewImage = {};
			return;
		}

		const bbe::Rectanglei draftRect = rectangleDragPreviewRect;
		const bbe::Image draftImage = rectangleDragPreviewImage.getWidth() > 0 && rectangleDragPreviewImage.getHeight() > 0
			? rectangleDragPreviewImage
			: createRectangleImage(draftRect.width, draftRect.height, useRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor());

		clearSelectionState();
		hasSelection = true;
		selectionFloating = true;
		selectionRect = draftRect;
		rectangleDraftActive = true;
		rectangleDraftUsesRightColor = useRightColor;
		selectionFloatingImage = draftImage;
		rectangleDragPreviewRect = {};
		rectangleDragPreviewImage = {};
	}

	void beginRectangleDrag(const bbe::Vector2i &mousePixel, bool useRightColor)
	{
		rectangleDragActive = true;
		rectangleDragUsesRightColor = useRightColor;
		rectangleDragStart = mousePixel;
		rectangleDragPreviewRect = {};
		rectangleDragPreviewImage = {};
	}

	void updateRectangleDragPreview(const bbe::Vector2i &mousePixel)
	{
		const bool shiftDown = isKeyDown(bbe::Key::LEFT_SHIFT) || isKeyDown(bbe::Key::RIGHT_SHIFT);
		const bbe::Vector2i constrainedPixel = shiftDown ? constrainToSquare(rectangleDragStart, mousePixel) : mousePixel;
		if (!buildRectangleDraftRect(rectangleDragStart, constrainedPixel, rectangleDragPreviewRect))
		{
			rectangleDragPreviewRect = {};
			rectangleDragPreviewImage = {};
			return;
		}

		rectangleDragPreviewImage = createRectangleDragPreviewImage(rectangleDragPreviewRect.width, rectangleDragPreviewRect.height);
	}

	void updateRectangleTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2i mousePixel = toCanvasPixel(currMousePos);
		bool handledMousePress = false;

		if (rectangleDraftActive)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				const SelectionHitZone hitZone = getSelectionHitZone(mousePixel);
				if (isSelectionResizeHit(hitZone))
				{
					beginSelectionResize(hitZone);
				}
				else if (hitZone == SelectionHitZone::INSIDE)
				{
					beginSelectionMove(mousePixel);
				}
					else
					{
						commitFloatingSelection();
						clearSelectionState();
					}
					handledMousePress = true;
				}
				if (!handledMousePress && isMousePressed(bbe::MouseButton::RIGHT))
				{
					commitFloatingSelection();
					clearSelectionState();
					handledMousePress = true;
				}
			}

		if (!handledMousePress && !rectangleDraftActive)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				beginRectangleDrag(mousePixel, false);
			}
			else if (isMousePressed(bbe::MouseButton::RIGHT))
			{
				beginRectangleDrag(mousePixel, true);
			}
		}

		if (selectionMoveActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionMovePreview(mousePixel);
		}
		if (selectionResizeActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionResizePreview(mousePixel);
		}
		if ((selectionMoveActive || selectionResizeActive) && isMouseReleased(bbe::MouseButton::LEFT))
		{
			applySelectionTransform();
		}

		if (!rectangleDragActive) return;
		updateRectangleDragPreview(mousePixel);

		const bool dragReleased = rectangleDragUsesRightColor
			? isMouseReleased(bbe::MouseButton::RIGHT)
			: isMouseReleased(bbe::MouseButton::LEFT);
		if (dragReleased)
		{
			finalizeRectangleDrag(mousePixel);
		}
	}

	bbe::Rectangle selectionRectToScreen(const bbe::Rectanglei &rect) const
	{
		return bbe::Rectangle(
			offset.x + rect.x * zoomLevel,
			offset.y + rect.y * zoomLevel,
			rect.width * zoomLevel,
			rect.height * zoomLevel);
	}

	void drawSelectionOutline(bbe::PrimitiveBrush2D &brush, const bbe::Rectanglei &rect) const
	{
		const bbe::Rectangle screenRect = selectionRectToScreen(rect);
		brush.setColorRGB(0.0f, 0.0f, 0.0f);
		brush.sketchRect(screenRect);
		if (screenRect.width > 2 && screenRect.height > 2)
		{
			brush.setColorRGB(1.0f, 1.0f, 1.0f);
			brush.sketchRect(screenRect.shrinked(1.0f));
		}
	}

	void clampBrushWidth()
	{
		if (brushWidth < 1) brushWidth = 1;
	}

	void clampTextFontSize()
	{
		if (textFontSize < 1) textFontSize = 1;
	}

	const bbe::Font &getTextToolFont() const
	{
		static std::map<int32_t, bbe::Font> textFonts;
		const int32_t clampedSize = bbe::Math::max<int32_t>(textFontSize, 1);
		auto it = textFonts.find(clampedSize);
		if (it == textFonts.end())
		{
			it = textFonts.emplace(clampedSize, bbe::Font("OpenSansRegular.ttf", (unsigned)clampedSize)).first;
		}
		return it->second;
	}

	const bbe::Image &getTextGlyphImage(const bbe::Font &font, int32_t codePoint) const
	{
		bbe::Image &glyph = const_cast<bbe::Image &>(font.getImage(codePoint, 1.0f));
		glyph.keepAfterUpload();
		return glyph;
	}

	bbe::String getTextBufferString() const
	{
		return bbe::String(textBuffer);
	}

	bool getTextOriginAndBounds(const bbe::Vector2i &topLeft, bbe::Vector2 &outOrigin, bbe::Rectangle &outBounds) const
	{
		const bbe::String text = getTextBufferString();
		if (text.isEmpty()) return false;

		const bbe::Font &font = getTextToolFont();
		outBounds = font.getBoundingBox(text);
		outOrigin = topLeft.as<float>() - outBounds.getPos();
		return true;
	}

	void blendGlyphOntoCanvas(const bbe::Image &glyph, const bbe::Vector2i &pos, const bbe::Colori &color)
	{
		for (int32_t x = 0; x < glyph.getWidth(); x++)
		{
			for (int32_t y = 0; y < glyph.getHeight(); y++)
			{
				int32_t targetX = pos.x + x;
				int32_t targetY = pos.y + y;
				if (tiled)
				{
					targetX = bbe::Math::mod<int32_t>(targetX, getCanvasWidth());
					targetY = bbe::Math::mod<int32_t>(targetY, getCanvasHeight());
				}
				else
				{
					if (targetX < 0 || targetY < 0 || targetX >= getCanvasWidth() || targetY >= getCanvasHeight()) continue;
				}

				const bbe::Colori glyphColor = glyph.getPixel((size_t)x, (size_t)y);
				if (glyphColor.r == 0) continue;

				bbe::Colori sourceColor = color;
				sourceColor.a = static_cast<bbe::byte>((uint32_t(color.a) * uint32_t(glyphColor.r)) / 255u);

				const bbe::Colori oldColor = getActiveLayerImage().getPixel((size_t)targetX, (size_t)targetY);
				getActiveLayerImage().setPixel((size_t)targetX, (size_t)targetY, oldColor.blendTo(sourceColor));
			}
		}
	}

	void placeTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color)
	{
		bbe::Vector2 origin;
		bbe::Rectangle bounds;
		if (!getTextOriginAndBounds(topLeft, origin, bounds)) return;

		const bbe::String text = getTextBufferString();
		const bbe::Font &font = getTextToolFont();
		const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(origin, text);

		bool changed = false;
		auto it = text.getIterator();
		for (size_t i = 0; i < renderPositions.getLength() && it.valid(); i++, ++it)
		{
			const int32_t codePoint = it.getCodepoint();
			if (codePoint == ' ' || codePoint == '\n' || codePoint == '\r' || codePoint == '\t') continue;

			const bbe::Vector2i glyphPos(
				(int32_t)bbe::Math::round(renderPositions[i].x),
				(int32_t)bbe::Math::round(renderPositions[i].y));
			blendGlyphOntoCanvas(getTextGlyphImage(font, codePoint), glyphPos, color);
			changed = true;
		}

		if (changed)
		{
			canvas.submit();
		}
	}

	void drawTextPreview(bbe::PrimitiveBrush2D &brush, const bbe::Vector2i &topLeft)
	{
		bbe::Vector2 origin;
		bbe::Rectangle bounds;
		if (!getTextOriginAndBounds(topLeft, origin, bounds)) return;

		const bbe::String text = getTextBufferString();
		const bbe::Font &font = getTextToolFont();
		const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(origin, text);
		const bbe::Color previewColor = bbe::Color(leftColor).blendTo(bbe::Color::white(), 0.15f);

		const int32_t tileDraw = tiled ? 20 : 0;
		for (int32_t ti = -tileDraw; ti <= tileDraw; ti++)
		{
			for (int32_t tk = -tileDraw; tk <= tileDraw; tk++)
			{
				const float tileOffX = ti * getCanvasWidth() * zoomLevel;
				const float tileOffY = tk * getCanvasHeight() * zoomLevel;

				brush.setColorRGB(previewColor);
				auto it = text.getIterator();
				for (size_t i = 0; i < renderPositions.getLength() && it.valid(); i++, ++it)
				{
					const int32_t codePoint = it.getCodepoint();
					if (codePoint == ' ' || codePoint == '\n' || codePoint == '\r' || codePoint == '\t') continue;

					const bbe::Image &glyph = getTextGlyphImage(font, codePoint);
					brush.drawImage(
						offset.x + tileOffX + renderPositions[i].x * zoomLevel,
						offset.y + tileOffY + renderPositions[i].y * zoomLevel,
						glyph.getWidth() * zoomLevel,
						glyph.getHeight() * zoomLevel,
						glyph);
				}

				drawSelectionOutline(brush, bbe::Rectanglei(
					topLeft.x + ti * getCanvasWidth(),
					topLeft.y + tk * getCanvasHeight(),
					(int32_t)bbe::Math::ceil(bounds.width),
					(int32_t)bbe::Math::ceil(bounds.height)));
			}
		}
	}

	void swapColors()
	{
		for (size_t i = 0; i < std::size(leftColor); i++)
		{
			std::swap(leftColor[i], rightColor[i]);
		}
	}

	void resetColorsToDefault()
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

	void serializeLayerImage(const bbe::Image &image, bbe::ByteBuffer &buffer) const
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

	bool deserializeLayerImage(bbe::ByteBufferSpan &span, int32_t width, int32_t height, bbe::Image &outImage) const
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

	bool saveLayeredDocument(const bbe::String &filePath)
	{
		commitFloatingSelection();

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
			PaintLayer &layer = canvas.get().layers[i];
			buffer.write(layer.visible);
			buffer.write(layer.name);
			serializeLayerImage(layer.image, buffer);
		}

		bbe::simpleFile::writeBinaryToFile(filePath, buffer);
		return true;
	}

	bool loadLayeredDocument(const bbe::String &filePath)
	{
		bbe::ByteBuffer buffer = bbe::simpleFile::readBinaryFile(filePath);
		if (buffer.getLength() == 0) return false;

		bbe::ByteBufferSpan span = buffer.getSpan();
		const bbe::String magic = span.readNullString();
		if (magic != LAYERED_FILE_MAGIC) return false;

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
			if (!deserializeLayerImage(span, width, height, layer.image))
			{
				return false;
			}
			document.layers.add(std::move(layer));
		}

		canvas.get() = std::move(document);
		activeLayerIndex = storedActiveLayerIndex;
		this->path = filePath;
		setupCanvas();
		clampActiveLayerIndex();
		return true;
	}

	bool saveFlattenedPng(const bbe::String &filePath)
	{
		commitFloatingSelection();
		flattenVisibleLayers().writeToFile(filePath);
		return true;
	}

	bool saveDocumentToPath(const bbe::String &filePath)
	{
		if (isLayeredDocumentPath(filePath))
		{
			return saveLayeredDocument(filePath);
		}
		return saveFlattenedPng(filePath);
	}

	void saveDocumentAs(SaveFormat format)
	{
		bbe::String newPath = path;
		const bbe::String defaultExtension = format == SaveFormat::PNG ? "png" : "bbepaint";
		if (bbe::simpleFile::showSaveDialog(newPath, defaultExtension))
		{
			path = newPath;
			saveDocumentToPath(path);
		}
	}

	void requestSave()
	{
		if (!path.isEmpty())
		{
			saveDocumentToPath(path);
			return;
		}

		openSaveChoicePopup = true;
	}

	void saveCanvas()
	{
		requestSave();
	}

	void resetCamera()
	{
		offset = bbe::Vector2(getWindowWidth() / 2 - getCanvasWidth() / 2, getWindowHeight() / 2 - getCanvasHeight() / 2);
		zoomLevel = 1.f;
	}

	void clearWorkArea()
	{
		workArea = bbe::Image(getCanvasWidth(), getCanvasHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		workArea.keepAfterUpload();
		workArea.setFilterMode(bbe::ImageFilterMode::NEAREST);
	}

	void applyWorkArea()
	{
		// TODO: This should probably be moved to the image class
		for (size_t i = 0; i < workArea.getWidth(); i++)
		{
			for (size_t k = 0; k < workArea.getHeight(); k++)
			{
				const bbe::Colori c = workArea.getPixel(i, k);
				if (c.a == 0) continue;

				const bbe::Colori oldColor = getActiveLayerImage().getPixel(i, k);
				const bbe::Colori newColor = oldColor.blendTo(c);
				getActiveLayerImage().setPixel(i, k, newColor);
			}
		}
		clearWorkArea();
	}

	void setupCanvas(bool clearHistory = true)
	{
		prepareDocumentImages();
		clearWorkArea();
		resetCamera();
		clearSelectionState();
		clampActiveLayerIndex();
		if (clearHistory) canvas.clearHistory();
	}

	void newCanvas(uint32_t width, uint32_t height)
	{
		canvas.get().layers.clear();
		canvas.get().layers.add(makeLayer("Layer 1", (int32_t)width, (int32_t)height, bbe::Color::white()));
		activeLayerIndex = 0;
		this->path = "";
		setupCanvas();
	}

	bool newCanvas(const char *path)
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
		canvas.get().layers.add(PaintLayer{ "Layer 1", true, bbe::Image(path) });
		activeLayerIndex = 0;
		this->path = path;
		setupCanvas();
		return true;
	}

	bbe::Vector2 screenToCanvas(const bbe::Vector2 &pos)
	{
		return (pos - offset) / zoomLevel;
	}

	bool toTiledPos(bbe::Vector2 &pos)
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

	void changeZoom(float val)
	{
		auto mouseBeforeZoom = screenToCanvas(getMouse());
		zoomLevel *= val;
		auto mouseAfterZoom = screenToCanvas(getMouse());
		offset += (mouseAfterZoom - mouseBeforeZoom) * zoomLevel;
	}

	bbe::Colori getMouseColor() const
	{
		if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT)) return bbe::Color(leftColor).asByteColor();
		return isMouseDown(bbe::MouseButton::LEFT) ? bbe::Color(leftColor).asByteColor() : bbe::Color(rightColor).asByteColor();
	}

	bool touch(const bbe::Vector2 &touchPos, bool rectangleShape = false)
	{
		return touchImage(workArea, touchPos, getMouseColor(), brushWidth, rectangleShape, tiled);
	}

	bool touchLine(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, bool rectangleShape = false)
	{
		return touchLineImage(workArea, pos1, pos2, getMouseColor(), brushWidth, rectangleShape, tiled);
	}

	virtual void onStart() override
	{
		newCanvas(400, 300);
	}
	virtual void onFilesDropped(const bbe::List<bbe::String> &paths) override
	{
		for (size_t i = 0; i < paths.getLength(); i++)
		{
			if (!isSupportedDroppedDocumentPath(paths[i])) continue;
			newCanvas(paths[i].getRaw());
			break;
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		static bool changeRegistered = false;
		static int32_t previousMode = mode;
		const bool drawButtonDown = isMouseDown(bbe::MouseButton::LEFT) || isMouseDown(bbe::MouseButton::RIGHT);
		auto discardTransientWorkArea = [&]()
		{
			clearWorkArea();
			changeRegistered = false;
		};
		if (mode != previousMode && !drawButtonDown)
		{
			if (previousMode == MODE_RECTANGLE && rectangleDraftActive)
			{
				commitFloatingSelection();
				clearSelectionState();
			}
			if (previousMode == MODE_CIRCLE && circleDraftActive)
			{
				commitFloatingSelection();
				clearSelectionState();
			}
			discardTransientWorkArea();
		}

		const bbe::Vector2 prevMousePos = screenToCanvas(getMousePrevious());
		const bool ctrlDown = isKeyDown(bbe::Key::LEFT_CONTROL) || isKeyDown(bbe::Key::RIGHT_CONTROL);
		const int32_t modeBeforeInput = mode;
		bool refreshRectangleDraft = false;
		if (isKeyPressed(bbe::Key::SPACE))
		{
			resetCamera();
		}

		constexpr float CAM_WASD_SPEED = 400;
		if (!ctrlDown && isKeyDown(bbe::Key::W))
		{
			offset.y += timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (!ctrlDown && isKeyDown(bbe::Key::S))
		{
			offset.y -= timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (!ctrlDown && isKeyDown(bbe::Key::A))
		{
			offset.x += timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (!ctrlDown && isKeyDown(bbe::Key::D))
		{
			offset.x -= timeSinceLastFrame * CAM_WASD_SPEED;
		}

		if (isMouseDown(bbe::MouseButton::MIDDLE))
		{
			offset += getMouseDelta();
			if (tiled)
			{
				if (offset.x < 0) offset.x += getCanvasWidth() * zoomLevel;
				if (offset.y < 0) offset.y += getCanvasHeight() * zoomLevel;
				if (offset.x > getCanvasWidth() * zoomLevel) offset.x -= getCanvasWidth() * zoomLevel;
				if (offset.y > getCanvasHeight() * zoomLevel) offset.y -= getCanvasHeight() * zoomLevel;
			}
		}

		if (getMouseScrollY() < 0)
		{
			changeZoom(1.0f / 1.1f);
		}
		else if (getMouseScrollY() > 0)
		{
			changeZoom(1.1f);
		}
		const bbe::Vector2 currMousePos = screenToCanvas(getMouse());

		if (isKeyPressed(bbe::Key::_1))
		{
			mode = MODE_BRUSH;
		}
		if (isKeyPressed(bbe::Key::_2))
		{
			mode = MODE_FLOOD_FILL;
		}
		if (isKeyPressed(bbe::Key::_3))
		{
			mode = MODE_LINE;
		}
		if (isKeyPressed(bbe::Key::_4))
		{
			mode = MODE_RECTANGLE;
		}
		if (isKeyPressed(bbe::Key::_5))
		{
			mode = MODE_SELECTION;
		}
		if (isKeyPressed(bbe::Key::_6))
		{
			mode = MODE_TEXT;
		}
		if (isKeyPressed(bbe::Key::_7))
		{
			mode = MODE_PIPETTE;
		}
		if (isKeyPressed(bbe::Key::_8))
		{
			mode = MODE_CIRCLE;
		}
		bool refreshCircleDraft = false;
		if (!ctrlDown && isKeyPressed(bbe::Key::X))
		{
			swapColors();
			refreshRectangleDraft = rectangleDraftActive;
			refreshCircleDraft = circleDraftActive;
		}
		const bool increaseToolSize = isKeyTyped(bbe::Key::EQUAL)
			|| isKeyTyped(bbe::Key::KP_ADD)
			|| isKeyTyped(bbe::Key::RIGHT_BRACKET);
		const bool decreaseToolSize = isKeyTyped(bbe::Key::MINUS) || isKeyTyped(bbe::Key::KP_SUBTRACT);
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE || mode == MODE_CIRCLE)
		{
			const int32_t previousBrushWidth = brushWidth;
			if (increaseToolSize) brushWidth++;
			if (decreaseToolSize) brushWidth--;
			clampBrushWidth();
			if (rectangleDraftActive && brushWidth != previousBrushWidth)
			{
				refreshRectangleDraft = true;
			}
			if (circleDraftActive && brushWidth != previousBrushWidth)
			{
				refreshCircleDraft = true;
			}
		}
		else if (mode == MODE_TEXT)
		{
			if (increaseToolSize) textFontSize++;
			if (decreaseToolSize) textFontSize--;
			clampTextFontSize();
		}

		if (ctrlDown)
		{
			if (isKeyTyped(bbe::Key::Z) && canvas.isUndoable())
			{
				canvas.undo();
				clampActiveLayerIndex();
				clearSelectionState();
				clearWorkArea();
			}
			if (isKeyTyped(bbe::Key::Y) && canvas.isRedoable())
			{
				canvas.redo();
				clampActiveLayerIndex();
				clearSelectionState();
				clearWorkArea();
			}
			if (isKeyPressed(bbe::Key::S))
			{
				saveCanvas();
			}
			if (isKeyPressed(bbe::Key::D))
			{
				resetColorsToDefault();
				refreshRectangleDraft = rectangleDraftActive;
				refreshCircleDraft = circleDraftActive;
			}
			if (isKeyPressed(bbe::Key::A))
			{
				selectWholeLayer();
			}
			if (isKeyPressed(bbe::Key::C))
			{
				if (mode != MODE_SELECTION)
				{
					mode = MODE_SELECTION;
				}
				else
				{
					storeSelectionInClipboard();
				}
			}
			if (isKeyPressed(bbe::Key::X))
			{
				cutSelection();
			}
			if (isKeyPressed(bbe::Key::V))
			{
				pasteSelectionAt(toCanvasPixel(currMousePos));
			}
		}
		if (mode != modeBeforeInput && !drawButtonDown)
		{
			if (modeBeforeInput == MODE_RECTANGLE && rectangleDraftActive)
			{
				commitFloatingSelection();
				clearSelectionState();
			}
			if (modeBeforeInput == MODE_CIRCLE && circleDraftActive)
			{
				commitFloatingSelection();
				clearSelectionState();
			}
			discardTransientWorkArea();
		}
		previousMode = mode;
		if (refreshRectangleDraft)
		{
			refreshActiveRectangleDraftImage();
		}
		if (refreshCircleDraft)
		{
			refreshActiveCircleDraftImage();
		}
		if (selectionFloating && mode != MODE_SELECTION && !(mode == MODE_RECTANGLE && rectangleDraftActive) && !(mode == MODE_CIRCLE && circleDraftActive))
		{
			commitFloatingSelection();
		}
		if (isKeyPressed(bbe::Key::DELETE) || isKeyPressed(bbe::Key::BACKSPACE))
		{
			deleteSelection();
		}

		if (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT))
		{
			startMousePos = screenToCanvas(getMouse());
		}

		if (mode == MODE_SELECTION)
		{
			updateSelectionTool(currMousePos);
		}
		if (mode == MODE_RECTANGLE)
		{
			updateRectangleTool(currMousePos);
		}
		if (mode == MODE_CIRCLE)
		{
			updateCircleTool(currMousePos);
		}
		if (mode == MODE_TEXT && (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT)))
		{
			bbe::Vector2 pos = currMousePos;
			if (toTiledPos(pos))
			{
				placeTextAt(toCanvasPixel(pos), getMouseColor());
			}
		}

		// TODO: Would be nice if we had a constexpr list
		const bbe::List<decltype(mode)> shadowDrawModes = { MODE_BRUSH };
		const bool drawMode = mode != MODE_SELECTION
			&& mode != MODE_TEXT
			&& mode != MODE_RECTANGLE
			&& mode != MODE_CIRCLE
			&& drawButtonDown;
		const bool shadowDrawMode = shadowDrawModes.contains(mode);

		if (changeRegistered)
		{
			if (isMouseReleased(bbe::MouseButton::LEFT) || isMouseReleased(bbe::MouseButton::RIGHT))
			{
				if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT))
				{
					applyWorkArea();
					canvas.submit(); // <- changeRegistered is for this
					changeRegistered = false;
				}
			}
		}

		if (drawMode || shadowDrawMode)
		{
			static uint32_t counter = 0;
			if (!drawMode)
			{
				counter++;
				if (counter > 1) clearWorkArea();
			}
			else
			{
				if (counter > 0) clearWorkArea();
				counter = 0;
			}

			if (mode == MODE_BRUSH)
			{
				const bool touched = touchLine(currMousePos, prevMousePos);
				if (drawMode)
				{
					changeRegistered |= touched;
				}
			}
			else if (mode == MODE_FLOOD_FILL)
			{
				bbe::Vector2 pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					getActiveLayerImage().floodFill(pos.as<int32_t>(), getMouseColor(), false, tiled);
					changeRegistered = true;
				}
			}
			else if (mode == MODE_LINE)
			{
				clearWorkArea();
				changeRegistered |= touchLine(currMousePos, startMousePos);
			}
			else if (mode == MODE_PIPETTE)
			{
				auto pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					const size_t x = (size_t)pos.x;
					const size_t y = (size_t)pos.y;
					const bbe::Colori color = getVisiblePixel(x, y);
					if (isMouseDown(bbe::MouseButton::LEFT))
					{
						leftColor[0] = color.r / 255.f;
						leftColor[1] = color.g / 255.f;
						leftColor[2] = color.b / 255.f;
						leftColor[3] = color.a / 255.f;
					}
					if (isMouseDown(bbe::MouseButton::RIGHT))
					{
						rightColor[0] = color.r / 255.f;
						rightColor[1] = color.g / 255.f;
						rightColor[2] = color.b / 255.f;
						rightColor[3] = color.a / 255.f;
					}
				}
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
		}

	}
	virtual void draw3D(bbe::PrimitiveBrush3D &brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D &brush) override
	{
		const float PANEL_WIDTH = 260.f * bbe::Math::sqrt(getWindow()->getScale());
		ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, (float)getWindowHeight() - ImGui::GetFrameHeight()), ImGuiCond_Always);
		ImGui::Begin("##panel", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove);

		// --- Undo / Redo ---
		const float halfW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		ImGui::BeginDisabled(!canvas.isUndoable());
		if (ImGui::Button("Undo", ImVec2(halfW, 0)))
		{
			canvas.undo();
			clampActiveLayerIndex();
			clearSelectionState();
			clearWorkArea();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!canvas.isRedoable());
		if (ImGui::Button("Redo", ImVec2(halfW, 0)))
		{
			canvas.redo();
			clampActiveLayerIndex();
			clearSelectionState();
			clearWorkArea();
		}
		ImGui::EndDisabled();

		// --- Colors ---
		ImGui::SeparatorText("Colors");
		const bool leftColorChanged  = ImGui::ColorEdit4("Primary",   leftColor);
		const bool rightColorChanged = ImGui::ColorEdit4("Secondary", rightColor);
		if (rectangleDraftActive && ((leftColorChanged && !rectangleDraftUsesRightColor) || (rightColorChanged && rectangleDraftUsesRightColor)))
		{
			refreshActiveRectangleDraftImage();
		}
		if (circleDraftActive && ((leftColorChanged && !circleDraftUsesRightColor) || (rightColorChanged && circleDraftUsesRightColor)))
		{
			refreshActiveCircleDraftImage();
		}

		// --- Tool ---
		ImGui::SeparatorText("Tool");
		{
			const float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			auto toolBtn = [&](const char* label, int32_t toolMode)
			{
				const bool active = (mode == toolMode);
				if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(label, ImVec2(w, 0))) mode = toolMode;
				if (active) ImGui::PopStyleColor();
			};
			toolBtn("Brush",     MODE_BRUSH);      ImGui::SameLine();
			toolBtn("Fill",      MODE_FLOOD_FILL);
			toolBtn("Line",      MODE_LINE);        ImGui::SameLine();
			toolBtn("Rectangle", MODE_RECTANGLE);
			toolBtn("Circle",    MODE_CIRCLE);      ImGui::SameLine();
			toolBtn("Selection", MODE_SELECTION);
			toolBtn("Text",      MODE_TEXT);        ImGui::SameLine();
			toolBtn("Pipette",   MODE_PIPETTE);
		}

		// --- Tool options ---
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE || mode == MODE_CIRCLE || mode == MODE_TEXT)
		{
			ImGui::SeparatorText("Options");
		}
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE || mode == MODE_CIRCLE)
		{
			if (ImGui::InputInt("Width", &brushWidth))
			{
				clampBrushWidth();
				if (rectangleDraftActive) refreshActiveRectangleDraftImage();
				if (circleDraftActive) refreshActiveCircleDraftImage();
			}
		}
		if (mode == MODE_RECTANGLE)
		{
			if (ImGui::Checkbox("Round Edges", &roundEdges) && rectangleDraftActive)
			{
				refreshActiveRectangleDraftImage();
			}
			ImGui::TextDisabled(rectangleDraftActive
				? "Drag inside/border to move/resize.\nClick outside to place."
				: "Drag to draw. Click outside to place.");
		}
		if (mode == MODE_CIRCLE)
		{
			ImGui::TextDisabled(circleDraftActive
				? "Drag inside/border to move/resize.\nClick outside to place."
				: "Drag to draw. Click outside to place.");
		}
		if (mode == MODE_TEXT)
		{
			if (ImGui::InputInt("Font Size", &textFontSize))
			{
				clampTextFontSize();
			}
			ImGui::InputTextMultiline("##text", textBuffer, sizeof(textBuffer), ImVec2(-1, ImGui::GetTextLineHeight() * 4.0f));
			ImGui::TextDisabled("L/R click places text.");
		}

		// --- Selection actions ---
		if (mode == MODE_SELECTION)
		{
			ImGui::SeparatorText("Selection");
			ImGui::BeginDisabled(!hasSelection);
			if (ImGui::Button("Copy",   ImVec2(-1, 0))) storeSelectionInClipboard();
			if (ImGui::Button("Cut",    ImVec2(-1, 0))) cutSelection();
			if (ImGui::Button("Delete", ImVec2(-1, 0))) deleteSelection();
			ImGui::EndDisabled();
			if (hasSelection)
				ImGui::Text("%d x %d px", selectionRect.width, selectionRect.height);
			else
				ImGui::TextDisabled("No selection");
		}

		// --- Clipboard ---
		const bool supportsClipboardImages = bbe::Image::supportsClipboardImages();
		ImGui::SeparatorText("Clipboard");
		ImGui::BeginDisabled(!supportsClipboardImages);
		if (ImGui::Button("Copy Canvas to Clipboard", ImVec2(-1, 0)))
		{
			flattenVisibleLayers().copyToClipboard();
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!supportsClipboardImages || !bbe::Image::isImageInClipbaord());
		if (ImGui::Button("Paste as New Canvas", ImVec2(-1, 0)))
		{
			canvas.get().layers.clear();
			canvas.get().layers.add(PaintLayer{ "Layer 1", true, bbe::Image::getClipboardImage() });
			path = "";
			canvas.submit();
			setupCanvas(false);
		}
		ImGui::EndDisabled();
		if (!supportsClipboardImages)
			ImGui::TextDisabled("Not supported on this platform");

		// --- Layers ---
		ImGui::SeparatorText("Layers");
		{
			const float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3) * 0.25f;
			if (ImGui::Button("+ New", ImVec2(btnW * 1.5f, 0))) addLayer();
			ImGui::SameLine();
			ImGui::BeginDisabled(canvas.get().layers.getLength() <= 1);
			if (ImGui::Button("- Del", ImVec2(btnW * 1.5f, 0))) deleteActiveLayer();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled((size_t)activeLayerIndex + 1 >= canvas.get().layers.getLength());
			if (ImGui::Button("Up", ImVec2(btnW, 0))) moveActiveLayerUp();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(activeLayerIndex <= 0);
			if (ImGui::Button("Dn", ImVec2(btnW, 0))) moveActiveLayerDown();
			ImGui::EndDisabled();
		}
		if (!canvas.get().layers.isEmpty())
		{
			if (ImGui::bbe::InputText("Name##layerName", getActiveLayer().name))
			{
				canvas.submit();
			}
		}
		if (ImGui::BeginChild("##layerList", ImVec2(-1, ImGui::GetContentRegionAvail().y), true))
		{
			for (int32_t layerIndex = (int32_t)canvas.get().layers.getLength() - 1; layerIndex >= 0; layerIndex--)
			{
				PaintLayer &layer = canvas.get().layers[(size_t)layerIndex];
				ImGui::PushID(layerIndex);
				bool visible = layer.visible;
				if (ImGui::Checkbox("##vis", &visible))
				{
					layer.visible = visible;
					canvas.submit();
				}
				ImGui::SameLine();
				if (ImGui::Selectable(layer.name.getRaw(), activeLayerIndex == layerIndex))
				{
					setActiveLayerIndex(layerIndex);
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		ImGui::End();

		const int32_t repeats = tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				for (size_t layerIndex = 0; layerIndex < canvas.get().layers.getLength(); layerIndex++)
				{
					const PaintLayer &layer = canvas.get().layers[layerIndex];
					if (!layer.visible) continue;
					brush.drawImage(offset.x + i * getCanvasWidth() * zoomLevel, offset.y + k * getCanvasHeight() * zoomLevel, getCanvasWidth() * zoomLevel, getCanvasHeight() * zoomLevel, layer.image);
				}
				brush.drawImage(offset.x + i * getCanvasWidth() * zoomLevel, offset.y + k * getCanvasHeight() * zoomLevel, getCanvasWidth() * zoomLevel, getCanvasHeight() * zoomLevel, workArea);
			}
		}
		if (zoomLevel > 3 && drawGridLines)
		{
			bbe::Vector2 zeroPos = screenToCanvas({ 0, 0 });
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.5f);
			for (float i = -(zeroPos.x - (int)zeroPos.x) * zoomLevel; i < getWindowWidth(); i += zoomLevel)
			{
				brush.fillLine(i, 0, i, getWindowHeight());
			}
			for (float i = -(zeroPos.y - (int)zeroPos.y) * zoomLevel; i < getWindowHeight(); i += zoomLevel)
			{
				brush.fillLine(0, i, getWindowWidth(), i);
			}
		}
		const int32_t ghostRepeats = tiled ? 20 : 0;
		auto drawInAllTiles = [&](const bbe::Rectanglei &rect, const bbe::Image &image)
		{
			for (int32_t i = -ghostRepeats; i <= ghostRepeats; i++)
			{
				for (int32_t k = -ghostRepeats; k <= ghostRepeats; k++)
				{
					const bbe::Rectanglei offsetRect(
						rect.x + i * getCanvasWidth(),
						rect.y + k * getCanvasHeight(),
						rect.width,
						rect.height);
					brush.drawImage(selectionRectToScreen(offsetRect), image);
					drawSelectionOutline(brush, offsetRect);
				}
			}
		};

		if (rectangleDragActive && rectangleDragPreviewRect.width > 0 && rectangleDragPreviewRect.height > 0)
		{
			drawInAllTiles(rectangleDragPreviewRect, rectangleDragPreviewImage);
		}
		else if (circleDragActive && circleDragPreviewRect.width > 0 && circleDragPreviewRect.height > 0)
		{
			drawInAllTiles(circleDragPreviewRect, circleDragPreviewImage);
		}
		else if (selectionMoveActive || selectionResizeActive)
		{
			const bbe::Image previewImage = selectionResizeActive ? buildSelectionPreviewResultImage() : selectionPreviewImage;
			const bool isDraft = rectangleDraftActive || circleDraftActive;
			if (isDraft)
				drawInAllTiles(selectionPreviewRect, previewImage);
			else
			{
				brush.drawImage(selectionRectToScreen(selectionPreviewRect), previewImage);
				drawSelectionOutline(brush, selectionPreviewRect);
			}
		}
		else if (selectionFloating)
		{
			const bool isDraft = rectangleDraftActive || circleDraftActive;
			if (isDraft)
				drawInAllTiles(selectionRect, selectionFloatingImage);
			else
			{
				brush.drawImage(selectionRectToScreen(selectionRect), selectionFloatingImage);
				drawSelectionOutline(brush, selectionRect);
			}
		}
		else if (selectionDragActive)
		{
			drawSelectionOutline(brush, selectionPreviewRect);
		}
		else if (hasSelection)
		{
			drawSelectionOutline(brush, selectionRect);
		}
		if (mode == MODE_TEXT)
		{
			bbe::Vector2 previewPos = screenToCanvas(getMouse());
			if (toTiledPos(previewPos))
			{
				drawTextPreview(brush, toCanvasPixel(previewPos));
			}
		}

		// HACK: We can only open popups if we are in the same ID Stack. See: https://github.com/ocornut/imgui/issues/331
		bool openNewCanvas = false;
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
					bbe::String newPath = path;
					if (bbe::simpleFile::showOpenDialog(newPath))
					{
						newCanvas(newPath.getRaw());
					}
				}
				if (ImGui::MenuItem("Save"))
				{
					saveCanvas();
				}
				if (ImGui::MenuItem("Save As PNG..."))
				{
					saveDocumentAs(SaveFormat::PNG);
				}
				if (ImGui::MenuItem("Save As Layered..."))
				{
					saveDocumentAs(SaveFormat::LAYERED);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Draw Grid Lines", nullptr, drawGridLines))
				{
					drawGridLines = !drawGridLines;
				}
				if (ImGui::MenuItem("Tiled", nullptr, tiled))
				{
					tiled = !tiled;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Show Help"))
				{
					showHelpWindow = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (openSaveChoicePopup)
		{
			ImGui::OpenPopup("Save Document");
			openSaveChoicePopup = false;
		}
		if (ImGui::BeginPopupModal("Save Document", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Choose a save format for this document.");
			if (ImGui::Button("PNG", ImVec2(120, 0)))
			{
				saveDocumentAs(SaveFormat::PNG);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Layered", ImVec2(120, 0)))
			{
				saveDocumentAs(SaveFormat::LAYERED);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (showHelpWindow)
		{
			if (ImGui::Begin("ExamplePaint Help", &showHelpWindow))
			{
				ImGui::SeparatorText("Tools");
				ImGui::BulletText("1 Brush");
				ImGui::BulletText("2 Flood Fill");
				ImGui::BulletText("3 Line");
				ImGui::BulletText("4 Rectangle");
				ImGui::BulletText("5 Selection");
				ImGui::BulletText("6 Text");
				ImGui::BulletText("7 Pipette");
				ImGui::BulletText("8 Circle");

				ImGui::SeparatorText("General");
				ImGui::BulletText("+/- changes brush size or text size for the active tool");
				ImGui::BulletText("X swaps primary and secondary color");
				ImGui::BulletText("Ctrl+D resets colors to black/white");
				ImGui::BulletText("Drag and drop PNG or .bbepaint files into the window to open them");
				ImGui::BulletText("Space resets the camera");
				ImGui::BulletText("Middle mouse pans");
				ImGui::BulletText("Mouse wheel zooms");

				ImGui::SeparatorText("Edit");
				ImGui::BulletText("Ctrl+S saves");
				ImGui::BulletText("Ctrl+Z / Ctrl+Y undo and redo");
				ImGui::BulletText("Delete / Backspace deletes the current selection");

				ImGui::SeparatorText("Selection");
				ImGui::BulletText("Drag to create a rectangular selection");
				ImGui::BulletText("Drag inside a selection to move it");
				ImGui::BulletText("Drag the selection border to resize it");
				ImGui::BulletText("Rectangle creates a floating selection first; click outside to place it");
				ImGui::BulletText("Ctrl+A selects the whole active layer");
				ImGui::BulletText("Ctrl+C / Ctrl+X / Ctrl+V copy, cut and paste");

				ImGui::SeparatorText("Layers");
				ImGui::BulletText("Painting and text placement affect only the active layer");
				ImGui::BulletText("Visible layers are flattened when saving as PNG");
				ImGui::BulletText("Save as Layered keeps all layers in .bbepaint");
				ImGui::BulletText("Opening PNG still works as a normal single-layer document");
			}
			ImGui::End();
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		static int newWidth = 0;
		static int newHeight = 0;
		if (openNewCanvas)
		{
			ImGui::OpenPopup("New Canvas");
			newWidth = getCanvasWidth();
			newHeight = getCanvasHeight();
		}
		if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputInt("Width", &newWidth);
			ImGui::SameLine();
			ImGui::InputInt("Height", &newHeight);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				newCanvas(newWidth, newHeight);
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

	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "ExamplePaint");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
