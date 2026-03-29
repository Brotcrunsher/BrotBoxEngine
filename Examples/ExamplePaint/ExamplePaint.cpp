
#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>

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

enum class LayerBlendMode : uint8_t
{
	Normal   = 0,
	Multiply = 1,
	Screen   = 2,
	Overlay  = 3,
};

struct PaintLayer
{
	bbe::String name = "";
	bool visible = true;
	float opacity = 1.0f;
	LayerBlendMode blendMode = LayerBlendMode::Normal;
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
	bool openDropChoicePopup = false;
	bbe::List<bbe::String> pendingDroppedPaths;
	bool showHelpWindow = false;
	bool showNavigator = true;
	int64_t canvasGeneration = 0;
	int64_t savedGeneration = 0;

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

	constexpr static int32_t MODE_BRUSH = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	constexpr static int32_t MODE_LINE = 2;
	constexpr static int32_t MODE_RECTANGLE = 3;
	constexpr static int32_t MODE_SELECTION = 4;
	constexpr static int32_t MODE_TEXT = 5;
	constexpr static int32_t MODE_PIPETTE = 6;
	constexpr static int32_t MODE_CIRCLE  = 7;
	constexpr static int32_t MODE_ARROW   = 8;
	constexpr static int32_t MODE_BEZIER  = 9;
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
		ROTATION,
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

	int32_t arrowHeadSize    = 15;
	int32_t arrowHeadWidth   = 12;
	bool    arrowDoubleHeaded = false;
	bool    arrowFilledHead   = true;

	bool lineDraftActive = false;
	bool lineDraftUsesRightColor = false;
	bbe::Vector2 lineDraftStart;
	bbe::Vector2 lineDraftEnd;
	int32_t lineDraftDragEndpoint = 0; // 0=none, 1=start, 2=end
	bool lineDragInProgress = false;
	bool lineDragUsesRightColor = false;

	bool arrowDraftActive = false;
	bool arrowDraftUsesRightColor = false;
	bbe::Vector2 arrowDraftStart;
	bbe::Vector2 arrowDraftEnd;
	int32_t arrowDraftDragEndpoint = 0; // 0=none, 1=start, 2=end
	bool arrowDragInProgress = false;
	bool arrowDragUsesRightColor = false;

	bbe::List<bbe::Vector2> bezierControlPoints;
	bool bezierUsesRightColor = false;
	int32_t bezierDragPointIndex = -1;

	float selectionRotation = 0.f;
	bool  rotationHandleActive = false;
	bbe::Vector2 rotationDragPivot;
	float rotationDragStartAngle = 0.f;
	float rotationDragBaseAngle = 0.f;

	bool canvasResizeActive = false;
	int32_t canvasResizeHandleIndex = -1;
	bbe::Rectanglei canvasResizePreviewRect;

	enum class SymmetryMode
	{
		None       = 0,
		Horizontal = 1,
		Vertical   = 2,
		FourWay    = 3,
		Radial     = 4,
	};
	SymmetryMode symmetryMode = SymmetryMode::None;
	int32_t radialSymmetryCount = 6;
	bool symmetryOffsetCustom = false;
	bbe::Vector2 symmetryOffset;

	bool antiAliasingEnabled = true;

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
		lineDraftActive = false;
		lineDraftDragEndpoint = 0;
		lineDragInProgress = false;
		arrowDraftActive = false;
		arrowDraftDragEndpoint = 0;
		arrowDragInProgress = false;
		bezierControlPoints.clear();
		bezierDragPointIndex = -1;
		selectionRotation = 0.f;
		rotationHandleActive = false;
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

