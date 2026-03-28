
#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.

// TODO: Proper GUI
// TODO: Drag and Drop image files into paint
// TODO: Circle tool
// TODO: Flood fill with edges of brush tool kinda bad.
// TODO: Bug: right click has weird behaviour with shadow

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
	bbe::Rectanglei selectionPreviewRect;
	bbe::Image selectionPreviewImage;

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
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	void selectWholeLayer()
	{
		if (selectionFloating)
		{
			commitFloatingSelection();
		}
		if (getCanvasWidth() <= 0 || getCanvasHeight() <= 0) return;

		mode = MODE_SELECTION;
		hasSelection = true;
		selectionRect = bbe::Rectanglei(0, 0, getCanvasWidth(), getCanvasHeight());
		selectionFloating = false;
		selectionFloatingImage = {};
		selectionDragActive = false;
		selectionDragStart = {};
		selectionMoveActive = false;
		selectionMoveOffset = {};
		selectionPreviewRect = {};
		selectionPreviewImage = {};
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

	void blendImageOntoImage(bbe::Image &target, const bbe::Image &image, const bbe::Vector2i &pos) const
	{
		for (int32_t x = 0; x < image.getWidth(); x++)
		{
			for (int32_t y = 0; y < image.getHeight(); y++)
			{
				const int32_t targetX = pos.x + x;
				const int32_t targetY = pos.y + y;
				if (targetX < 0 || targetY < 0 || targetX >= target.getWidth() || targetY >= target.getHeight()) continue;

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

	bool buildSelectionRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
	{
		const int32_t left = bbe::Math::min(pos1.x, pos2.x);
		const int32_t top = bbe::Math::min(pos1.y, pos2.y);
		const int32_t right = bbe::Math::max(pos1.x, pos2.x);
		const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
		return clampRectToCanvas(bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1), outRect);
	}

	bool isPointInSelection(const bbe::Vector2i &point) const
	{
		return hasSelection && selectionRect.isPointInRectangle(point, true);
	}

	bool isWholeLayerSelection(const bbe::Rectanglei &rect) const
	{
		return rect.x == 0
			&& rect.y == 0
			&& rect.width == getCanvasWidth()
			&& rect.height == getCanvasHeight();
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
		const bbe::Colori backgroundColor = isWholeLayerSelection(rect) ? bbe::Colori(0, 0, 0, 0) : bbe::Color(rightColor).asByteColor();
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
		blendImageOntoImage(getActiveLayerImage(), image, pos);
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
		selectionMoveActive = false;
		selectionDragActive = false;
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	void commitFloatingSelection()
	{
		if (!selectionFloating) return;

		blendImageOntoCanvas(selectionFloatingImage, selectionRect.getPos());
		canvas.submit();

		bbe::Rectanglei clampedRect;
		if (!clampRectToCanvas(bbe::Rectanglei(selectionRect.x, selectionRect.y, selectionFloatingImage.getWidth(), selectionFloatingImage.getHeight()), clampedRect))
		{
			clearSelectionState();
			return;
		}

		selectionRect = clampedRect;
		selectionFloating = false;
		selectionFloatingImage = {};
	}

	void applySelectionMove()
	{
		if (!selectionMoveActive) return;

		if (selectionFloating)
		{
			selectionRect = selectionPreviewRect;
			selectionMoveActive = false;
			selectionPreviewRect = {};
			selectionPreviewImage = {};
			return;
		}

		if (selectionPreviewRect.x != selectionRect.x || selectionPreviewRect.y != selectionRect.y)
		{
			clearCanvasRect(selectionRect);
			selectionRect = selectionPreviewRect;
			selectionFloating = true;
			selectionFloatingImage = selectionPreviewImage;
			hasSelection = true;
		}

		selectionMoveActive = false;
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	void updateSelectionTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2i mousePixel = toCanvasPixel(currMousePos);

		if (isMousePressed(bbe::MouseButton::LEFT))
		{
			if (isPointInSelection(mousePixel))
			{
				selectionMoveActive = true;
				selectionMoveOffset = mousePixel - selectionRect.getPos();
				selectionPreviewRect = selectionRect;
				selectionPreviewImage = selectionFloating ? selectionFloatingImage : copyCanvasRect(selectionRect);
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
				selectionPreviewRect = bbe::Rectanglei(
					mousePixel.x - selectionMoveOffset.x,
					mousePixel.y - selectionMoveOffset.y,
					selectionPreviewImage.getWidth(),
					selectionPreviewImage.getHeight());
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

			if (selectionMoveActive)
			{
				applySelectionMove();
			}
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
				const int32_t targetX = pos.x + x;
				const int32_t targetY = pos.y + y;
				if (targetX < 0 || targetY < 0 || targetX >= getCanvasWidth() || targetY >= getCanvasHeight()) continue;

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
		const bbe::Color previewBase = isMouseDown(bbe::MouseButton::RIGHT) ? bbe::Color(rightColor) : bbe::Color(leftColor);
		const bbe::Color previewColor = previewBase.blendTo(bbe::Color::white(), 0.15f);
		brush.setColorRGB(previewColor);

		auto it = text.getIterator();
		for (size_t i = 0; i < renderPositions.getLength() && it.valid(); i++, ++it)
		{
			const int32_t codePoint = it.getCodepoint();
			if (codePoint == ' ' || codePoint == '\n' || codePoint == '\r' || codePoint == '\t') continue;

			const bbe::Image &glyph = getTextGlyphImage(font, codePoint);
			brush.drawImage(
				offset.x + renderPositions[i].x * zoomLevel,
				offset.y + renderPositions[i].y * zoomLevel,
				glyph.getWidth() * zoomLevel,
				glyph.getHeight() * zoomLevel,
				glyph);
		}

		drawSelectionOutline(brush, bbe::Rectanglei(
			topLeft.x,
			topLeft.y,
			(int32_t)bbe::Math::ceil(bounds.width),
			(int32_t)bbe::Math::ceil(bounds.height)));
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

	void setupCanvas()
	{
		prepareDocumentImages();
		clearWorkArea();
		resetCamera();
		clearSelectionState();
		clampActiveLayerIndex();
		canvas.clearHistory();
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
		bool changeRegistered = false;
		for (int32_t i = -brushWidth; i <= brushWidth; i++)
		{
			for (int32_t k = -brushWidth; k <= brushWidth; k++)
			{
				float pencilStrength = bbe::Math::clamp01(brushWidth - bbe::Math::sqrt(i * i + k * k));
				if (rectangleShape)
				{
					if (i != -brushWidth && i != brushWidth && k != -brushWidth && k != brushWidth)
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
				if (toTiledPos(coord))
				{
					bbe::Colori newColor = getMouseColor();
					newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
					bbe::Colori oldWorkColor = workArea.getPixel(coord.x, coord.y);
					if (newColor.a > oldWorkColor.a)
					{
						workArea.setPixel(coord.x, coord.y, newColor);
						changeRegistered = true;
					}
				}
			}
		}
		return changeRegistered;
	}

	bool touchLine(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, bool rectangleShape = false)
	{
		bool changeRegistered = false;
		bbe::GridIterator gi(pos1, pos2);
		while (gi.hasNext())
		{
			const bbe::Vector2 coordBase = gi.next().as<float>();
			changeRegistered |= touch(coordBase, rectangleShape);
		}
		return changeRegistered;
	}

	virtual void onStart() override
	{
		newCanvas(400, 300);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		const bbe::Vector2 prevMousePos = screenToCanvas(getMousePrevious());
		const bool ctrlDown = isKeyDown(bbe::Key::LEFT_CONTROL) || isKeyDown(bbe::Key::RIGHT_CONTROL);
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
		if (!ctrlDown && isKeyPressed(bbe::Key::X))
		{
			swapColors();
		}
		const bool increaseToolSize = isKeyTyped(bbe::Key::EQUAL)
			|| isKeyTyped(bbe::Key::KP_ADD)
			|| isKeyTyped(bbe::Key::RIGHT_BRACKET);
		const bool decreaseToolSize = isKeyTyped(bbe::Key::MINUS) || isKeyTyped(bbe::Key::KP_SUBTRACT);
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE)
		{
			if (increaseToolSize) brushWidth++;
			if (decreaseToolSize) brushWidth--;
			clampBrushWidth();
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
			}
			if (isKeyPressed(bbe::Key::A))
			{
				selectWholeLayer();
			}
			if (isKeyPressed(bbe::Key::C))
			{
				storeSelectionInClipboard();
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
		if (selectionFloating && mode != MODE_SELECTION)
		{
			commitFloatingSelection();
		}
		if (isKeyPressed(bbe::Key::DELETE) || isKeyPressed(bbe::Key::BACKSPACE))
		{
			deleteSelection();
		}

		static bool changeRegistered = false;
		if (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT))
		{
			startMousePos = screenToCanvas(getMouse());
		}

		if (mode == MODE_SELECTION)
		{
			updateSelectionTool(currMousePos);
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
		const bool drawMode = mode != MODE_SELECTION && mode != MODE_TEXT && (isMouseDown(bbe::MouseButton::LEFT) || isMouseDown(bbe::MouseButton::RIGHT));

		if (drawMode || shadowDrawModes.contains(mode))
		{
			// This counter is a bit of a hack. Problem is if we wouldn't have it and immediately call clearWorkArea then the workArea would never get applied.
			// Applying the work area could be moved before this if, but then we might lose a frame of "work" which might feel odd.
			static uint32_t counter = 0;
			if (!drawMode)
			{
				counter++;
				if (counter > 1) clearWorkArea();
			}
			else
			{
				counter = 0;
			}

			if (mode == MODE_BRUSH)
			{
				changeRegistered |= touchLine(currMousePos, prevMousePos);
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
			else if (mode == MODE_RECTANGLE)
			{
				clearWorkArea();
				const bbe::Vector2 topLeft = startMousePos;
				const bbe::Vector2 topRight = bbe::Vector2(currMousePos.x, startMousePos.y);
				const bbe::Vector2 bottomLeft = bbe::Vector2(startMousePos.x, currMousePos.y);
				const bbe::Vector2 bottomRight = currMousePos;
				changeRegistered |= touchLine(topLeft, topRight, !roundEdges);
				changeRegistered |= touchLine(topRight, bottomRight, !roundEdges);
				changeRegistered |= touchLine(bottomRight, bottomLeft, !roundEdges);
				changeRegistered |= touchLine(bottomLeft, topLeft, !roundEdges);
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
	}
	virtual void draw3D(bbe::PrimitiveBrush3D &brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D &brush) override
	{
		ImGui::BeginDisabled(!canvas.isUndoable());
		if (ImGui::Button("Undo"))
		{
			canvas.undo();
			clampActiveLayerIndex();
			clearSelectionState();
			clearWorkArea();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!canvas.isRedoable());
		if (ImGui::Button("Redo"))
		{
			canvas.redo();
			clampActiveLayerIndex();
			clearSelectionState();
			clearWorkArea();
		}
		ImGui::EndDisabled();

		ImGui::ColorEdit4("Left Color", leftColor);
		ImGui::ColorEdit4("Right Color", rightColor);
		ImGui::bbe::combo("Mode", { "Brush", "Flood fill", "Line", "Rectangle", "Selection", "Text", "Pipette" }, &mode);
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE)
		{
			if (ImGui::InputInt("Brush Width", &brushWidth))
			{
				clampBrushWidth();
			}
		}
		if (mode == MODE_RECTANGLE)
		{
			ImGui::Checkbox("Round Edges", &roundEdges);
		}
		if (mode == MODE_TEXT)
		{
			if (ImGui::InputInt("Font Size", &textFontSize))
			{
				clampTextFontSize();
			}
			ImGui::InputTextMultiline("Text", textBuffer, sizeof(textBuffer), ImVec2(0, ImGui::GetTextLineHeight() * 5.0f));
			ImGui::Text("Left/Right click places text with the active color.");
		}
		const bool supportsClipboardImages = bbe::Image::supportsClipboardImages();
		ImGui::BeginDisabled(!supportsClipboardImages);
		if (ImGui::Button("Copy to Clipboard"))
		{
			flattenVisibleLayers().copyToClipboard();
		}
		ImGui::EndDisabled();
		if (supportsClipboardImages)
		{
			ImGui::Text(bbe::Image::isImageInClipbaord() ? "Yes" : "No");
		}
		else
		{
			ImGui::Text("Image Clipboard not supported on this platform");
		}

		ImGui::BeginDisabled(!supportsClipboardImages || !bbe::Image::isImageInClipbaord());
		if (ImGui::Button("Paste"))
		{
			canvas.get().layers.clear();
			canvas.get().layers.add(PaintLayer{ "Layer 1", true, bbe::Image::getClipboardImage() });
			path = "";
			setupCanvas();
		}
		ImGui::EndDisabled();

		if (mode == MODE_SELECTION)
		{
			ImGui::BeginDisabled(!hasSelection);
			if (ImGui::Button("Copy Selection"))
			{
				storeSelectionInClipboard();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cut Selection"))
			{
				cutSelection();
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Selection"))
			{
				deleteSelection();
			}
			ImGui::EndDisabled();

			if (hasSelection)
			{
				ImGui::Text("Selection: %d x %d", selectionRect.width, selectionRect.height);
			}
			else
			{
				ImGui::Text("Selection: none");
			}
		}

		ImGui::SeparatorText("Layers");
		ImGui::BeginDisabled(canvas.get().layers.getLength() <= 1);
		if (ImGui::Button("Delete Layer"))
		{
			deleteActiveLayer();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("New Layer"))
		{
			addLayer();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled((size_t)activeLayerIndex + 1 >= canvas.get().layers.getLength());
		if (ImGui::Button("Up"))
		{
			moveActiveLayerUp();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(activeLayerIndex <= 0);
		if (ImGui::Button("Down"))
		{
			moveActiveLayerDown();
		}
		ImGui::EndDisabled();

		if (!canvas.get().layers.isEmpty())
		{
			if (ImGui::bbe::InputText("Layer Name", getActiveLayer().name))
			{
				canvas.submit();
			}
		}

		for (int32_t layerIndex = (int32_t)canvas.get().layers.getLength() - 1; layerIndex >= 0; layerIndex--)
		{
			PaintLayer &layer = canvas.get().layers[(size_t)layerIndex];
			ImGui::PushID(layerIndex);
			bool visible = layer.visible;
			if (ImGui::Checkbox("##visible", &visible))
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
		if (selectionMoveActive)
		{
			brush.drawImage(selectionRectToScreen(selectionPreviewRect), selectionPreviewImage);
			drawSelectionOutline(brush, selectionPreviewRect);
		}
		else if (selectionFloating)
		{
			brush.drawImage(selectionRectToScreen(selectionRect), selectionFloatingImage);
			drawSelectionOutline(brush, selectionRect);
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

				ImGui::SeparatorText("General");
				ImGui::BulletText("+/- changes brush size or text size for the active tool");
				ImGui::BulletText("X swaps primary and secondary color");
				ImGui::BulletText("Ctrl+D resets colors to black/white");
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

		ImGui::ShowDemoWindow();
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