	bbe::Colori blendPixels(const bbe::Colori &dst, const bbe::Colori &src, float opacity, LayerBlendMode blendMode) const
	{
		const float srcA = (src.a / 255.0f) * opacity;
		if (srcA <= 0.0f) return dst;

		const float srcR = src.r / 255.0f;
		const float srcG = src.g / 255.0f;
		const float srcB = src.b / 255.0f;
		const float dstR = dst.r / 255.0f;
		const float dstG = dst.g / 255.0f;
		const float dstB = dst.b / 255.0f;
		const float dstA = dst.a / 255.0f;

		float blR, blG, blB;
		switch (blendMode)
		{
		case LayerBlendMode::Multiply:
			blR = dstR * srcR;
			blG = dstG * srcG;
			blB = dstB * srcB;
			break;
		case LayerBlendMode::Screen:
			blR = 1.0f - (1.0f - dstR) * (1.0f - srcR);
			blG = 1.0f - (1.0f - dstG) * (1.0f - srcG);
			blB = 1.0f - (1.0f - dstB) * (1.0f - srcB);
			break;
		case LayerBlendMode::Overlay:
			blR = dstR < 0.5f ? 2.0f * dstR * srcR : 1.0f - 2.0f * (1.0f - dstR) * (1.0f - srcR);
			blG = dstG < 0.5f ? 2.0f * dstG * srcG : 1.0f - 2.0f * (1.0f - dstG) * (1.0f - srcG);
			blB = dstB < 0.5f ? 2.0f * dstB * srcB : 1.0f - 2.0f * (1.0f - dstB) * (1.0f - srcB);
			break;
		default: // Normal
			blR = srcR;
			blG = srcG;
			blB = srcB;
			break;
		}

		const float outA = srcA + dstA * (1.0f - srcA);
		float outR, outG, outB;
		if (outA > 0.0f)
		{
			outR = (blR * srcA + dstR * dstA * (1.0f - srcA)) / outA;
			outG = (blG * srcA + dstG * dstA * (1.0f - srcA)) / outA;
			outB = (blB * srcA + dstB * dstA * (1.0f - srcA)) / outA;
		}
		else
		{
			outR = outG = outB = 0.0f;
		}

		return bbe::Colori(
			(uint8_t)(outR * 255.0f),
			(uint8_t)(outG * 255.0f),
			(uint8_t)(outB * 255.0f),
			(uint8_t)(outA * 255.0f)
		);
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

	void blendRotatedImageOntoCanvas(const bbe::Image &image, const bbe::Rectanglei &rect, float rotation)
	{
		const float cx = rect.x + rect.width / 2.f;
		const float cy = rect.y + rect.height / 2.f;
		const float cosA = std::cos(-rotation);
		const float sinA = std::sin(-rotation);
		// Use (size-1)/2 so that canvas pixel (rect.x+k) maps to source pixel k for
		// 0° rotation: srcX = (canvasX+0.5 - cx) + srcCX = canvasX - rect.x, an integer.
		// With the simpler imgW/2 the result is non-integer for odd-width images, causing
		// a 1-px smear at half-intensity via bilinear (the "2px wide" symmetry artifact).
		const float srcCX = (image.getWidth()  - 1) / 2.f;
		const float srcCY = (image.getHeight() - 1) / 2.f;

		const float hw = image.getWidth() / 2.f;
		const float hh = image.getHeight() / 2.f;
		const float newHW = std::abs(hw * std::cos(rotation)) + std::abs(hh * std::sin(rotation));
		const float newHH = std::abs(hw * std::sin(rotation)) + std::abs(hh * std::cos(rotation));

		const int32_t x0 = (int32_t)std::floor(cx - newHW);
		const int32_t x1 = (int32_t)std::ceil(cx + newHW);
		const int32_t y0 = (int32_t)std::floor(cy - newHH);
		const int32_t y1 = (int32_t)std::ceil(cy + newHH);

		const int32_t imgW = (int32_t)image.getWidth();
		const int32_t imgH = (int32_t)image.getHeight();

		// Bilinear sample using premultiplied alpha — out-of-bounds pixels are fully
		// transparent. Premultiplied interpolation prevents color fringing and keeps
		// edge pixels at the correct text colour rather than bleeding toward zero.
		auto sampleBilinear = [&](float sx, float sy) -> bbe::Colori
		{
			const int32_t ix0 = (int32_t)std::floor(sx);
			const int32_t iy0 = (int32_t)std::floor(sy);
			const float   fx  = sx - (float)ix0;
			const float   fy  = sy - (float)iy0;
			const float   iFx = 1.f - fx;
			const float   iFy = 1.f - fy;

			auto get = [&](int32_t x, int32_t y) -> bbe::Colori
			{
				if (x < 0 || y < 0 || x >= imgW || y >= imgH) return bbe::Colori(0, 0, 0, 0);
				return image.getPixel((size_t)x, (size_t)y);
			};

			const bbe::Colori c00 = get(ix0,     iy0);
			const bbe::Colori c10 = get(ix0 + 1, iy0);
			const bbe::Colori c01 = get(ix0,     iy0 + 1);
			const bbe::Colori c11 = get(ix0 + 1, iy0 + 1);

			// Convert straight-alpha pixels to premultiplied for interpolation.
			const float a00 = c00.a, a10 = c10.a, a01 = c01.a, a11 = c11.a;
			const float pa = (a00*iFx + a10*fx)*iFy + (a01*iFx + a11*fx)*fy;
			if (pa < 0.5f) return bbe::Colori(0, 0, 0, 0);

			const float pr = (c00.r*a00*iFx + c10.r*a10*fx)*iFy + (c01.r*a01*iFx + c11.r*a11*fx)*fy;
			const float pg = (c00.g*a00*iFx + c10.g*a10*fx)*iFy + (c01.g*a01*iFx + c11.g*a11*fx)*fy;
			const float pb = (c00.b*a00*iFx + c10.b*a10*fx)*iFy + (c01.b*a01*iFx + c11.b*a11*fx)*fy;

			// Convert back to straight alpha.
			const float invA = 1.f / pa;
			return bbe::Colori(
				(bbe::byte)(pr * invA + 0.5f),
				(bbe::byte)(pg * invA + 0.5f),
				(bbe::byte)(pb * invA + 0.5f),
				(bbe::byte)(pa    + 0.5f));
		};

		for (int32_t canvasY = y0; canvasY <= y1; canvasY++)
		{
			for (int32_t canvasX = x0; canvasX <= x1; canvasX++)
			{
				const float dx = canvasX + 0.5f - cx;
				const float dy = canvasY + 0.5f - cy;
				const float srcX = dx * cosA - dy * sinA + srcCX;
				const float srcY = dx * sinA + dy * cosA + srcCY;
				const bbe::Colori srcPixel = sampleBilinear(srcX, srcY);
				if (srcPixel.a == 0) continue;

				int32_t targetX = canvasX;
				int32_t targetY = canvasY;
				if (tiled)
				{
					targetX = bbe::Math::mod<int32_t>(targetX, getCanvasWidth());
					targetY = bbe::Math::mod<int32_t>(targetY, getCanvasHeight());
				}
				else
				{
					if (targetX < 0 || targetX >= getCanvasWidth()) continue;
					if (targetY < 0 || targetY >= getCanvasHeight()) continue;
				}
				const bbe::Colori oldColor = getActiveLayerImage().getPixel((size_t)targetX, (size_t)targetY);
				getActiveLayerImage().setPixel((size_t)targetX, (size_t)targetY, oldColor.blendTo(srcPixel));
			}
		}
	}

	// Returns a nearest-neighbor rasterized rotation of src, sized to fit the rotated bounding box.
	bbe::Image createRotatedPreviewImage(const bbe::Image &src, float rotation) const
	{
		const float cosA = std::cos(-rotation);
		const float sinA = std::sin(-rotation);
		const float srcCX = src.getWidth() / 2.f;
		const float srcCY = src.getHeight() / 2.f;

		const float hw = src.getWidth() / 2.f;
		const float hh = src.getHeight() / 2.f;
		const float newHW = std::abs(hw * std::cos(rotation)) + std::abs(hh * std::sin(rotation));
		const float newHH = std::abs(hw * std::sin(rotation)) + std::abs(hh * std::cos(rotation));

		const int32_t newW = std::max(1, (int32_t)std::ceil(newHW * 2.f));
		const int32_t newH = std::max(1, (int32_t)std::ceil(newHH * 2.f));

		bbe::Image result(newW, newH, bbe::Color(0.f, 0.f, 0.f, 0.f));
		prepareImageForCanvas(result);

		for (int32_t py = 0; py < newH; py++)
		{
			for (int32_t px = 0; px < newW; px++)
			{
				const float dx = px + 0.5f - newHW;
				const float dy = py + 0.5f - newHH;
				const float srcX = dx * cosA - dy * sinA + srcCX;
				const float srcY = dx * sinA + dy * cosA + srcCY;
				if (srcX < 0.f || srcX >= (float)src.getWidth() || srcY < 0.f || srcY >= (float)src.getHeight()) continue;
				result.setPixel((size_t)px, (size_t)py, src.getPixel((size_t)srcX, (size_t)srcY));
			}
		}
		return result;
	}

	bbe::Image flattenVisibleLayers() const
	{
		bbe::Image flattened(getCanvasWidth(), getCanvasHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		prepareImageForCanvas(flattened);
		for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
		{
			const PaintLayer &layer = canvas.get().layers[i];
			if (!layer.visible) continue;
			for (int32_t ly = 0; ly < layer.image.getHeight(); ly++)
			{
				for (int32_t lx = 0; lx < layer.image.getWidth(); lx++)
				{
					const bbe::Colori src = layer.image.getPixel((size_t)lx, (size_t)ly);
					const bbe::Colori dst = flattened.getPixel((size_t)lx, (size_t)ly);
					flattened.setPixel((size_t)lx, (size_t)ly, blendPixels(dst, src, layer.opacity, layer.blendMode));
				}
			}
		}
		return flattened;
	}

	bbe::Colori getVisiblePixel(size_t x, size_t y) const
	{
		bbe::Colori color(0, 0, 0, 0);
		for (size_t i = 0; i < canvas.get().layers.getLength(); i++)
		{
			const PaintLayer &layer = canvas.get().layers[i];
			if (!layer.visible) continue;
			const bbe::Colori src = layer.image.getPixel(x, y);
			color = blendPixels(color, src, layer.opacity, layer.blendMode);
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
		submitCanvas();
	}

	void importFileAsLayers(const bbe::List<bbe::String> &paths)
	{
		for (size_t i = 0; i < paths.getLength(); i++)
		{
			const bbe::String &path = paths[i];
			if (!isSupportedDroppedDocumentPath(path)) continue;

			if (isLayeredDocumentPath(path))
			{
				// Import every layer from the .bbepaint file
				bbe::ByteBuffer buffer = bbe::simpleFile::readBinaryFile(path);
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
						layer.blendMode = (LayerBlendMode)blendModeRaw;
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
				bbe::Image img(path.getRaw());
				if (img.getWidth() <= 0 || img.getHeight() <= 0) continue;
				prepareImageForCanvas(img);
				bbe::String name = std::filesystem::path(path.getRaw()).stem().string().c_str();
				prepareForLayerTargetChange();
				canvas.get().layers.add(PaintLayer{ name, true, 1.0f, LayerBlendMode::Normal, std::move(img) });
				activeLayerIndex = (int32_t)canvas.get().layers.getLength() - 1;
			}
		}
		submitCanvas();
	}

	void deleteActiveLayer()
	{
		if (canvas.get().layers.getLength() <= 1) return;
		prepareForLayerTargetChange();
		canvas.get().layers.removeIndex((size_t)activeLayerIndex);
		clampActiveLayerIndex();
		submitCanvas();
	}

	void moveActiveLayerUp()
	{
		if ((size_t)activeLayerIndex + 1 >= canvas.get().layers.getLength()) return;
		prepareForLayerTargetChange();
		canvas.get().layers.swap((size_t)activeLayerIndex, (size_t)activeLayerIndex + 1);
		activeLayerIndex++;
		submitCanvas();
	}

	void moveActiveLayerDown()
	{
		if (activeLayerIndex <= 0) return;
		prepareForLayerTargetChange();
		canvas.get().layers.swap((size_t)activeLayerIndex, (size_t)activeLayerIndex - 1);
		activeLayerIndex--;
		submitCanvas();
	}

	void duplicateActiveLayer()
	{
		prepareForLayerTargetChange();
		PaintLayer dup = getActiveLayer();
		dup.name = dup.name + " Copy";
		canvas.get().layers.addAt((size_t)activeLayerIndex + 1, dup);
		activeLayerIndex++;
		submitCanvas();
	}

	void mergeActiveLayerDown()
	{
		if (activeLayerIndex <= 0) return;
		prepareForLayerTargetChange();
		PaintLayer &above = canvas.get().layers[(size_t)activeLayerIndex];
		PaintLayer &below = canvas.get().layers[(size_t)(activeLayerIndex - 1)];
		for (int32_t y = 0; y < above.image.getHeight(); y++)
		{
			for (int32_t x = 0; x < above.image.getWidth(); x++)
			{
				const bbe::Colori src = above.image.getPixel((size_t)x, (size_t)y);
				const bbe::Colori dst = below.image.getPixel((size_t)x, (size_t)y);
				below.image.setPixel((size_t)x, (size_t)y, blendPixels(dst, src, above.opacity, above.blendMode));
			}
		}
		canvas.get().layers.removeIndex((size_t)activeLayerIndex);
		activeLayerIndex--;
		submitCanvas();
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
		return hitZone != SelectionHitZone::NONE
			&& hitZone != SelectionHitZone::INSIDE
			&& hitZone != SelectionHitZone::ROTATION;
	}

	bbe::Vector2 getRotationHandleCanvasPos(const bbe::Rectanglei &rect) const
	{
		const float stemScreenLen = 30.f;
		return {
			rect.x + rect.width / 2.f,
			rect.y - stemScreenLen / zoomLevel
		};
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

		if (hasSelection && !selectionDragActive)
		{
			const bbe::Vector2 handlePos = getRotationHandleCanvasPos(selectionRect);
			const float hitRadius = 8.f / zoomLevel;
			const float dx = point.x - handlePos.x;
			const float dy = point.y - handlePos.y;
			if (dx * dx + dy * dy <= hitRadius * hitRadius) return SelectionHitZone::ROTATION;
		}

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
		submitCanvas();
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

		if (std::abs(selectionRotation) > 0.0001f)
		{
			blendRotatedImageOntoCanvas(selectionFloatingImage, selectionRect, selectionRotation);
			if (rectangleDraftActive || circleDraftActive)
			{
				const bbe::Vector2 center = {
					selectionRect.x + selectionRect.width  * 0.5f,
					selectionRect.y + selectionRect.height * 0.5f
				};
				const auto symCenters = getSymmetryPositions(center);
				const auto symAngles  = getSymmetryRotationAngles();
				for (size_t i = 1; i < symCenters.getLength(); i++)
				{
					const bbe::Rectanglei symRect = {
						(int32_t)std::round(symCenters[i].x - selectionRect.width  * 0.5f),
						(int32_t)std::round(symCenters[i].y - selectionRect.height * 0.5f),
						selectionRect.width,
						selectionRect.height
					};
					blendRotatedImageOntoCanvas(selectionFloatingImage, symRect, selectionRotation + symAngles[i]);
				}
			}
			submitCanvas();
			clearSelectionState();
			return;
		}

		blendImageOntoCanvas(selectionFloatingImage, selectionRect.getPos());
		if (rectangleDraftActive || circleDraftActive)
		{
			const bbe::Vector2 center = {
				selectionRect.x + selectionRect.width  * 0.5f,
				selectionRect.y + selectionRect.height * 0.5f
			};
			const auto symCenters = getSymmetryPositions(center);
			const auto symAngles  = getSymmetryRotationAngles();
			for (size_t i = 1; i < symCenters.getLength(); i++)
			{
				const bbe::Rectanglei symRect = {
					(int32_t)std::round(symCenters[i].x - selectionRect.width  * 0.5f),
					(int32_t)std::round(symCenters[i].y - selectionRect.height * 0.5f),
					selectionRect.width,
					selectionRect.height
				};
				if (std::abs(symAngles[i]) > 0.0001f)
					blendRotatedImageOntoCanvas(selectionFloatingImage, symRect, symAngles[i]);
				else
					blendImageOntoCanvas(selectionFloatingImage, symRect.getPos());
			}
		}
		submitCanvas();

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
		// With AA off, snap to the containing pixel's centre so that distances
		// are always whole numbers — identical to the original integer-offset
		// formula, giving clean single-pixel results with no boundary bleed.
		const float effX = antiAliasingEnabled ? touchPos.x : std::floor(touchPos.x) + 0.5f;
		const float effY = antiAliasingEnabled ? touchPos.y : std::floor(touchPos.y) + 0.5f;
		for (int32_t i = -toolBrushWidth; i <= toolBrushWidth; i++)
		{
			for (int32_t k = -toolBrushWidth; k <= toolBrushWidth; k++)
			{
				// Pixel centers live at (floor(eff + offset) + 0.5) in canvas space,
				// so a click at the visual centre of any pixel has distance 0 from that pixel.
				const float logicalX = std::floor(effX + (float)i) + 0.5f;
				const float logicalY = std::floor(effY + (float)k) + 0.5f;
				const float dx = logicalX - effX;
				const float dy = logicalY - effY;
				float pencilStrength;
				if (rectangleShape)
				{
					const float maxD = std::max(std::fabsf(dx), std::fabsf(dy));
					pencilStrength = bbe::Math::clamp01((float)toolBrushWidth - maxD);
				}
				else
				{
					pencilStrength = bbe::Math::clamp01((float)toolBrushWidth - bbe::Math::sqrt(dx * dx + dy * dy));
				}
				if (pencilStrength <= 0.f) continue;

				bbe::Vector2 coord = touchPos + bbe::Vector2(i, k);
				if (toImagePos(coord, image.getWidth(), image.getHeight(), repeated))
				{
					bbe::Colori newColor = color;
					newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
					const int32_t pixelX = std::min((int32_t)std::floor(coord.x), (int32_t)image.getWidth()  - 1);
					const int32_t pixelY = std::min((int32_t)std::floor(coord.y), (int32_t)image.getHeight() - 1);
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
		// For tiled/wrapped mode, square brushes, or AA-off fall back to the stamp-based
		// path. The stamp path uses touchImage which handles AA-off via pixel-centre
		// snapping, giving clean single-pixel results with no capsule-SDF boundary bleed.
		if (repeated || rectangleShape || !antiAliasingEnabled)
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

		// Capsule SDF rasterizer: for each pixel in the bounding box, compute the
		// exact distance from the pixel center to the segment and apply SDF-based AA —
		// the same technique used by the rotated-rectangle and circle tools.
		const float ax = pos2.x - pos1.x;
		const float ay = pos2.y - pos1.y;
		const float lenSq = ax * ax + ay * ay;

		const float margin = (float)toolBrushWidth + 1.f;
		const int32_t xMin = (int32_t)std::floor(std::min(pos1.x, pos2.x) - margin);
		const int32_t xMax = (int32_t)std::ceil (std::max(pos1.x, pos2.x) + margin);
		const int32_t yMin = (int32_t)std::floor(std::min(pos1.y, pos2.y) - margin);
		const int32_t yMax = (int32_t)std::ceil (std::max(pos1.y, pos2.y) + margin);

		bool changeRegistered = false;
		for (int32_t y = yMin; y <= yMax; y++)
		{
			for (int32_t x = xMin; x <= xMax; x++)
			{
				// Use pixel centres (+0.5) so the distance is 0 when the segment
				// passes through the middle of a pixel, consistent with touchImage.
				const float pcx = (float)x + 0.5f;
				const float pcy = (float)y + 0.5f;
				float dist;
				if (lenSq < 1e-6f)
				{
					const float dx = pcx - pos1.x;
					const float dy = pcy - pos1.y;
					dist = bbe::Math::sqrt(dx * dx + dy * dy);
				}
				else
				{
					const float bx = pcx - pos1.x;
					const float by = pcy - pos1.y;
					const float t = bbe::Math::clamp01((bx * ax + by * ay) / lenSq);
					const float cx = pcx - (pos1.x + t * ax);
					const float cy = pcy - (pos1.y + t * ay);
					dist = bbe::Math::sqrt(cx * cx + cy * cy);
				}

				const float pencilStrength = bbe::Math::clamp01((float)toolBrushWidth - dist);
				if (pencilStrength <= 0.f) continue;

				bbe::Vector2 coord = { (float)x, (float)y };
				if (!toImagePos(coord, image.getWidth(), image.getHeight(), repeated)) continue;

				const int32_t pixelX = std::min((int32_t)std::round(coord.x), (int32_t)image.getWidth()  - 1);
				const int32_t pixelY = std::min((int32_t)std::round(coord.y), (int32_t)image.getHeight() - 1);
				bbe::Colori newColor = color;
				newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
				const bbe::Colori oldColor = image.getPixel((size_t)pixelX, (size_t)pixelY);
				if (newColor.a > oldColor.a)
				{
					image.setPixel((size_t)pixelX, (size_t)pixelY, newColor);
					changeRegistered = true;
				}
			}
		}
		return changeRegistered;
	}

	void fillTriangleOnImage(bbe::Image &image, const bbe::Vector2 &v0, const bbe::Vector2 &v1, const bbe::Vector2 &v2, const bbe::Colori &color) const
	{
		auto edgeDist = [](float ax, float ay, float bx, float by, float px, float py) -> float
		{
			const float ex = bx - ax, ey = by - ay;
			const float len = bbe::Math::sqrt(ex * ex + ey * ey);
			if (len < 1e-6f) return 0.f;
			return (ex * (py - ay) - ey * (px - ax)) / len;
		};

		// Ensure CCW winding
		const float area2 = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
		const bbe::Vector2 a = v0;
		const bbe::Vector2 b = area2 >= 0.f ? v1 : v2;
		const bbe::Vector2 c = area2 >= 0.f ? v2 : v1;

		const int32_t x0 = (int32_t)bbe::Math::floor(bbe::Math::min(bbe::Math::min(a.x, b.x), c.x) - 1.f);
		const int32_t x1 = (int32_t)bbe::Math::ceil (bbe::Math::max(bbe::Math::max(a.x, b.x), c.x) + 1.f);
		const int32_t y0 = (int32_t)bbe::Math::floor(bbe::Math::min(bbe::Math::min(a.y, b.y), c.y) - 1.f);
		const int32_t y1 = (int32_t)bbe::Math::ceil (bbe::Math::max(bbe::Math::max(a.y, b.y), c.y) + 1.f);

		for (int32_t y = y0; y <= y1; y++)
		{
			for (int32_t x = x0; x <= x1; x++)
			{
				const float px = x + 0.5f, py = y + 0.5f;
				const float d0 = edgeDist(a.x, a.y, b.x, b.y, px, py);
				const float d1 = edgeDist(b.x, b.y, c.x, c.y, px, py);
				const float d2 = edgeDist(c.x, c.y, a.x, a.y, px, py);
				const float minD = d0 < d1 ? (d0 < d2 ? d0 : d2) : (d1 < d2 ? d1 : d2);
				const float alpha = bbe::Math::clamp01(minD + 0.5f);
				if (alpha <= 0.f) continue;
				const float finalAlpha = antiAliasingEnabled ? alpha : (alpha > 0.5f ? 1.0f : 0.0f);
				if (finalAlpha <= 0.f) continue;

				int32_t tx = x, ty = y;
				if (tiled)
				{
					tx = bbe::Math::mod<int32_t>(tx, image.getWidth());
					ty = bbe::Math::mod<int32_t>(ty, image.getHeight());
				}
				else
				{
					if (tx < 0 || ty < 0 || tx >= image.getWidth() || ty >= image.getHeight()) continue;
				}

				bbe::Colori pix = color;
				pix.a = (bbe::byte)(pix.a * finalAlpha);
				const bbe::Colori old = image.getPixel((size_t)tx, (size_t)ty);
				if (pix.a > old.a)
				{
					image.setPixel((size_t)tx, (size_t)ty, pix);
				}
			}
		}
	}

	void drawArrowToWorkArea(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color)
	{
		const float dx = to.x - from.x;
		const float dy = to.y - from.y;
		const float len = bbe::Math::sqrt(dx * dx + dy * dy);
		if (len < 1.f) return;

		const float nx = dx / len;
		const float ny = dy / len;
		const float px = -ny;
		const float py =  nx;

		const float headLen   = (float)arrowHeadSize;
		const float halfWidth = (float)arrowHeadWidth * 0.5f;

		bbe::Vector2 shaftFrom = from;
		bbe::Vector2 shaftTo   = to;

		if (arrowFilledHead)
		{
			shaftTo.x = to.x - nx * headLen;
			shaftTo.y = to.y - ny * headLen;
			if (arrowDoubleHeaded)
			{
				shaftFrom.x = from.x + nx * headLen;
				shaftFrom.y = from.y + ny * headLen;
			}
		}

		touchLineImage(workArea, shaftFrom, shaftTo, color, brushWidth, false, tiled);

		// Forward head
		{
			const bbe::Vector2 tip  = to;
			const bbe::Vector2 base(to.x - nx * headLen, to.y - ny * headLen);
			const bbe::Vector2 left (base.x + px * halfWidth, base.y + py * halfWidth);
			const bbe::Vector2 right(base.x - px * halfWidth, base.y - py * halfWidth);
			if (arrowFilledHead)
			{
				fillTriangleOnImage(workArea, tip, left, right, color);
			}
			else
			{
				touchLineImage(workArea, tip, left, color, brushWidth, false, tiled);
				touchLineImage(workArea, tip, right, color, brushWidth, false, tiled);
			}
		}

		// Backward head
		if (arrowDoubleHeaded)
		{
			const bbe::Vector2 tip  = from;
			const bbe::Vector2 base(from.x + nx * headLen, from.y + ny * headLen);
			const bbe::Vector2 left (base.x + px * halfWidth, base.y + py * halfWidth);
			const bbe::Vector2 right(base.x - px * halfWidth, base.y - py * halfWidth);
			if (arrowFilledHead)
			{
				fillTriangleOnImage(workArea, tip, left, right, color);
			}
			else
			{
				touchLineImage(workArea, tip, left, color, brushWidth, false, tiled);
				touchLineImage(workArea, tip, right, color, brushWidth, false, tiled);
			}
		}
	}

	bbe::Colori getLineDraftColor() const
	{
		return lineDraftUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	bbe::Colori getArrowDraftColor() const
	{
		return arrowDraftUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	void finalizeLineDraft()
	{
		applyWorkArea();
		submitCanvas();
		lineDraftActive = false;
		lineDraftDragEndpoint = 0;
	}

	void finalizeArrowDraft()
	{
		applyWorkArea();
		submitCanvas();
		arrowDraftActive = false;
		arrowDraftDragEndpoint = 0;
	}

	void updateLineTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2 mouseCanvas = screenToCanvas(getMouse());

		if (lineDraftActive)
		{
			bool handledMousePress = false;

			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				const float handleRadius = 6.f / zoomLevel;
				const float distToStart = (mouseCanvas - lineDraftStart).getLength();
				const float distToEnd   = (mouseCanvas - lineDraftEnd).getLength();

				if (distToStart <= handleRadius && distToStart <= distToEnd)
				{
					lineDraftDragEndpoint = 1;
					handledMousePress = true;
				}
				else if (distToEnd <= handleRadius)
				{
					lineDraftDragEndpoint = 2;
					handledMousePress = true;
				}
				else
				{
					finalizeLineDraft();
					return;
				}
			}

			if (!handledMousePress && isMousePressed(bbe::MouseButton::RIGHT))
			{
				finalizeLineDraft();
				return;
			}

			if (lineDraftDragEndpoint != 0 && isMouseDown(bbe::MouseButton::LEFT))
			{
				if (lineDraftDragEndpoint == 1) lineDraftStart = mouseCanvas;
				else                            lineDraftEnd   = mouseCanvas;
			}
			if (lineDraftDragEndpoint != 0 && isMouseReleased(bbe::MouseButton::LEFT))
			{
				lineDraftDragEndpoint = 0;
			}

			clearWorkArea();
			touchLineSymmetry(lineDraftEnd, lineDraftStart, getLineDraftColor(), brushWidth);
			return;
		}

		if (!lineDragInProgress)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				lineDragInProgress = true;
				lineDragUsesRightColor = false;
				lineDraftStart = mouseCanvas;
			}
			else if (isMousePressed(bbe::MouseButton::RIGHT))
			{
				lineDragInProgress = true;
				lineDragUsesRightColor = true;
				lineDraftStart = mouseCanvas;
			}
		}

		if (lineDragInProgress)
		{
			clearWorkArea();
			const bbe::Colori dragColor = lineDragUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
			touchLineSymmetry(mouseCanvas, lineDraftStart, dragColor, brushWidth);

			const bool released = lineDragUsesRightColor
				? isMouseReleased(bbe::MouseButton::RIGHT)
				: isMouseReleased(bbe::MouseButton::LEFT);
			if (released)
			{
				lineDraftActive = true;
				lineDraftUsesRightColor = lineDragUsesRightColor;
				lineDraftEnd = mouseCanvas;
				lineDragInProgress = false;
			}
		}
	}

	void updateArrowTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2 mouseCanvas = screenToCanvas(getMouse());

		if (arrowDraftActive)
		{
			bool handledMousePress = false;

			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				const float handleRadius = 6.f / zoomLevel;
				const float distToStart = (mouseCanvas - arrowDraftStart).getLength();
				const float distToEnd   = (mouseCanvas - arrowDraftEnd).getLength();

				if (distToStart <= handleRadius && distToStart <= distToEnd)
				{
					arrowDraftDragEndpoint = 1;
					handledMousePress = true;
				}
				else if (distToEnd <= handleRadius)
				{
					arrowDraftDragEndpoint = 2;
					handledMousePress = true;
				}
				else
				{
					finalizeArrowDraft();
					return;
				}
			}

			if (!handledMousePress && isMousePressed(bbe::MouseButton::RIGHT))
			{
				finalizeArrowDraft();
				return;
			}

			if (arrowDraftDragEndpoint != 0 && isMouseDown(bbe::MouseButton::LEFT))
			{
				if (arrowDraftDragEndpoint == 1) arrowDraftStart = mouseCanvas;
				else                             arrowDraftEnd   = mouseCanvas;
			}
			if (arrowDraftDragEndpoint != 0 && isMouseReleased(bbe::MouseButton::LEFT))
			{
				arrowDraftDragEndpoint = 0;
			}

			clearWorkArea();
			drawArrowSymmetry(arrowDraftStart, arrowDraftEnd, getArrowDraftColor());
			return;
		}

		if (!arrowDragInProgress)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				arrowDragInProgress = true;
				arrowDragUsesRightColor = false;
				arrowDraftStart = mouseCanvas;
			}
			else if (isMousePressed(bbe::MouseButton::RIGHT))
			{
				arrowDragInProgress = true;
				arrowDragUsesRightColor = true;
				arrowDraftStart = mouseCanvas;
			}
		}

		if (arrowDragInProgress)
		{
			clearWorkArea();
			const bbe::Colori dragColor = arrowDragUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
			drawArrowSymmetry(arrowDraftStart, mouseCanvas, dragColor);

			const bool released = arrowDragUsesRightColor
				? isMouseReleased(bbe::MouseButton::RIGHT)
				: isMouseReleased(bbe::MouseButton::LEFT);
			if (released)
			{
				arrowDraftActive = true;
				arrowDraftUsesRightColor = arrowDragUsesRightColor;
				arrowDraftEnd = mouseCanvas;
				arrowDragInProgress = false;
			}
		}
	}

	bbe::Colori getBezierColor() const
	{
		return bezierUsesRightColor ? bbe::Color(rightColor).asByteColor() : bbe::Color(leftColor).asByteColor();
	}

	bbe::Vector2 evaluateBezier(const bbe::List<bbe::Vector2> &points, float t) const
	{
		// De Casteljau's algorithm with a stack buffer (supports up to 64 control points)
		static bbe::Vector2 work[64];
		const size_t n = points.getLength();
		for (size_t i = 0; i < n; i++) work[i] = points[i];
		for (size_t r = 1; r < n; r++)
		{
			for (size_t i = 0; i < n - r; i++)
			{
				work[i] = bbe::Vector2(
					work[i].x * (1.f - t) + work[i + 1].x * t,
					work[i].y * (1.f - t) + work[i + 1].y * t);
			}
		}
		return work[0];
	}

	void drawBezierToWorkArea(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color)
	{
		if (points.getLength() < 2) return;
		const int32_t numSamples = bbe::Math::max(200, (int32_t)points.getLength() * 100);
		bbe::Vector2 prev = evaluateBezier(points, 0.f);
		for (int32_t i = 1; i <= numSamples; i++)
		{
			const float t = (float)i / (float)numSamples;
			const bbe::Vector2 curr = evaluateBezier(points, t);
			touchLineImage(workArea, prev, curr, color, brushWidth, false, tiled);
			prev = curr;
		}
	}

	void finalizeBezierDraft()
	{
		if (bezierControlPoints.getLength() >= 2)
		{
			clearWorkArea();
			drawBezierSymmetry(bezierControlPoints, getBezierColor());
			applyWorkArea();
			submitCanvas();
		}
		else
		{
			clearWorkArea();
		}
		bezierControlPoints.clear();
		bezierDragPointIndex = -1;
	}

	void updateBezierTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2 mouseCanvas = screenToCanvas(getMouse());
		const float handleRadius = 6.f / zoomLevel;

		// Update dragged control point
		if (bezierDragPointIndex >= 0)
		{
			if (isMouseDown(bbe::MouseButton::LEFT))
			{
				bezierControlPoints[(size_t)bezierDragPointIndex] = mouseCanvas;
			}
			if (isMouseReleased(bbe::MouseButton::LEFT))
			{
				bezierDragPointIndex = -1;
			}
		}

		if (bezierDragPointIndex < 0)
		{
			if (isMousePressed(bbe::MouseButton::LEFT))
			{
				// Check if clicking near an existing control point to drag it
				int32_t hitIndex = -1;
				float bestDist = handleRadius;
				for (size_t i = 0; i < bezierControlPoints.getLength(); i++)
				{
					const float dist = (mouseCanvas - bezierControlPoints[i]).getLength();
					if (dist < bestDist)
					{
						bestDist = dist;
						hitIndex = (int32_t)i;
					}
				}

				if (hitIndex >= 0)
				{
					bezierDragPointIndex = hitIndex;
				}
				else
				{
					// Add new control point
					if (bezierControlPoints.isEmpty()) bezierUsesRightColor = false;
					bezierControlPoints.add(mouseCanvas);
				}
			}

			if (isMousePressed(bbe::MouseButton::RIGHT))
			{
				finalizeBezierDraft();
				return;
			}

			// Backspace removes the last placed control point
			if ((isKeyPressed(bbe::Key::BACKSPACE)) && !bezierControlPoints.isEmpty())
			{
				bezierControlPoints.popBack();
			}
		}

		// Rebuild workArea preview each frame
		clearWorkArea();
		if (bezierControlPoints.getLength() >= 2)
		{
			// Show a ghost preview with the cursor as a potential next point
			bool nearExisting = false;
			for (size_t i = 0; i < bezierControlPoints.getLength(); i++)
			{
				if ((mouseCanvas - bezierControlPoints[i]).getLength() < handleRadius)
				{
					nearExisting = true;
					break;
				}
			}

			bbe::List<bbe::Vector2> previewPoints = bezierControlPoints;
			if (!nearExisting && bezierDragPointIndex < 0)
			{
				previewPoints.add(mouseCanvas);
			}
			drawBezierSymmetry(previewPoints, getBezierColor());
		}
		else if (bezierControlPoints.getLength() == 1)
		{
			// Before we have 2 points, just draw a line preview to cursor
			touchLineSymmetry(bezierControlPoints[0], mouseCanvas, getBezierColor(), brushWidth);
		}
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

		// SDF of a rounded rectangle centered at origin.
		// hw/hh = outer half-extents, r = corner radius.
		auto sdfRoundRect = [](float px, float py, float hw, float hh, float r) -> float
		{
			const float ax  = px < 0.f ? -px : px;
			const float ay  = py < 0.f ? -py : py;
			const float qx  = ax - hw + r;
			const float qy  = ay - hh + r;
			const float qxp = qx > 0.f ? qx : 0.f;
			const float qyp = qy > 0.f ? qy : 0.f;
			const float mx  = qx > qy ? qx : qy;
			return bbe::Math::sqrt(qxp * qxp + qyp * qyp) + (mx < 0.f ? mx : 0.f) - r;
		};

		const float R  = (float)bbe::Math::min(cornerRadius, bbe::Math::min(width / 2, height / 2));
		const float T  = (float)brushWidth;
		const float hw = width  * 0.5f;
		const float hh = height * 0.5f;
		const float hwi = hw - T;
		const float hhi = hh - T;
		const float Ri  = R - T > 0.f ? R - T : 0.f;

		for (int32_t y = 0; y < height; y++)
		{
			for (int32_t x = 0; x < width; x++)
			{
				// Pixel center in rectangle-centered coordinates
				const float px = (x + 0.5f) - hw;
				const float py = (y + 0.5f) - hh;

				const float dOuter     = sdfRoundRect(px, py, hw, hh, R);
				const float alphaOuter = bbe::Math::clamp01(-dOuter + 0.5f);
				if (alphaOuter <= 0.f) continue;

				float alphaInner = 1.f;
				if (hwi > 0.f && hhi > 0.f)
				{
					const float dInner = sdfRoundRect(px, py, hwi, hhi, Ri);
					alphaInner = bbe::Math::clamp01(dInner + 0.5f);
				}

				const float alpha = alphaOuter < alphaInner ? alphaOuter : alphaInner;
				if (alpha <= 0.f) continue;
				const float finalAlpha = antiAliasingEnabled ? alpha : (alpha > 0.5f ? 1.0f : 0.0f);
				if (finalAlpha <= 0.f) continue;

				bbe::Colori c = color;
				c.a = (bbe::byte)(c.a * finalAlpha);
				image.setPixel((size_t)x, (size_t)y, c);
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
				const float finalAlpha = antiAliasingEnabled ? alpha : (alpha > 0.5f ? 1.0f : 0.0f);
				if (finalAlpha <= 0.f) continue;

				bbe::Colori c = color;
				c.a = (bbe::byte)(c.a * finalAlpha);
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
				if (hitZone == SelectionHitZone::ROTATION)
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

		if (rotationHandleActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateRotationDrag(mousePixel);
		}
		if (selectionMoveActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionMovePreview(mousePixel);
		}
		if (selectionResizeActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionResizePreview(mousePixel);
		}
		if (rotationHandleActive && isMouseReleased(bbe::MouseButton::LEFT))
		{
			rotationHandleActive = false;
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

	void beginRotationDrag(const bbe::Vector2i &mousePixel)
	{
		if (!selectionFloating && hasSelection)
		{
			selectionFloatingImage = copyCanvasRect(selectionRect);
			prepareImageForCanvas(selectionFloatingImage);
			clearCanvasRect(selectionRect);
			selectionFloating = true;
			submitCanvas();
		}

		rotationHandleActive = true;
		rotationDragPivot = {
			selectionRect.x + selectionRect.width / 2.f,
			selectionRect.y + selectionRect.height / 2.f
		};
		const bbe::Vector2 toMouse((float)mousePixel.x - rotationDragPivot.x, (float)mousePixel.y - rotationDragPivot.y);
		rotationDragStartAngle = toMouse.getLength() > 0.001f ? toMouse.getAngle() : 0.f;
		rotationDragBaseAngle = selectionRotation;
	}

	void updateRotationDrag(const bbe::Vector2i &mousePixel)
	{
		const bbe::Vector2 toMouse((float)mousePixel.x - rotationDragPivot.x, (float)mousePixel.y - rotationDragPivot.y);
		if (toMouse.getLength() > 0.001f)
		{
			selectionRotation = rotationDragBaseAngle + (toMouse.getAngle() - rotationDragStartAngle);
		}
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
			if (hitZone == SelectionHitZone::ROTATION && hasSelection)
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
			if (rotationHandleActive)
			{
				updateRotationDrag(mousePixel);
			}
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
			rotationHandleActive = false;

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
				if (hitZone == SelectionHitZone::ROTATION)
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

		if (rotationHandleActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateRotationDrag(mousePixel);
		}
		if (selectionMoveActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionMovePreview(mousePixel);
		}
		if (selectionResizeActive && isMouseDown(bbe::MouseButton::LEFT))
		{
			updateSelectionResizePreview(mousePixel);
		}
		if (rotationHandleActive && isMouseReleased(bbe::MouseButton::LEFT))
		{
			rotationHandleActive = false;
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

	// Returns the screen-space position of canvas resize handle i (0=TL,1=T,2=TR,3=R,4=BR,5=B,6=BL,7=L).
	bbe::Vector2 getCanvasHandleScreenPos(int32_t i) const
	{
		const float W = getCanvasWidth() * zoomLevel;
		const float H = getCanvasHeight() * zoomLevel;
		switch (i)
		{
			case 0: return { offset.x,         offset.y         };
			case 1: return { offset.x + W/2.f,  offset.y         };
			case 2: return { offset.x + W,      offset.y         };
			case 3: return { offset.x + W,      offset.y + H/2.f };
			case 4: return { offset.x + W,      offset.y + H     };
			case 5: return { offset.x + W/2.f,  offset.y + H     };
			case 6: return { offset.x,           offset.y + H     };
			case 7: return { offset.x,           offset.y + H/2.f };
			default: return offset;
		}
	}

	// Returns the index (0-7) of the canvas resize handle the screen-space point is near, or -1.
	int32_t getCanvasResizeHitHandle(const bbe::Vector2 &screenPos) const
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

	void updateCanvasResizePreview(const bbe::Vector2 &canvasMousePos)
	{
		const int32_t W = getCanvasWidth();
		const int32_t H = getCanvasHeight();
		const int32_t mx = (int32_t)std::round(canvasMousePos.x);
		const int32_t my = (int32_t)std::round(canvasMousePos.y);
		switch (canvasResizeHandleIndex)
		{
			case 0: canvasResizePreviewRect = { mx, my, std::max(1, W - mx), std::max(1, H - my) }; break;
			case 1: canvasResizePreviewRect = { 0,  my, W,                   std::max(1, H - my) }; break;
			case 2: canvasResizePreviewRect = { 0,  my, std::max(1, mx),     std::max(1, H - my) }; break;
			case 3: canvasResizePreviewRect = { 0,  0,  std::max(1, mx),     H                   }; break;
			case 4: canvasResizePreviewRect = { 0,  0,  std::max(1, mx),     std::max(1, my)     }; break;
			case 5: canvasResizePreviewRect = { 0,  0,  W,                   std::max(1, my)     }; break;
			case 6: canvasResizePreviewRect = { mx, 0,  std::max(1, W - mx), std::max(1, my)     }; break;
			case 7: canvasResizePreviewRect = { mx, 0,  std::max(1, W - mx), H                   }; break;
			default: break;
		}
	}

	void applyCanvasResize(const bbe::Rectanglei &previewRect)
	{
		if (previewRect.width <= 0 || previewRect.height <= 0) return;
		if (canvas.get().layers.isEmpty()) return;

		const bbe::Color fillColor(rightColor[0], rightColor[1], rightColor[2], rightColor[3]);
		const int32_t oldW = getCanvasWidth();
		const int32_t oldH = getCanvasHeight();

		for (size_t li = 0; li < canvas.get().layers.getLength(); li++)
		{
			bbe::Image newImage(previewRect.width, previewRect.height, fillColor);
			prepareImageForCanvas(newImage);
			for (int32_t ny = 0; ny < previewRect.height; ny++)
			{
				for (int32_t nx = 0; nx < previewRect.width; nx++)
				{
					const int32_t ox = nx + previewRect.x;
					const int32_t oy = ny + previewRect.y;
					if (ox < 0 || ox >= oldW || oy < 0 || oy >= oldH) continue;
					newImage.setPixel((size_t)nx, (size_t)ny,
						canvas.get().layers[li].image.getPixel((size_t)ox, (size_t)oy));
				}
			}
			canvas.get().layers[li].image = std::move(newImage);
		}

		// Shift offset so visual position of original content is preserved.
		offset.x += previewRect.x * zoomLevel;
		offset.y += previewRect.y * zoomLevel;

		clearSelectionState();
		clearWorkArea();
		submitCanvas();
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

		if (hasSelection && !selectionDragActive)
		{
			const float cx = screenRect.x + screenRect.width / 2.f;
			const float ty = screenRect.y;
			constexpr float stemLen = 30.f;
			const float handleY = ty - stemLen;
			constexpr float handleR = 6.f;

			// Stem line
			brush.setColorRGB(0.f, 0.f, 0.f);
			brush.fillLine(cx + 1.f, ty, cx + 1.f, handleY, 1.f);
			brush.setColorRGB(1.f, 1.f, 1.f);
			brush.fillLine(cx, ty, cx, handleY, 1.f);

			// Handle circle: black border, white fill
			brush.setColorRGB(0.f, 0.f, 0.f);
			brush.fillCircle(cx - handleR - 1.f, handleY - handleR - 1.f, (handleR + 1.f) * 2.f, (handleR + 1.f) * 2.f);
			brush.setColorRGB(1.f, 1.f, 1.f);
			brush.fillCircle(cx - handleR, handleY - handleR, handleR * 2.f, handleR * 2.f);
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

	void clampTextFontIndex()
	{
		if (availableFonts.isEmpty()) { textFontIndex = 0; return; }
		textFontIndex = bbe::Math::clamp(textFontIndex, 0, (int32_t)availableFonts.getLength() - 1);
	}

	static bool isFontUsable(const bbe::String &path)
	{
		if (path == "OpenSansRegular.ttf") return true;

		// Read raw font bytes
		std::ifstream file(path.getRaw(), std::ios::binary | std::ios::ate);
		if (!file) return false;
		const auto size = file.tellg();
		if (size <= 0) return false;
		file.seekg(0);
		std::vector<unsigned char> data(static_cast<size_t>(size));
		if (!file.read(reinterpret_cast<char *>(data.data()), size)) return false;

		// Check stb_truetype can initialise the font
		stbtt_fontinfo info = {};
		const int offset = stbtt_GetFontOffsetForIndex(data.data(), 0);
		if (offset < 0) return false;
		if (!stbtt_InitFont(&info, data.data(), offset)) return false;

		// Require the basic Latin glyphs used by the default "Text" string
		for (const int c : { 'T', 'e', 'x', 't' })
		{
			if (stbtt_FindGlyphIndex(&info, c) == 0) return false;
		}
		return true;
	}

	void buildAvailableFontList()
	{
		availableFonts.clear();
		availableFonts.add({"OpenSansRegular", "OpenSansRegular.ttf"});

		bbe::List<bbe::String> fontDirs;
#ifdef _WIN32
		fontDirs.add("C:/Windows/Fonts/");
#else
		fontDirs.add("/usr/share/fonts/");
		fontDirs.add("/usr/local/share/fonts/");
		{
			const char *home = std::getenv("HOME");
			if (home)
			{
				fontDirs.add(bbe::String(home) + "/.fonts/");
				fontDirs.add(bbe::String(home) + "/.local/share/fonts/");
			}
		}
#endif

		for (size_t d = 0; d < fontDirs.getLength(); d++)
		{
			const std::string dirStr(fontDirs[d].getRaw());
			if (!std::filesystem::exists(dirStr)) continue;
			try
			{
				for (const auto &entry : std::filesystem::recursive_directory_iterator(dirStr))
				{
					if (!entry.is_regular_file()) continue;
					std::string ext = entry.path().extension().string();
					for (char &c : ext) c = (char)std::tolower((unsigned char)c);
					if (ext != ".ttf" && ext != ".otf") continue;

					FontEntry fe;
					fe.displayName = entry.path().stem().string().c_str();
					fe.path        = entry.path().string().c_str();
					if (!isFontUsable(fe.path)) continue;
					availableFonts.add(fe);
				}
			}
			catch (...) {}
		}

		// Sort alphabetically by display name (keep OpenSansRegular at index 0 by sorting from 1)
		if (availableFonts.getLength() > 2)
		{
			// Extract tail, sort, reinsert
			bbe::List<FontEntry> tail;
			for (size_t i = 1; i < availableFonts.getLength(); i++) tail.add(availableFonts[i]);
			tail.sort([](const FontEntry &a, const FontEntry &b) { return a.displayName < b.displayName; });
			while (availableFonts.getLength() > 1) availableFonts.popBack();
			for (size_t i = 0; i < tail.getLength(); i++) availableFonts.add(tail[i]);
		}
	}

	const bbe::Font &getTextToolFont() const
	{
		using FontKey = std::pair<std::string, int32_t>;
		static std::map<FontKey, bbe::Font> textFonts;
		const int32_t clampedSize = bbe::Math::max<int32_t>(textFontSize, 1);
		const bbe::String &fontPath = (textFontIndex >= 0 && textFontIndex < (int32_t)availableFonts.getLength())
			? availableFonts[(size_t)textFontIndex].path
			: bbe::String("OpenSansRegular.ttf");
		const FontKey key = { std::string(fontPath.getRaw()), clampedSize };
		auto it = textFonts.find(key);
		if (it == textFonts.end())
		{
			it = textFonts.emplace(key, bbe::Font(fontPath, (unsigned)clampedSize)).first;
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

	// Renders the text (as it would appear at topLeft) into a standalone image.
	bbe::Image renderTextToImage(const bbe::Vector2i &topLeft, const bbe::Colori &color) const
	{
		bbe::Vector2 origin;
		bbe::Rectangle bounds;
		if (!getTextOriginAndBounds(topLeft, origin, bounds)) return {};

		const int32_t imgW = (int32_t)std::ceil(bounds.width);
		const int32_t imgH = (int32_t)std::ceil(bounds.height);
		if (imgW <= 0 || imgH <= 0) return {};

		bbe::Image img(imgW, imgH, bbe::Color(0.f, 0.f, 0.f, 0.f));
		prepareImageForCanvas(img);

		const bbe::String text = getTextBufferString();
		const bbe::Font &font = getTextToolFont();
		const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(origin, text);

		auto it = text.getIterator();
		for (size_t i = 0; i < renderPositions.getLength() && it.valid(); i++, ++it)
		{
			const int32_t codePoint = it.getCodepoint();
			if (codePoint == ' ' || codePoint == '\n' || codePoint == '\r' || codePoint == '\t') continue;

			const bbe::Image &glyph = getTextGlyphImage(font, codePoint);
			const int32_t gx = (int32_t)bbe::Math::round(renderPositions[i].x) - topLeft.x;
			const int32_t gy = (int32_t)bbe::Math::round(renderPositions[i].y) - topLeft.y;

			for (int32_t x = 0; x < glyph.getWidth(); x++)
			{
				for (int32_t y = 0; y < glyph.getHeight(); y++)
				{
					const int32_t px = gx + x;
					const int32_t py = gy + y;
					if (px < 0 || py < 0 || px >= imgW || py >= imgH) continue;

					const bbe::Colori glyphColor = glyph.getPixel((size_t)x, (size_t)y);
					if (glyphColor.r == 0) continue;

					// Store (textColor, coverage) in straight-alpha format so bilinear
					// interpolation during rotation preserves correct color at edges.
					const bbe::byte coverage = static_cast<bbe::byte>((uint32_t(color.a) * uint32_t(glyphColor.r)) / 255u);
					const bbe::byte existing  = img.getPixel((size_t)px, (size_t)py).a;
					if (coverage > existing)
						img.setPixel((size_t)px, (size_t)py, bbe::Colori(color.r, color.g, color.b, coverage));
				}
			}
		}
		return img;
	}

	bool drawTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color)
	{
		bbe::Vector2 origin;
		bbe::Rectangle bounds;
		if (!getTextOriginAndBounds(topLeft, origin, bounds)) return false;

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
		return changed;
	}

	void placeTextAt(const bbe::Vector2i &topLeft, const bbe::Colori &color)
	{
		bool changed = drawTextAt(topLeft, color);
		if (changed)
		{
			submitCanvas();
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
			buffer.write(layer.opacity);
			uint8_t blendModeRaw = (uint8_t)layer.blendMode;
			buffer.write(blendModeRaw);
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
				layer.blendMode = (LayerBlendMode)blendModeRaw;
			}
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
		bool ok;
		if (isLayeredDocumentPath(filePath))
			ok = saveLayeredDocument(filePath);
		else
			ok = saveFlattenedPng(filePath);
		if (ok) savedGeneration = canvasGeneration;
		return ok;
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

	void submitCanvas()
	{
		canvas.submit();
		canvasGeneration++;
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
		symmetryOffsetCustom = false;
		if (clearHistory)
		{
			canvas.clearHistory();
			canvasGeneration = 0;
			savedGeneration = 0;
		}
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
		canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, LayerBlendMode::Normal, bbe::Image(path) });
		activeLayerIndex = 0;
		this->path = path;
		setupCanvas();
		return true;
	}

	bbe::Vector2 screenToCanvas(const bbe::Vector2 &pos)
	{
		return (pos - offset) / zoomLevel;
	}

	bbe::Rectangle getNavigatorRect()
	{
		const float canvasW = (float)getCanvasWidth();
		const float canvasH = (float)getCanvasHeight();
		if (canvasW <= 0.f || canvasH <= 0.f) return {};
		const float navMaxSize = 160.f * bbe::Math::sqrt(getWindow()->getScale());
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
		return bbe::Rectangle(getWindowWidth() - navW - margin, getWindowHeight() - navH - margin, navW, navH);
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

	bbe::Vector2 getSymmetryCenter() const
	{
		if (symmetryOffsetCustom) return symmetryOffset;
		return { getCanvasWidth() * 0.5f, getCanvasHeight() * 0.5f };
	}

	// Returns all canvas-space positions for the given position under the active symmetry mode.
	bbe::List<bbe::Vector2> getSymmetryPositions(const bbe::Vector2 &pos) const
	{
		bbe::List<bbe::Vector2> result;
		const bbe::Vector2 center = getSymmetryCenter();
		switch (symmetryMode)
		{
		case SymmetryMode::None:
			result.add(pos);
			break;
		case SymmetryMode::Horizontal:
			result.add(pos);
			result.add({ pos.x, 2.f * center.y - pos.y });
			break;
		case SymmetryMode::Vertical:
			result.add(pos);
			result.add({ 2.f * center.x - pos.x, pos.y });
			break;
		case SymmetryMode::FourWay:
			result.add(pos);
			result.add({ 2.f * center.x - pos.x, pos.y });
			result.add({ pos.x, 2.f * center.y - pos.y });
			result.add({ 2.f * center.x - pos.x, 2.f * center.y - pos.y });
			break;
		case SymmetryMode::Radial:
		{
			const float step = 2.f * bbe::Math::PI / (float)radialSymmetryCount;
			for (int32_t i = 0; i < radialSymmetryCount; i++)
				result.add(pos.rotate(step * (float)i, center));
			break;
		}
		}
		return result;
	}

	// Returns the extra rotation angle (radians) for each copy returned by getSymmetryPositions.
	// Only radial symmetry produces non-zero angles; mirror modes return all zeros.
	bbe::List<float> getSymmetryRotationAngles() const
	{
		bbe::List<float> result;
		switch (symmetryMode)
		{
		case SymmetryMode::None:
			result.add(0.f);
			break;
		case SymmetryMode::Horizontal:
		case SymmetryMode::Vertical:
			result.add(0.f);
			result.add(0.f);
			break;
		case SymmetryMode::FourWay:
			result.add(0.f);
			result.add(0.f);
			result.add(0.f);
			result.add(0.f);
			break;
		case SymmetryMode::Radial:
		{
			const float step = 2.f * bbe::Math::PI / (float)radialSymmetryCount;
			for (int32_t i = 0; i < radialSymmetryCount; i++)
				result.add(step * (float)i);
			break;
		}
		}
		return result;
	}

	bool touch(const bbe::Vector2 &touchPos, bool rectangleShape = false)
	{
		bool changed = false;
		const auto positions = getSymmetryPositions(touchPos);
		for (size_t i = 0; i < positions.getLength(); i++)
			changed |= touchImage(workArea, positions[i], getMouseColor(), brushWidth, rectangleShape, tiled);
		return changed;
	}

	bool touchLine(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, bool rectangleShape = false)
	{
		bool changed = false;
		const auto starts = getSymmetryPositions(pos1);
		const auto ends   = getSymmetryPositions(pos2);
		for (size_t i = 0; i < starts.getLength(); i++)
			changed |= touchLineImage(workArea, starts[i], ends[i], getMouseColor(), brushWidth, rectangleShape, tiled);
		return changed;
	}

	void touchLineSymmetry(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, const bbe::Colori &color, int32_t width, bool rectShape = false)
	{
		const auto starts = getSymmetryPositions(pos1);
		const auto ends   = getSymmetryPositions(pos2);
		for (size_t i = 0; i < starts.getLength(); i++)
			touchLineImage(workArea, starts[i], ends[i], color, width, rectShape, tiled);
	}

	void drawArrowSymmetry(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color)
	{
		const auto froms = getSymmetryPositions(from);
		const auto tos   = getSymmetryPositions(to);
		for (size_t i = 0; i < froms.getLength(); i++)
			drawArrowToWorkArea(froms[i], tos[i], color);
	}

	void drawBezierSymmetry(const bbe::List<bbe::Vector2> &points, const bbe::Colori &color)
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

	virtual void onStart() override
	{
		buildAvailableFontList();
		newCanvas(400, 300);
	}
	virtual void onFilesDropped(const bbe::List<bbe::String> &paths) override
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
			if (previousMode == MODE_LINE && lineDraftActive)
			{
				finalizeLineDraft();
			}
			if (previousMode == MODE_ARROW && arrowDraftActive)
			{
				finalizeArrowDraft();
			}
			if (previousMode == MODE_BEZIER && !bezierControlPoints.isEmpty())
			{
				finalizeBezierDraft();
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
		if (isKeyPressed(bbe::Key::F1) && symmetryMode != SymmetryMode::None)
		{
			symmetryOffsetCustom = true;
			symmetryOffset = screenToCanvas(getMouse());
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
		if (isKeyPressed(bbe::Key::_9))
		{
			mode = MODE_ARROW;
		}
		if (isKeyPressed(bbe::Key::_0))
		{
			mode = MODE_BEZIER;
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
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE || mode == MODE_CIRCLE || mode == MODE_ARROW || mode == MODE_BEZIER)
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
				canvasGeneration--;
				clampActiveLayerIndex();
				clearSelectionState();
				clearWorkArea();
			}
			if (isKeyTyped(bbe::Key::Y) && canvas.isRedoable())
			{
				canvas.redo();
				canvasGeneration++;
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
			if (modeBeforeInput == MODE_LINE && lineDraftActive)
			{
				finalizeLineDraft();
			}
			if (modeBeforeInput == MODE_ARROW && arrowDraftActive)
			{
				finalizeArrowDraft();
			}
			if (modeBeforeInput == MODE_BEZIER && !bezierControlPoints.isEmpty())
			{
				finalizeBezierDraft();
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

		const bool mouseOnNavigator = showNavigator
			&& getCanvasWidth() > 0
			&& getNavigatorRect().isPointInRectangle(getMouse());

		if (mouseOnNavigator && isMouseDown(bbe::MouseButton::LEFT))
		{
			const bbe::Rectangle navRect = getNavigatorRect();
			const float canvasClickX = (getMouse().x - navRect.x) / navRect.width  * getCanvasWidth();
			const float canvasClickY = (getMouse().y - navRect.y) / navRect.height * getCanvasHeight();
			offset.x = getWindowWidth()  / 2.f - canvasClickX * zoomLevel;
			offset.y = getWindowHeight() / 2.f - canvasClickY * zoomLevel;
		}

		if (!mouseOnNavigator && (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT)))
		{
			startMousePos = screenToCanvas(getMouse());
		}

		// Canvas resize handles — checked before tool handling so they take priority.
		if (!mouseOnNavigator && !canvasResizeActive && isMousePressed(bbe::MouseButton::LEFT))
		{
			const int32_t hitHandle = getCanvasResizeHitHandle(getMouse());
			if (hitHandle >= 0)
			{
				canvasResizeActive = true;
				canvasResizeHandleIndex = hitHandle;
				canvasResizePreviewRect = { 0, 0, getCanvasWidth(), getCanvasHeight() };
				updateCanvasResizePreview(currMousePos);
			}
		}
		if (canvasResizeActive)
		{
			if (isMouseDown(bbe::MouseButton::LEFT))
			{
				updateCanvasResizePreview(currMousePos);
			}
			if (isMouseReleased(bbe::MouseButton::LEFT))
			{
				applyCanvasResize(canvasResizePreviewRect);
				canvasResizeActive = false;
				canvasResizeHandleIndex = -1;
				canvasResizePreviewRect = {};
			}
		}

		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_SELECTION)
		{
			updateSelectionTool(currMousePos);
		}
		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_RECTANGLE)
		{
			updateRectangleTool(currMousePos);
		}
		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_CIRCLE)
		{
			updateCircleTool(currMousePos);
		}
		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_LINE)
		{
			updateLineTool(currMousePos);
		}
		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_ARROW)
		{
			updateArrowTool(currMousePos);
		}
		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_BEZIER)
		{
			updateBezierTool(currMousePos);
		}
		if (!mouseOnNavigator && !canvasResizeActive && mode == MODE_TEXT && (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT)))
		{
			bbe::Vector2 pos = currMousePos;
			if (toTiledPos(pos))
			{
				bool textChanged = false;
				const bbe::Vector2i originalTopLeft = toCanvasPixel(pos);
				const auto symPositions = getSymmetryPositions(pos);
				const auto symAngles    = getSymmetryRotationAngles();

				// Pre-render the text image and compute the symmetric positions of its
				// canvas-space centre.  blendRotatedImageOntoCanvas rotates around the
				// centre of the dest rect, so we must derive that rect from the rotated
				// IMAGE CENTRE — not from the rotated click position (top-left corner).
				const bbe::Image textImg = renderTextToImage(originalTopLeft, getMouseColor());
				const bbe::Vector2 imgCenter = {
					originalTopLeft.x + textImg.getWidth()  * 0.5f,
					originalTopLeft.y + textImg.getHeight() * 0.5f
				};
				const auto imgCenterSymPositions = getSymmetryPositions(imgCenter);

				for (size_t i = 0; i < symPositions.getLength(); i++)
				{
					bbe::Vector2 symPos = symPositions[i];
					if (!toTiledPos(symPos)) continue;
					const bbe::Vector2i symTopLeft = toCanvasPixel(symPos);
					if (std::abs(symAngles[i]) > 0.0001f)
					{
						if (textImg.getWidth() > 0 && textImg.getHeight() > 0)
						{
							const bbe::Rectanglei symRect = {
								(int32_t)std::round(imgCenterSymPositions[i].x - textImg.getWidth()  * 0.5f),
								(int32_t)std::round(imgCenterSymPositions[i].y - textImg.getHeight() * 0.5f),
								textImg.getWidth(),
								textImg.getHeight()
							};
							blendRotatedImageOntoCanvas(textImg, symRect, symAngles[i]);
							textChanged = true;
						}
					}
					else
					{
						textChanged |= drawTextAt(symTopLeft, getMouseColor());
					}
				}
				if (textChanged) submitCanvas();
			}
		}

		// TODO: Would be nice if we had a constexpr list
		const bbe::List<decltype(mode)> shadowDrawModes = { MODE_BRUSH };
		const bool drawMode = mode != MODE_SELECTION
			&& mode != MODE_TEXT
			&& mode != MODE_RECTANGLE
			&& mode != MODE_CIRCLE
			&& mode != MODE_LINE
			&& mode != MODE_ARROW
			&& mode != MODE_BEZIER
			&& !canvasResizeActive
			&& !mouseOnNavigator
			&& drawButtonDown;
		const bool shadowDrawMode = shadowDrawModes.contains(mode);

		if (changeRegistered)
		{
			if (isMouseReleased(bbe::MouseButton::LEFT) || isMouseReleased(bbe::MouseButton::RIGHT))
			{
				if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT))
				{
					applyWorkArea();
					submitCanvas(); // <- changeRegistered is for this
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
					const auto symPositions = getSymmetryPositions(pos);
					for (size_t i = 0; i < symPositions.getLength(); i++)
					{
						bbe::Vector2 symPos = symPositions[i];
						if (toTiledPos(symPos))
							getActiveLayerImage().floodFill(symPos.as<int32_t>(), getMouseColor(), false, tiled);
					}
					changeRegistered = true;
				}
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
			canvasGeneration--;
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
			canvasGeneration++;
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
		if (lineDraftActive && ((leftColorChanged && !lineDraftUsesRightColor) || (rightColorChanged && lineDraftUsesRightColor)))
		{
			clearWorkArea();
			touchLineSymmetry(lineDraftEnd, lineDraftStart, getLineDraftColor(), brushWidth);
		}
		if (arrowDraftActive && ((leftColorChanged && !arrowDraftUsesRightColor) || (rightColorChanged && arrowDraftUsesRightColor)))
		{
			clearWorkArea();
			drawArrowSymmetry(arrowDraftStart, arrowDraftEnd, getArrowDraftColor());
		}
		if (!bezierControlPoints.isEmpty() && ((leftColorChanged && !bezierUsesRightColor) || (rightColorChanged && bezierUsesRightColor)))
		{
			clearWorkArea();
			drawBezierSymmetry(bezierControlPoints, getBezierColor());
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
			toolBtn("Arrow",     MODE_ARROW);       ImGui::SameLine();
			toolBtn("Bezier",    MODE_BEZIER);
		}

		// --- Tool options ---
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE || mode == MODE_CIRCLE || mode == MODE_TEXT || mode == MODE_ARROW || mode == MODE_BEZIER)
		{
			ImGui::SeparatorText("Options");
		}
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE || mode == MODE_CIRCLE || mode == MODE_ARROW || mode == MODE_BEZIER)
		{
			if (ImGui::InputInt("Width", &brushWidth))
			{
				clampBrushWidth();
				if (rectangleDraftActive) refreshActiveRectangleDraftImage();
				if (circleDraftActive) refreshActiveCircleDraftImage();
				if (lineDraftActive)
				{
					clearWorkArea();
					touchLineSymmetry(lineDraftEnd, lineDraftStart, getLineDraftColor(), brushWidth);
				}
				if (arrowDraftActive)
				{
					clearWorkArea();
					drawArrowSymmetry(arrowDraftStart, arrowDraftEnd, getArrowDraftColor());
				}
				if (!bezierControlPoints.isEmpty())
				{
					clearWorkArea();
					drawBezierSymmetry(bezierControlPoints, getBezierColor());
				}
			}
		}
		if (mode == MODE_RECTANGLE)
		{
			if (ImGui::InputInt("Corner Radius", &cornerRadius))
			{
				if (cornerRadius < 0) cornerRadius = 0;
				if (rectangleDraftActive) refreshActiveRectangleDraftImage();
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
		if (mode == MODE_LINE)
		{
			ImGui::TextDisabled(lineDraftActive
				? "Drag endpoints to adjust.\nClick outside or R-click to place."
				: "Drag to draw.");
		}
		if (mode == MODE_ARROW)
		{
			bool arrowOptionChanged = false;
			if (ImGui::InputInt("Head Size", &arrowHeadSize))
			{
				if (arrowHeadSize < 1) arrowHeadSize = 1;
				arrowOptionChanged = true;
			}
			if (ImGui::InputInt("Head Width", &arrowHeadWidth))
			{
				if (arrowHeadWidth < 1) arrowHeadWidth = 1;
				arrowOptionChanged = true;
			}
			if (ImGui::Checkbox("Double Headed", &arrowDoubleHeaded)) arrowOptionChanged = true;
			if (ImGui::Checkbox("Filled Head",   &arrowFilledHead))   arrowOptionChanged = true;
			if (arrowOptionChanged && arrowDraftActive)
			{
				clearWorkArea();
				drawArrowToWorkArea(arrowDraftStart, arrowDraftEnd, getArrowDraftColor());
			}
			ImGui::TextDisabled(arrowDraftActive
				? "Drag endpoints to adjust.\nClick outside or R-click to place."
				: "Drag to draw.");
		}
		if (mode == MODE_BEZIER)
		{
			ImGui::TextDisabled(bezierControlPoints.isEmpty()
				? "L-click to place control points.\nR-click to commit curve."
				: "L-click to add/drag points.\nBackspace removes last point.\nR-click to commit curve.");
			if (!bezierControlPoints.isEmpty())
			{
				ImGui::Text("%d control point(s)", (int)bezierControlPoints.getLength());
				if (ImGui::Button("Commit", ImVec2(-1, 0)))
				{
					finalizeBezierDraft();
				}
			}
		}
		if (mode == MODE_TEXT)
		{
			// Font picker
			static char fontFilter[128] = "";
			ImGui::InputText("Filter##fontFilter", fontFilter, sizeof(fontFilter));
			clampTextFontIndex();
			const char *currentFontName = availableFonts.isEmpty() ? "None" : availableFonts[(size_t)textFontIndex].displayName.getRaw();
			if (ImGui::BeginCombo("Font", currentFontName))
			{
				for (size_t i = 0; i < availableFonts.getLength(); i++)
				{
					const char *name = availableFonts[i].displayName.getRaw();
					if (fontFilter[0] != '\0')
					{
						std::string nameLower = name;
						std::string filterLower = fontFilter;
						for (char &c : nameLower)   c = (char)std::tolower((unsigned char)c);
						for (char &c : filterLower) c = (char)std::tolower((unsigned char)c);
						if (nameLower.find(filterLower) == std::string::npos) continue;
					}
					const bool selected = (textFontIndex == (int32_t)i);
					if (ImGui::Selectable(name, selected))
					{
						textFontIndex = (int32_t)i;
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (ImGui::InputInt("Font Size", &textFontSize))
			{
				clampTextFontSize();
			}
			ImGui::InputTextMultiline("##text", textBuffer, sizeof(textBuffer), ImVec2(-1, ImGui::GetTextLineHeight() * 4.0f));
			ImGui::TextDisabled("L/R click places text.");
		}

		// --- Symmetry ---
		ImGui::SeparatorText("Symmetry");
		{
			const float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 4) / 5.f;
			auto symBtn = [&](const char* label, SymmetryMode sm)
			{
				const bool active = (symmetryMode == sm);
				if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(label, ImVec2(w, 0))) symmetryMode = sm;
				if (active) ImGui::PopStyleColor();
			};
			symBtn("Off",  SymmetryMode::None);       ImGui::SameLine();
			symBtn("H",    SymmetryMode::Horizontal);  ImGui::SameLine();
			symBtn("V",    SymmetryMode::Vertical);    ImGui::SameLine();
			symBtn("4W",   SymmetryMode::FourWay);     ImGui::SameLine();
			symBtn("Rad",  SymmetryMode::Radial);
			if (symmetryMode == SymmetryMode::Radial)
			{
				if (ImGui::InputInt("Spokes##radialCount", &radialSymmetryCount))
				{
					if (radialSymmetryCount < 2) radialSymmetryCount = 2;
					if (radialSymmetryCount > 32) radialSymmetryCount = 32;
				}
			}
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
			canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, LayerBlendMode::Normal, bbe::Image::getClipboardImage() });
			path = "";
			submitCanvas();
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

			const float halfW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			if (ImGui::Button("Dup", ImVec2(halfW, 0))) duplicateActiveLayer();
			ImGui::SameLine();
			ImGui::BeginDisabled(activeLayerIndex <= 0);
			if (ImGui::Button("Merge Dn", ImVec2(halfW, 0))) mergeActiveLayerDown();
			ImGui::EndDisabled();
		}
		if (!canvas.get().layers.isEmpty())
		{
			if (ImGui::bbe::InputText("Name##layerName", getActiveLayer().name))
			{
				submitCanvas();
			}
			float opacity = getActiveLayer().opacity;
			if (ImGui::SliderFloat("Opacity##layerOpacity", &opacity, 0.0f, 1.0f))
			{
				getActiveLayer().opacity = opacity;
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				submitCanvas();
			}
			const char *blendModeNames[] = { "Normal", "Multiply", "Screen", "Overlay" };
			int blendModeIdx = (int)getActiveLayer().blendMode;
			if (ImGui::Combo("Blend##layerBlend", &blendModeIdx, blendModeNames, 4))
			{
				getActiveLayer().blendMode = (LayerBlendMode)blendModeIdx;
				submitCanvas();
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
					submitCanvas();
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

		bool anyNonNormalBlendMode = false;
		for (size_t layerIndex = 0; layerIndex < canvas.get().layers.getLength(); layerIndex++)
		{
			const PaintLayer &layer = canvas.get().layers[layerIndex];
			if (layer.visible && layer.blendMode != LayerBlendMode::Normal)
			{
				anyNonNormalBlendMode = true;
				break;
			}
		}
		bbe::Image blendModePreview;
		if (anyNonNormalBlendMode)
		{
			blendModePreview = flattenVisibleLayers();
		}

		const int32_t repeats = tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				if (anyNonNormalBlendMode)
				{
					brush.drawImage(offset.x + i * getCanvasWidth() * zoomLevel, offset.y + k * getCanvasHeight() * zoomLevel, getCanvasWidth() * zoomLevel, getCanvasHeight() * zoomLevel, blendModePreview);
				}
				else
				{
					for (size_t layerIndex = 0; layerIndex < canvas.get().layers.getLength(); layerIndex++)
					{
						const PaintLayer &layer = canvas.get().layers[layerIndex];
						if (!layer.visible) continue;
						brush.setColorRGB(1.0f, 1.0f, 1.0f, layer.opacity);
						brush.drawImage(offset.x + i * getCanvasWidth() * zoomLevel, offset.y + k * getCanvasHeight() * zoomLevel, getCanvasWidth() * zoomLevel, getCanvasHeight() * zoomLevel, layer.image);
					}
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
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
		// Canvas resize handles
		if (getCanvasWidth() > 0 && getCanvasHeight() > 0 && !tiled)
		{
			constexpr float hs = 5.f;
			for (int32_t i = 0; i < 8; i++)
			{
				const bbe::Vector2 hp = getCanvasHandleScreenPos(i);
				brush.setColorRGB(1.f, 1.f, 1.f);
				brush.fillRect(hp.x - hs, hp.y - hs, hs * 2.f, hs * 2.f);
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.sketchRect(bbe::Rectangle(hp.x - hs, hp.y - hs, hs * 2.f, hs * 2.f));
			}
			if (canvasResizeActive && canvasResizePreviewRect.width > 0 && canvasResizePreviewRect.height > 0)
			{
				const bbe::Rectangle previewScreen = selectionRectToScreen(canvasResizePreviewRect);
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.sketchRect(previewScreen);
				if (previewScreen.width > 2 && previewScreen.height > 2)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.sketchRect(previewScreen.shrinked(1.f));
				}
			}
		}

		const int32_t ghostRepeats = tiled ? 20 : 0;
		auto drawInAllTiles = [&](const bbe::Rectanglei &rect, const bbe::Image &image, float rotation = 0.f)
		{
			// Pre-rasterize rotation so the preview matches the committed pixel-grid result.
			const bool hasRot = std::abs(rotation) > 0.0001f;
			bbe::Image rotatedImg;
			bbe::Rectanglei displayRect = rect;
			const bbe::Image *pImg = &image;
			if (hasRot)
			{
				rotatedImg = createRotatedPreviewImage(image, rotation);
				pImg = &rotatedImg;
				const float cx = rect.x + rect.width / 2.f;
				const float cy = rect.y + rect.height / 2.f;
				displayRect = bbe::Rectanglei(
					(int32_t)std::floor(cx - rotatedImg.getWidth() / 2.f),
					(int32_t)std::floor(cy - rotatedImg.getHeight() / 2.f),
					rotatedImg.getWidth(),
					rotatedImg.getHeight());
			}
			for (int32_t i = -ghostRepeats; i <= ghostRepeats; i++)
			{
				for (int32_t k = -ghostRepeats; k <= ghostRepeats; k++)
				{
					const bbe::Rectanglei tileDisplay(
						displayRect.x + i * getCanvasWidth(),
						displayRect.y + k * getCanvasHeight(),
						displayRect.width,
						displayRect.height);
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
					brush.drawImage(selectionRectToScreen(tileDisplay), *pImg);
					const bbe::Rectanglei tileOutline(
						rect.x + i * getCanvasWidth(),
						rect.y + k * getCanvasHeight(),
						rect.width,
						rect.height);
					drawSelectionOutline(brush, tileOutline);
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
			drawInAllTiles(selectionPreviewRect, previewImage, selectionRotation);
		}
		else if (selectionFloating)
		{
			drawInAllTiles(selectionRect, selectionFloatingImage, selectionRotation);
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
		auto drawEndpointHandle = [&](const bbe::Vector2 &canvasPos)
		{
			const float sx = offset.x + canvasPos.x * zoomLevel;
			const float sy = offset.y + canvasPos.y * zoomLevel;
			constexpr float hs = 4.f;
			brush.setColorRGB(1.f, 1.f, 1.f);
			brush.fillRect(sx - hs, sy - hs, hs * 2.f, hs * 2.f);
			brush.setColorRGB(0.f, 0.f, 0.f);
			brush.sketchRect(bbe::Rectangle(sx - hs, sy - hs, hs * 2.f, hs * 2.f));
		};
		if (mode == MODE_LINE && lineDraftActive)
		{
			drawEndpointHandle(lineDraftStart);
			drawEndpointHandle(lineDraftEnd);
		}
		if (mode == MODE_ARROW && arrowDraftActive)
		{
			drawEndpointHandle(arrowDraftStart);
			drawEndpointHandle(arrowDraftEnd);
		}
		if (mode == MODE_BEZIER && !bezierControlPoints.isEmpty())
		{
			// Draw the control polygon
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.6f);
			for (size_t i = 0; i + 1 < bezierControlPoints.getLength(); i++)
			{
				const float x0 = offset.x + bezierControlPoints[i    ].x * zoomLevel;
				const float y0 = offset.y + bezierControlPoints[i    ].y * zoomLevel;
				const float x1 = offset.x + bezierControlPoints[i + 1].x * zoomLevel;
				const float y1 = offset.y + bezierControlPoints[i + 1].y * zoomLevel;
				brush.fillLine(x0, y0, x1, y1);
			}
			// Draw handles for each control point
			for (size_t i = 0; i < bezierControlPoints.getLength(); i++)
			{
				drawEndpointHandle(bezierControlPoints[i]);
			}
		}

		// Symmetry guide lines
		if (symmetryMode != SymmetryMode::None && getCanvasWidth() > 0)
		{
			const float cw = (float)getCanvasWidth();
			const float ch = (float)getCanvasHeight();
			const bbe::Vector2 center = getSymmetryCenter();
			// Convert canvas coords to screen coords: screen = pos * zoomLevel + offset
			auto c2s = [&](bbe::Vector2 p) -> bbe::Vector2
			{
				return p * zoomLevel + offset;
			};

			brush.setColorRGB(0.2f, 0.8f, 1.0f, 0.7f);
			brush.setOutlineWidth(0.f);

			if (symmetryMode == SymmetryMode::Horizontal || symmetryMode == SymmetryMode::FourWay)
			{
				const bbe::Vector2 a = c2s({ 0.f,  center.y });
				const bbe::Vector2 b = c2s({ cw,   center.y });
				brush.fillLine(a, b, 1.f);
			}
			if (symmetryMode == SymmetryMode::Vertical || symmetryMode == SymmetryMode::FourWay)
			{
				const bbe::Vector2 a = c2s({ center.x, 0.f });
				const bbe::Vector2 b = c2s({ center.x, ch  });
				brush.fillLine(a, b, 1.f);
			}
			if (symmetryMode == SymmetryMode::Radial)
			{
				const float step = 2.f * bbe::Math::PI / (float)radialSymmetryCount;
				const float extent = bbe::Math::sqrt(cw * cw + ch * ch) * 0.5f;
				for (int32_t i = 0; i < radialSymmetryCount; i++)
				{
					const float angle = step * (float)i;
					const bbe::Vector2 dir = { std::cosf(angle) * extent, std::sinf(angle) * extent };
					brush.fillLine(c2s(center), c2s(center + dir), 1.f);
				}
			}
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Navigator
		if (showNavigator && getCanvasWidth() > 0)
		{
			const bbe::Rectangle navRect = getNavigatorRect();
			const float navX = navRect.x;
			const float navY = navRect.y;
			const float navW = navRect.width;
			const float navH = navRect.height;

			// Background
			brush.setColorRGB(0.08f, 0.08f, 0.08f);
			brush.fillRect(navX - 2.f, navY - 2.f, navW + 4.f, navH + 4.f);

			// Layers
			if (anyNonNormalBlendMode)
			{
				brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
				brush.drawImage(navX, navY, navW, navH, blendModePreview);
			}
			else
			{
				for (size_t layerIndex = 0; layerIndex < canvas.get().layers.getLength(); layerIndex++)
				{
					const PaintLayer &layer = canvas.get().layers[layerIndex];
					if (!layer.visible) continue;
					brush.setColorRGB(1.0f, 1.0f, 1.0f, layer.opacity);
					brush.drawImage(navX, navY, navW, navH, layer.image);
				}
			}
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);

			// Viewport rectangle (clamped to navigator bounds)
			const float scaleX = navW / getCanvasWidth();
			const float scaleY = navH / getCanvasHeight();
			const bbe::Vector2 tlCanvas = screenToCanvas({ 0.f, 0.f });
			const bbe::Vector2 brCanvas = screenToCanvas({ (float)getWindowWidth(), (float)getWindowHeight() });
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
				if (ImGui::MenuItem("Navigator", nullptr, showNavigator))
				{
					showNavigator = !showNavigator;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Preferences"))
			{
				if (ImGui::MenuItem("Anti-Aliasing", nullptr, antiAliasingEnabled))
				{
					antiAliasingEnabled = !antiAliasingEnabled;
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
			if (canvasGeneration != savedGeneration)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f);
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "*");
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Unsaved changes");
			}
			ImGui::EndMainMenuBar();
		}

		if (openSaveChoicePopup)
		{
			ImGui::OpenPopup("Save Document");
			openSaveChoicePopup = false;
		}
		if (openDropChoicePopup)
		{
			ImGui::OpenPopup("Dropped File(s)");
			openDropChoicePopup = false;
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

		if (ImGui::BeginPopupModal("Dropped File(s)", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (pendingDroppedPaths.getLength() == 1)
			{
				ImGui::Text("What would you like to do with \"%s\"?",
					std::filesystem::path(pendingDroppedPaths[0].getRaw()).filename().string().c_str());
			}
			else
			{
				ImGui::Text("What would you like to do with %d dropped file(s)?",
					(int)pendingDroppedPaths.getLength());
			}
			ImGui::Spacing();
			if (ImGui::Button("Open as Document", ImVec2(160, 0)))
			{
				// Use only the first valid file as the new document
				newCanvas(pendingDroppedPaths[0].getRaw());
				pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add as Layer(s)", ImVec2(160, 0)))
			{
				importFileAsLayers(pendingDroppedPaths);
				pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				pendingDroppedPaths.clear();
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
				ImGui::BulletText("9 Arrow");

				ImGui::SeparatorText("General");
				ImGui::BulletText("+/- changes brush size or text size for the active tool");
				ImGui::BulletText("X swaps primary and secondary color");
				ImGui::BulletText("Ctrl+D resets colors to black/white");
				ImGui::BulletText("Drag and drop PNG or .bbepaint files to open as a document or add as a new layer");
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
