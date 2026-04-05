#include "ExamplePaintEditor.h"
#include "ExamplePaintGui.h"
#include "ExamplePaintPalette.h"
#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#ifdef BBE_RENDERER_OPENGL
#include "BBE/glfwWrapper.h"

static void updateIconSlot(ImTextureID &texId, const bbe::Image *&cachedPtr, const bbe::Image *current)
{
	if (current == cachedPtr && texId != ImTextureID_Invalid) return;
	if (texId != ImTextureID_Invalid)
	{
		GLuint old = (GLuint)(intptr_t)texId;
		glDeleteTextures(1, &old);
		texId = ImTextureID_Invalid;
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

static GLuint s_navigatorMinimapGlTex = 0;

static void navigatorUploadRgbaTexture(int w, int h, const unsigned char *pixels)
{
	if (s_navigatorMinimapGlTex == 0)
		glGenTextures(1, &s_navigatorMinimapGlTex);
	glBindTexture(GL_TEXTURE_2D, s_navigatorMinimapGlTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

static std::vector<unsigned char> s_navigatorThumbRgbaCache;
static int s_navigatorThumbCacheW = 0;
static int s_navigatorThumbCacheH = 0;

static uint64_t paintEditorLayerStackDisplayHash(const PaintEditor &editor)
{
	uint64_t h = 14695981039346656037ull;
	const size_t n = editor.canvas.get().layers.getLength();
	for (size_t i = 0; i < n; i++)
	{
		const PaintLayer &L = editor.canvas.get().layers[i];
		h ^= L.visible ? 0x9e3779b185ebca87ull : 0;
		h *= 1099511628211ull;
		h ^= (uint64_t)i * 0xC2B2AE3D27D4EB4Full;
		union
		{
			float f;
			uint32_t u;
		} op;
		op.f = L.opacity;
		h ^= (uint64_t)op.u;
		h *= 1099511628211ull;
		h ^= (uint64_t)(int)L.blendMode * 0x100000001B3ull;
	}
	return h;
}

static uint64_t paintEditorNavigatorContentHash(const PaintEditor &editor, bool anyNonNormalBlendMode)
{
	uint64_t h = paintEditorLayerStackDisplayHash(editor);
	h ^= (uint64_t)editor.canvasGeneration;
	h *= 1099511628211ull;
	h ^= (uint64_t)editor.canvasVisualEpoch;
	h *= 1099511628211ull;
	h ^= editor.navigatorThumbnailCacheRevision;
	h *= 1099511628211ull;
	h ^= (uint64_t)(int32_t)editor.getCanvasWidth() << 32 ^ (uint32_t)editor.getCanvasHeight();
	h ^= anyNonNormalBlendMode ? 0xD6E8FEB86655A902ull : 0;
	const float *fb = editor.canvas.get().canvasFallbackRgba;
	union
	{
		float f;
		uint32_t u;
	} q[4];
	for (int i = 0; i < 4; i++)
	{
		q[i].f = fb[i];
		h ^= (uint64_t)q[i].u << (i * 8);
	}
	return h;
}

static uint64_t paintEditorNavigatorThumbPixelHash(uint64_t contentHash, int tw, int th)
{
	uint64_t h = contentHash;
	h ^= (uint64_t)(uint32_t)tw * 0xC6BC279692B5CC83ull;
	h ^= (uint64_t)(uint32_t)th * 0x9E3779B97F4A7C15ull;
	return h;
}

/// Screen-space tile indices (i,k) for repeating the canvas in tiled mode; only copies that can intersect the viewport.
static void paintEditorVisibleMainTileRange(const PaintEditor &editor, int &outIMin, int &outIMax, int &outKMin, int &outKMax)
{
	if (!editor.tiled)
	{
		outIMin = outIMax = outKMin = outKMax = 0;
		return;
	}
	const float cwz = (float)editor.getCanvasWidth() * editor.zoomLevel;
	const float chz = (float)editor.getCanvasHeight() * editor.zoomLevel;
	const float ox = editor.offset.x;
	const float oy = editor.offset.y;
	const float vw = (float)editor.viewport.width;
	const float vh = (float)editor.viewport.height;
	const float m = 2.f;
	if (cwz <= 1e-6f || chz <= 1e-6f)
	{
		outIMin = outIMax = outKMin = outKMax = 0;
		return;
	}
	outIMin = (int)std::floor((0.f - m - ox - cwz) / cwz + 1e-5f) + 1;
	outIMax = (int)std::ceil((vw + m - ox) / cwz - 1e-5f) - 1;
	outKMin = (int)std::floor((0.f - m - oy - chz) / chz + 1e-5f) + 1;
	outKMax = (int)std::ceil((vh + m - oy) / chz - 1e-5f) - 1;
	if (outIMin > outIMax)
		outIMax = outIMin - 1;
	if (outKMin > outKMax)
		outKMax = outKMin - 1;
}

/// Tile indices so that \c canvasPixelRect shifted by (ti*W, tk*H) can intersect the viewport in canvas space.
static void paintEditorVisibleGhostTileRangeForCanvasRect(PaintEditor &editor, const bbe::Rectanglei &canvasPixelRect, int &tiMin, int &tiMax, int &tkMin, int &tkMax)
{
	if (!editor.tiled)
	{
		tiMin = tiMax = tkMin = tkMax = 0;
		return;
	}
	const int W = editor.getCanvasWidth();
	const int H = editor.getCanvasHeight();
	if (W <= 0 || H <= 0)
	{
		tiMin = tiMax = tkMin = tkMax = 0;
		return;
	}
	const bbe::Vector2 c0 = editor.screenToCanvas(bbe::Vector2(0.f, 0.f));
	const bbe::Vector2 c1 = editor.screenToCanvas(bbe::Vector2((float)editor.viewport.width, (float)editor.viewport.height));
	float cvMinX = std::min(c0.x, c1.x);
	float cvMaxX = std::max(c0.x, c1.x);
	float cvMinY = std::min(c0.y, c1.y);
	float cvMaxY = std::max(c0.y, c1.y);
	const float margin = 2.f;
	const int rw = std::max(0, canvasPixelRect.width);
	const int rh = std::max(0, canvasPixelRect.height);
	const double rx = (double)canvasPixelRect.x;
	const double ry = (double)canvasPixelRect.y;
	const double tiLow = ((double)cvMinX - margin - rx - (double)rw) / (double)W;
	const double tiHigh = ((double)cvMaxX + margin - rx) / (double)W;
	tiMin = (int)std::floor(tiLow + 1e-6) + 1;
	tiMax = (int)std::ceil(tiHigh - 1e-6) - 1;
	const double tkLow = ((double)cvMinY - margin - ry - (double)rh) / (double)H;
	const double tkHigh = ((double)cvMaxY + margin - ry) / (double)H;
	tkMin = (int)std::floor(tkLow + 1e-6) + 1;
	tkMax = (int)std::ceil(tkHigh - 1e-6) - 1;
	if (tiMin > tiMax)
		tiMax = tiMin - 1;
	if (tkMin > tkMax)
		tkMax = tkMin - 1;
}

static bbe::Rectanglei paintEditorUnionRecti(const bbe::Rectanglei &a, const bbe::Rectanglei &b)
{
	const int32_t x0 = std::min(a.x, b.x);
	const int32_t y0 = std::min(a.y, b.y);
	const int32_t x1 = std::max(a.x + a.width, b.x + b.width);
	const int32_t y1 = std::max(a.y + a.height, b.y + b.height);
	return bbe::Rectanglei(x0, y0, std::max(0, x1 - x0), std::max(0, y1 - y0));
}

static bbe::Rectanglei paintEditorBoundsOfLassoPath(const PaintEditor &editor, int32_t padPx)
{
	const std::vector<bbe::Vector2> &path = editor.selection.lassoPath;
	if (path.size() < 2) return {};
	int32_t minX = INT32_MAX, minY = INT32_MAX, maxX = INT32_MIN, maxY = INT32_MIN;
	for (size_t pi = 0; pi < path.size(); pi++)
	{
		const bbe::Vector2 &pf = path[pi];
		minX = std::min(minX, (int32_t)std::floor(pf.x));
		minY = std::min(minY, (int32_t)std::floor(pf.y));
		maxX = std::max(maxX, (int32_t)std::ceil(pf.x));
		maxY = std::max(maxY, (int32_t)std::ceil(pf.y));
	}
	return bbe::Rectanglei(
		minX - padPx,
		minY - padPx,
		std::max(0, maxX - minX + padPx * 2),
		std::max(0, maxY - minY + padPx * 2));
}

static bbe::Rectanglei paintEditorBoundsOfPolygonLasso(const PaintEditor &editor, int32_t padPx)
{
	const int32_t W = editor.getCanvasWidth();
	const int32_t H = editor.getCanvasHeight();
	const auto &verts = editor.selection.polygonLassoVertices;
	if (verts.empty()) return {};
	int32_t minX = verts[0].x, minY = verts[0].y, maxX = verts[0].x, maxY = verts[0].y;
	for (size_t i = 1; i < verts.size(); i++)
	{
		minX = std::min(minX, verts[i].x);
		minY = std::min(minY, verts[i].y);
		maxX = std::max(maxX, verts[i].x);
		maxY = std::max(maxY, verts[i].y);
	}
	if (editor.hasPointerPos && W > 0 && H > 0)
	{
		const int32_t cx = bbe::Math::clamp((int32_t)std::floor(editor.lastPointerCanvasPos.x), 0, W - 1);
		const int32_t cy = bbe::Math::clamp((int32_t)std::floor(editor.lastPointerCanvasPos.y), 0, H - 1);
		minX = std::min(minX, cx);
		minY = std::min(minY, cy);
		maxX = std::max(maxX, cx);
		maxY = std::max(maxY, cy);
	}
	return bbe::Rectanglei(
		minX - padPx,
		minY - padPx,
		std::max(0, maxX - minX + padPx * 2),
		std::max(0, maxY - minY + padPx * 2));
}

struct ToolIconTextures
{
	struct Slot { ImTextureID texId = ImTextureID_Invalid; const bbe::Image *cachedPtr = nullptr; };
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

	const bbe::Rectanglei textBounds(
		topLeft.x,
		topLeft.y,
		(int32_t)bbe::Math::ceil(bounds.width),
		(int32_t)bbe::Math::ceil(bounds.height));
	int tiMin = 0, tiMax = 0, tkMin = 0, tkMax = 0;
	paintEditorVisibleGhostTileRangeForCanvasRect(editor, textBounds, tiMin, tiMax, tkMin, tkMax);
	for (int32_t ti = tiMin; ti <= tiMax; ti++)
	{
		for (int32_t tk = tkMin; tk <= tkMax; tk++)
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
				textBounds.width,
				textBounds.height),
				true);
		}
	}
}

static void paintEditorFormatDigitHotkeys(const PaintEditor &editor, PaintEditor::DigitHotkeyAction target, char *out, size_t outN)
{
	if (outN == 0) return;
	out[0] = '\0';
	size_t n = 0;
	for (int i = 0; i < 10; i++)
	{
		if (editor.digitHotkeys[i] != target) continue;
		if (n > 0 && n + 2 < outN) out[n++] = ',';
		const char d = (i < 9) ? char('1' + i) : '0';
		if (n + 1 < outN) out[n++] = d;
	}
	if (n < outN) out[n] = '\0';
}

static void paintEditorTryBindDigitHotkeyOnHover(PaintEditor &editor, PaintEditor::DigitHotkeyAction binding)
{
	if (!ImGui::IsItemHovered()) return;
	if (!ImGui::GetIO().KeyCtrl) return;
	static const ImGuiKey digitKeys[10] = {
		ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5,
		ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9, ImGuiKey_0,
	};
	for (int i = 0; i < 10; i++)
	{
		if (ImGui::IsKeyPressed(digitKeys[i], false))
		{
			editor.digitHotkeys[i] = binding;
			if (editor.onDigitHotkeysChanged) editor.onDigitHotkeysChanged();
			break;
		}
	}
}

static void paintEditorDigitHotkeyTooltip(const PaintEditor &editor, PaintEditor::DigitHotkeyAction target, const char *title)
{
	char keys[24];
	paintEditorFormatDigitHotkeys(editor, target, keys, sizeof keys);
	if (keys[0] != '\0')
		ImGui::SetTooltip("%s\nNumber keys: %s\n(Ctrl+digit while hovered assigns)", title, keys);
	else
		ImGui::SetTooltip("%s\n(No number keys; Ctrl+digit while hovered assigns)", title);
}

static void paintEditorQuantizeImageCopyIfPaletteMode(const PaintEditor &editor, bbe::Image &img)
{
	if (!editor.canvas.get().paletteMode || editor.canvas.get().paletteColors.isEmpty()) return;
	if (img.getWidth() <= 0 || img.getHeight() <= 0 || !img.isLoadedCpu()) return;
	paintPalette::quantizeImageToPaletteInPlace(img, editor.canvas.get().paletteColors, false, 50);
}

static void paintEditorRefreshDraftsAfterPrimarySecondaryChange(PaintEditor &editor, bool leftColorChanged, bool rightColorChanged)
{
	if (editor.rectangle.draftActive && (editor.shapeFillMode != PaintEditor::ShapeFillMode::None || editor.shapeStripedStroke
		? (leftColorChanged || rightColorChanged)
		: ((leftColorChanged && !editor.rectangle.draftUsesRightColor) || (rightColorChanged && editor.rectangle.draftUsesRightColor))))
	{
		editor.refreshActiveRectangleDraftImage();
	}
	if (editor.circle.draftActive && (editor.shapeFillMode != PaintEditor::ShapeFillMode::None || editor.shapeStripedStroke
		? (leftColorChanged || rightColorChanged)
		: ((leftColorChanged && !editor.circle.draftUsesRightColor) || (rightColorChanged && editor.circle.draftUsesRightColor))))
	{
		editor.refreshActiveCircleDraftImage();
	}
	if (editor.line.draftActive && ((leftColorChanged && !editor.line.draftUsesRightColor) || (rightColorChanged && editor.line.draftUsesRightColor))) editor.redrawLineDraft();
	if (editor.arrow.draftActive && ((leftColorChanged && !editor.arrow.draftUsesRightColor) || (rightColorChanged && editor.arrow.draftUsesRightColor))) editor.redrawArrowDraft();
	if (!editor.bezier.controlPoints.isEmpty() && ((leftColorChanged && !editor.bezier.usesRightColor) || (rightColorChanged && editor.bezier.usesRightColor))) editor.redrawBezierDraft();
}

/// Third column: primary/secondary, pipette, persisted favorite swatches (Tools and Layers occupy the first two columns).
static void drawPaintColorsPanel(PaintEditor &editor, float panelWidth, float menuBarH, const ImVec2 &sidePanelSize)
{
	if (!editor.showColorsPanel) return;

	ImGui::SetNextWindowPos(ImVec2(panelWidth * 2.f, menuBarH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(sidePanelSize, ImGuiCond_FirstUseEver);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.15f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
	if (ImGui::Begin("Colors", &editor.showColorsPanel, ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		if (editor.canvas.get().paletteMode)
		{
			const ImGuiColorEditFlags pickerFlags =
				ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoLabel;
			ImGui::SeparatorText("Palette mode");
			bool dith = editor.canvas.get().paletteDither;
			if (ImGui::Checkbox("Dither when quantizing", &dith))
			{
				editor.canvas.get().paletteDither = dith;
				editor.submitCanvas();
			}
			ImGui::TextDisabled("Left-click swatch: primary · Right-click: secondary · Drag to reorder · Context: edit/remove");
			const PaintDocument &doc = editor.canvas.get();
			const int32_t nPal = (int32_t)doc.paletteColors.getLength();
			const int32_t priIdx = nPal > 0 ? bbe::Math::clamp(doc.palettePrimaryIndex, 0, nPal - 1) : 0;
			const int32_t secIdx = nPal > 0 ? bbe::Math::clamp(doc.paletteSecondaryIndex, 0, nPal - 1) : 0;
			ImGui::SeparatorText("Active colors");
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
			const float pipetteBtn = std::max(1.f, std::floor(26.f * editor.viewport.scale));
			const ImVec2 swatchSize(pipetteBtn, pipetteBtn);
			if (nPal > 0)
			{
				{
					const bbe::Colori &c = doc.paletteColors[(size_t)priIdx];
					const ImVec4 cv(c.r / 255.f, c.g / 255.f, c.b / 255.f, 1.f);
					ImGui::ColorButton("Primary##palact", cv, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, swatchSize);
					ImGui::SameLine();
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Primary (index %d)", (int)priIdx);
				}
				{
					const bbe::Colori &c = doc.paletteColors[(size_t)secIdx];
					const ImVec4 cv(c.r / 255.f, c.g / 255.f, c.b / 255.f, 1.f);
					ImGui::ColorButton("Secondary##palact", cv, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, swatchSize);
					ImGui::SameLine();
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Secondary (index %d)", (int)secIdx);
				}
			}
			else
			{
				ImGui::TextDisabled("(empty palette)");
			}
			ImGui::PopStyleVar();

			ImGui::SeparatorText("Palette");
			static float s_addPick[3] = { 1.f, 1.f, 1.f };
			if (ImGui::Button("Add color…"))
			{
				s_addPick[0] = s_addPick[1] = s_addPick[2] = 1.f;
				ImGui::OpenPopup("##paletteAddPick");
			}
			if (ImGui::BeginPopup("##paletteAddPick"))
			{
				if (ImGui::ColorPicker3("##addPicker", s_addPick, pickerFlags))
				{
				}
				if (ImGui::Button("Add"))
				{
					const uint8_t r = (uint8_t)bbe::Math::clamp((int32_t)std::lround(s_addPick[0] * 255.f), 0, 255);
					const uint8_t g = (uint8_t)bbe::Math::clamp((int32_t)std::lround(s_addPick[1] * 255.f), 0, 255);
					const uint8_t b = (uint8_t)bbe::Math::clamp((int32_t)std::lround(s_addPick[2] * 255.f), 0, 255);
					editor.paletteAddUniqueColor(r, g, b);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			static float s_editPick[3] = { 1.f, 1.f, 1.f };
			static int s_editIdx = -1;
			static bool s_editOpenRequest = false;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 2.f));
			const float cell = std::max(24.f, std::floor(28.f * editor.viewport.scale));
			const ImVec2 cellSz(cell, cell);
			const int kPalCols = 4;
			for (size_t i = 0; i < doc.paletteColors.getLength(); i++)
			{
				if (i > 0 && (i % (size_t)kPalCols) != 0) ImGui::SameLine();
				ImGui::PushID((int)(i + 90000));
				const bbe::Colori &c = doc.paletteColors[i];
				const ImVec4 cv(c.r / 255.f, c.g / 255.f, c.b / 255.f, 1.f);
				ImGui::ColorButton("##sw", cv, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, cellSz);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) editor.setPalettePrimaryIndex((int32_t)i);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) editor.setPaletteSecondaryIndex((int32_t)i);

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					int payload = (int)i;
					ImGui::SetDragDropPayload("PALETTE_ROW", &payload, sizeof(int));
					ImGui::ColorButton("##drag", cv, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, cellSz);
					ImGui::EndDragDropSource();
				}
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("PALETTE_ROW"))
					{
						if (payload->DataSize == sizeof(int))
						{
							const int from = *(const int *)payload->Data;
							if (from >= 0 && (size_t)from < doc.paletteColors.getLength() && (int)i != from)
							{
								editor.paletteMoveEntryBefore((size_t)from, i);
							}
						}
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::BeginPopupContextItem("palctx"))
				{
					if (ImGui::MenuItem("Edit color…"))
					{
						s_editIdx = (int)i;
						s_editPick[0] = c.r / 255.f;
						s_editPick[1] = c.g / 255.f;
						s_editPick[2] = c.b / 255.f;
						s_editOpenRequest = true;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Remove"))
					{
						editor.paletteTryRemoveColorAtIndex(i);
					}
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}
			ImGui::PopStyleVar();
			if (s_editOpenRequest)
			{
				ImGui::OpenPopup("Edit palette entry");
				s_editOpenRequest = false;
			}
			ImVec2 edCenter = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(edCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Edit palette entry", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (s_editIdx < 0 || (size_t)s_editIdx >= editor.canvas.get().paletteColors.getLength())
				{
					ImGui::CloseCurrentPopup();
				}
				else
				{
					ImGui::ColorPicker3("##editPicker", s_editPick, pickerFlags);
					if (ImGui::Button("Apply", ImVec2(120, 0)))
					{
						const uint8_t r = (uint8_t)bbe::Math::clamp((int32_t)std::lround(s_editPick[0] * 255.f), 0, 255);
						const uint8_t g = (uint8_t)bbe::Math::clamp((int32_t)std::lround(s_editPick[1] * 255.f), 0, 255);
						const uint8_t b = (uint8_t)bbe::Math::clamp((int32_t)std::lround(s_editPick[2] * 255.f), 0, 255);
						editor.paletteReplaceColorAtIndex((size_t)s_editIdx, r, g, b);
						s_editIdx = -1;
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
					{
						s_editIdx = -1;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
			ImGui::End();
			ImGui::PopStyleColor(2);
			return;
		}

		// Do not set DisplayRGB alone: ImGui only shows HSV + hex rows when DisplayMask is 0 or all of RGB/HSV/Hex are set.
		const ImGuiColorEditFlags pickerFlags =
			ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_Uint8;
		ImGui::SeparatorText("Primary / secondary");
		const float colorGap = ImGui::GetStyle().ItemSpacing.x;
		const float pipetteBtn = std::max(1.f, std::floor(26.f * editor.viewport.scale));
		const ImVec2 swatchSize(pipetteBtn, pipetteBtn);
		// Match favorite swatches: ColorButton fills the rect; zero frame padding (ColorEdit4 used large padding before).
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
		ImGui::BeginGroup();
		const ImVec4 leftVec(editor.leftColor[0], editor.leftColor[1], editor.leftColor[2], editor.leftColor[3]);
		const ImVec4 rightVec(editor.rightColor[0], editor.rightColor[1], editor.rightColor[2], editor.rightColor[3]);
		bool leftColorChanged = false;
		bool rightColorChanged = false;
		if (ImGui::ColorButton("##primaryColor", leftVec, ImGuiColorEditFlags_AlphaPreviewHalf, swatchSize))
			ImGui::OpenPopup("##primaryPick");
		ImGui::SameLine(0.f, colorGap);
		if (ImGui::ColorButton("##secondaryColor", rightVec, ImGuiColorEditFlags_AlphaPreviewHalf, swatchSize))
			ImGui::OpenPopup("##secondaryPick");
		ImGui::SameLine(0.f, colorGap);
		{
			const bool pipActive = editor.mode == PaintEditor::MODE_PIPETTE;
			if (pipActive) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			bool pipClicked = false;
#ifdef BBE_RENDERER_OPENGL
			if (s_toolIcons.pipette.texId)
			{
				pipClicked = ImGui::ImageButton("Pipette##colorsPipette", s_toolIcons.pipette.texId, ImVec2(pipetteBtn, pipetteBtn));
			}
			else
#endif
			{
				pipClicked = ImGui::Button("Pick##colorsPipette", ImVec2(pipetteBtn, pipetteBtn));
			}
			if (pipActive) ImGui::PopStyleColor();
			const PaintEditor::DigitHotkeyAction pipBind = PaintEditor::digitHotkeyActionForToolMode(PaintEditor::MODE_PIPETTE);
			paintEditorTryBindDigitHotkeyOnHover(editor, pipBind);
			if (pipClicked) editor.mode = PaintEditor::MODE_PIPETTE;
			if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, pipBind, "Pipette — click canvas to sample a color");
		}
		ImGui::EndGroup();
		if (ImGui::BeginPopup("##primaryPick"))
		{
			if (ImGui::ColorPicker4("##primaryPicker", editor.leftColor, pickerFlags)) leftColorChanged = true;
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("##secondaryPick"))
		{
			if (ImGui::ColorPicker4("##secondaryPicker", editor.rightColor, pickerFlags)) rightColorChanged = true;
			ImGui::EndPopup();
		}
		paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, leftColorChanged, rightColorChanged);

		ImGui::SeparatorText("Recent");
		ImGui::TextDisabled("Click to edit. Shift+left: primary · Shift+right: secondary · Right-click: menu.");
		constexpr int kHistCols = 4;
		const ImVec2 histCellSize = swatchSize;
		for (size_t i = 0; i < PaintEditor::colorHistoryCapacity; i++)
		{
			ImGui::PushID(static_cast<int>(i + 30000));
			const ImVec4 hVec(editor.colorHistoryRgba[i][0], editor.colorHistoryRgba[i][1], editor.colorHistoryRgba[i][2], editor.colorHistoryRgba[i][3]);
			const bool shiftHeld = ImGui::GetIO().KeyShift;
			const bool histPressed = ImGui::ColorButton("##hist", hVec, ImGuiColorEditFlags_AlphaPreviewHalf, histCellSize);
			const ImGuiHoveredFlags hh = ImGuiHoveredFlags_AllowWhenBlockedByPopup;
			if (histPressed && shiftHeld)
			{
				std::memcpy(editor.leftColor, editor.colorHistoryRgba[i], sizeof(editor.leftColor));
				paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, true, false);
			}
			else if (histPressed && !shiftHeld)
			{
				ImGui::OpenPopup("##histpick");
			}
			if (ImGui::IsItemHovered(hh) && shiftHeld && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				std::memcpy(editor.rightColor, editor.colorHistoryRgba[i], sizeof(editor.rightColor));
				paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, false, true);
			}
			else if (ImGui::IsItemHovered(hh) && !shiftHeld && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("hist_ctx");
			}
			if (ImGui::BeginPopup("##histpick"))
			{
				if (ImGui::ColorPicker4("##histpicker", editor.colorHistoryRgba[i], pickerFlags) && editor.onColorHistoryChanged) editor.onColorHistoryChanged();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopup("hist_ctx"))
			{
				bool usedPri = false;
				bool usedSec = false;
				if (ImGui::MenuItem("Use as primary"))
				{
					std::memcpy(editor.leftColor, editor.colorHistoryRgba[i], sizeof(editor.leftColor));
					usedPri = true;
				}
				if (ImGui::MenuItem("Use as secondary"))
				{
					std::memcpy(editor.rightColor, editor.colorHistoryRgba[i], sizeof(editor.rightColor));
					usedSec = true;
				}
				ImGui::EndPopup();
				paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, usedPri, usedSec);
			}
			ImGui::PopID();
			if ((i + 1) % (size_t)kHistCols != 0 && i + 1 < PaintEditor::colorHistoryCapacity) ImGui::SameLine();
		}

		ImGui::SeparatorText("Most used");
		editor.ensureMostUsedOnCanvasColorsUpdated();
		ImGui::TextDisabled("Top colors on the flattened visible canvas (alpha = 0 ignored). Large images are sampled.");
		if (editor.mostUsedOnCanvasCount == 0) ImGui::TextDisabled("No opaque pixels.");
		else
		{
			ImGui::TextDisabled("Shift+left: primary · Shift+right: secondary · Right-click: menu.");
			constexpr int kMostCols = 4;
			const ImVec2 mostCellSize = swatchSize;
			for (size_t i = 0; i < editor.mostUsedOnCanvasCount; i++)
			{
				ImGui::PushID(static_cast<int>(i + 50000));
				const ImVec4 mVec(editor.mostUsedOnCanvasRgba[i][0], editor.mostUsedOnCanvasRgba[i][1], editor.mostUsedOnCanvasRgba[i][2], editor.mostUsedOnCanvasRgba[i][3]);
				const bool shiftHeld = ImGui::GetIO().KeyShift;
				const bool mostPressed = ImGui::ColorButton("##mostused", mVec, ImGuiColorEditFlags_AlphaPreviewHalf, mostCellSize);
				const ImGuiHoveredFlags hm = ImGuiHoveredFlags_AllowWhenBlockedByPopup;
				if (mostPressed && shiftHeld)
				{
					std::memcpy(editor.leftColor, editor.mostUsedOnCanvasRgba[i], sizeof(editor.leftColor));
					paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, true, false);
				}
				if (ImGui::IsItemHovered(hm) && shiftHeld && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					std::memcpy(editor.rightColor, editor.mostUsedOnCanvasRgba[i], sizeof(editor.rightColor));
					paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, false, true);
				}
				else if (ImGui::IsItemHovered(hm) && !shiftHeld && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("mostused_ctx");
				}
				if (ImGui::BeginPopup("mostused_ctx"))
				{
					bool usedPri = false;
					bool usedSec = false;
					if (ImGui::MenuItem("Use as primary"))
					{
						std::memcpy(editor.leftColor, editor.mostUsedOnCanvasRgba[i], sizeof(editor.leftColor));
						usedPri = true;
					}
					if (ImGui::MenuItem("Use as secondary"))
					{
						std::memcpy(editor.rightColor, editor.mostUsedOnCanvasRgba[i], sizeof(editor.rightColor));
						usedSec = true;
					}
					ImGui::EndPopup();
					paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, usedPri, usedSec);
				}
				ImGui::PopID();
				if ((i + 1) % (size_t)kMostCols != 0 && i + 1 < editor.mostUsedOnCanvasCount) ImGui::SameLine();
			}
		}

		ImGui::SeparatorText("Favorites");
		ImGui::TextDisabled("Click to edit. Shift+left: primary · Shift+right: secondary · Right-click: menu.");
		constexpr int kFavCols = 4;
		const ImVec2 favCellSize = swatchSize;
		for (size_t i = 0; i < PaintEditor::favoriteColorSlotCount; i++)
		{
			ImGui::PushID(static_cast<int>(i));
			const ImVec4 favVec(editor.favoriteColorRgba[i][0], editor.favoriteColorRgba[i][1], editor.favoriteColorRgba[i][2], editor.favoriteColorRgba[i][3]);
			const bool shiftHeld = ImGui::GetIO().KeyShift;
			const bool favPressed = ImGui::ColorButton("##fav", favVec, ImGuiColorEditFlags_AlphaPreviewHalf, favCellSize);
			const ImGuiHoveredFlags hfav = ImGuiHoveredFlags_AllowWhenBlockedByPopup;
			if (favPressed && shiftHeld)
			{
				std::memcpy(editor.leftColor, editor.favoriteColorRgba[i], sizeof(editor.leftColor));
				paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, true, false);
			}
			else if (favPressed && !shiftHeld)
			{
				ImGui::OpenPopup("##favpick");
			}
			if (ImGui::IsItemHovered(hfav) && shiftHeld && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				std::memcpy(editor.rightColor, editor.favoriteColorRgba[i], sizeof(editor.rightColor));
				paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, false, true);
			}
			else if (ImGui::IsItemHovered(hfav) && !shiftHeld && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("fav_ctx");
			}
			if (ImGui::BeginPopup("##favpick"))
			{
				if (ImGui::ColorPicker4("##picker", editor.favoriteColorRgba[i], pickerFlags) && editor.onFavoriteColorsChanged) editor.onFavoriteColorsChanged();
				ImGui::EndPopup();
			}
			if (ImGui::BeginPopup("fav_ctx"))
			{
				bool usedPri = false;
				bool usedSec = false;
				if (ImGui::MenuItem("Use as primary"))
				{
					std::memcpy(editor.leftColor, editor.favoriteColorRgba[i], sizeof(editor.leftColor));
					usedPri = true;
				}
				if (ImGui::MenuItem("Use as secondary"))
				{
					std::memcpy(editor.rightColor, editor.favoriteColorRgba[i], sizeof(editor.rightColor));
					usedSec = true;
				}
				if (ImGui::MenuItem("Store primary here"))
				{
					std::memcpy(editor.favoriteColorRgba[i], editor.leftColor, sizeof(editor.leftColor));
					if (editor.onFavoriteColorsChanged) editor.onFavoriteColorsChanged();
				}
				if (ImGui::MenuItem("Store secondary here"))
				{
					std::memcpy(editor.favoriteColorRgba[i], editor.rightColor, sizeof(editor.rightColor));
					if (editor.onFavoriteColorsChanged) editor.onFavoriteColorsChanged();
				}
				ImGui::EndPopup();
				paintEditorRefreshDraftsAfterPrimarySecondaryChange(editor, usedPri, usedSec);
			}
			ImGui::PopID();
			if ((i + 1) % (size_t)kFavCols != 0 && i + 1 < PaintEditor::favoriteColorSlotCount) ImGui::SameLine();
		}
		ImGui::PopStyleVar(); // FramePadding — primary, secondary, favorites ColorButtons
	}
	ImGui::End();
	ImGui::PopStyleColor(2);
}

/// \p defaultLeft is the initial X offset from the left (matches default Tools + Layers + Colors widths on first run).
static void drawPaintToolOptionsPanel(PaintEditor &editor, float defaultLeft)
{
	if (!editor.showToolOptionsPanel) return;

	const float scale = editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f;
	const float optW = 292.f * scale;
	const float menuH = ImGui::GetFrameHeight();
	const float vh = (float)editor.viewport.height - menuH;

	ImGui::SetNextWindowPos(ImVec2(defaultLeft, menuH), ImGuiCond_FirstUseEver);
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
			ImGui::Checkbox("Smart Fill", &editor.floodFillSmartFill);
			if (ImGui::SliderInt("Tolerance", &editor.floodFillTolerance, 0, 255))
			{
				editor.clampFloodFillTolerance();
			}
			ImGui::TextDisabled("Fills the active layer from the click; per-channel match vs seed color. [+ / -] nudges tolerance. Smart Fill extends one pixel past the strict boundary with high overflow tolerance (helps thin gaps).");
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
			{
				static const char *shapeFillNames[] = {
					"None",
					"Primary color",
					"Secondary color",
					"Checkerboard",
					"Horizontal stripes",
					"Vertical stripes",
					"Diagonal stripes (/)",
					"Diagonal stripes (\\)",
					"Dot grid",
					"Concentric rings",
					"Radial gradient",
					"Crosshatch mesh",
					"Brick wall",
					"Noise dither",
				};
				static_assert(IM_ARRAYSIZE(shapeFillNames) == static_cast<int>(PaintEditor::ShapeFillMode::COUNT), "shapeFillNames must match ShapeFillMode");
				int fillIdx = static_cast<int>(editor.shapeFillMode);
				if (fillIdx < 0 || fillIdx >= static_cast<int>(PaintEditor::ShapeFillMode::COUNT)) fillIdx = 0;
				if (ImGui::Combo("Fill", &fillIdx, shapeFillNames, static_cast<int>(PaintEditor::ShapeFillMode::COUNT)))
				{
					editor.shapeFillMode = static_cast<PaintEditor::ShapeFillMode>(fillIdx);
					if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
					if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
				}
				const bool showPatternScale = editor.shapeFillMode != PaintEditor::ShapeFillMode::None && editor.shapeFillMode != PaintEditor::ShapeFillMode::Primary && editor.shapeFillMode != PaintEditor::ShapeFillMode::Secondary && editor.shapeFillMode != PaintEditor::ShapeFillMode::RadialGradient;
				if (showPatternScale)
				{
					if (ImGui::InputInt("Pattern scale (px)", &editor.shapeFillPatternCellPx))
					{
						editor.clampShapeFillPatternCellPx();
						if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
						if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
					}
				}
				if (editor.shapeFillMode == PaintEditor::ShapeFillMode::NoiseDither)
				{
					if (ImGui::InputScalar("Noise seed", ImGuiDataType_U32, &editor.shapeFillNoiseSeed))
					{
						if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
						if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
					}
					if (ImGui::Button("Reroll##shapeNoiseSeed"))
					{
						editor.rerollShapeFillNoiseSeed();
						if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
						if (editor.circle.draftActive) editor.refreshActiveCircleDraftImage();
					}
				}
				ImGui::TextDisabled("Interior uses primary/secondary swatches; outline still follows the mouse button used to draw.");
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

static void drawPaintNavigatorImGuiWindow(PaintEditor &editor, uint64_t navigatorContentHash, const bbe::Color &canvasBackdrop)
{
	if (!editor.showNavigator || editor.getCanvasWidth() <= 0)
	{
		editor.navigatorMinimapHitRectValid = false;
		return;
	}

	const float canvasW = (float)editor.getCanvasWidth();
	const float canvasH = (float)editor.getCanvasHeight();
	const float navMaxSize = 160.f * editor.viewport.scale;
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
	const int tw = std::max(1, (int)std::floor(navW));
	const int th = std::max(1, (int)std::floor(navH));

	ImGui::SetNextWindowPos(ImVec2((float)editor.viewport.width - navW - 16.f, (float)editor.viewport.height - th - 48.f), ImGuiCond_FirstUseEver);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.15f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
	if (!ImGui::Begin("Navigator", &editor.showNavigator))
	{
		ImGui::PopStyleColor(2);
		ImGui::End();
		editor.navigatorMinimapHitRectValid = false;
		return;
	}

	{

	const uint64_t thumbPixelHash = paintEditorNavigatorThumbPixelHash(navigatorContentHash, tw, th);
	if (thumbPixelHash != editor.navigatorThumbPixelHashStored
		|| tw != s_navigatorThumbCacheW || th != s_navigatorThumbCacheH
		|| s_navigatorThumbRgbaCache.size() != (size_t)tw * th * 4)
	{
		// Sample visible pixels at minimap resolution only (O(tw*th)), not full-canvas flatten + downscale (O(W*H)).
		const int32_t W = editor.getCanvasWidth();
		const int32_t H = editor.getCanvasHeight();
		s_navigatorThumbRgbaCache.resize((size_t)tw * th * 4);
		{
			for (int y = 0; y < th; y++)
			{
				for (int x = 0; x < tw; x++)
				{
					int32_t sx = (int32_t)((int64_t)x * W / tw);
					int32_t sy = (int32_t)((int64_t)y * H / th);
					if (sx >= W) sx = W - 1;
					if (sy >= H) sy = H - 1;
					bbe::Colori c = editor.getVisiblePixel((size_t)sx, (size_t)sy);
					const bbe::Colori wa = editor.getWorkAreaPixelAtCanvas(sx, sy);
					c = c.blendTo(wa, 1.f, bbe::BlendMode::Normal);
					const size_t i = ((size_t)y * tw + x) * 4;
					s_navigatorThumbRgbaCache[i + 0] = c.r;
					s_navigatorThumbRgbaCache[i + 1] = c.g;
					s_navigatorThumbRgbaCache[i + 2] = c.b;
					s_navigatorThumbRgbaCache[i + 3] = c.a;
				}
			}
		}
		s_navigatorThumbCacheW = tw;
		s_navigatorThumbCacheH = th;
		editor.navigatorThumbPixelHashStored = thumbPixelHash;
#ifdef BBE_RENDERER_OPENGL
		{
			navigatorUploadRgbaTexture(tw, th, s_navigatorThumbRgbaCache.data());
		}
#endif
	}

	{
	const ImVec2 imgSize((float)tw, (float)th);
#ifdef BBE_RENDERER_OPENGL
	ImGui::Image((ImTextureID)(intptr_t)s_navigatorMinimapGlTex, imgSize);
#else
	ImGui::InvisibleButton("##navigatorMap", imgSize);
	{
		const ImVec2 p0 = ImGui::GetItemRectMin();
		const ImVec2 p1 = ImGui::GetItemRectMax();
		ImDrawList *dl = ImGui::GetWindowDrawList();
		const auto u8 = [](float f) -> int { return (int)bbe::Math::clamp(f * 255.f, 0.f, 255.f); };
		const ImU32 backdropCol = IM_COL32(u8(canvasBackdrop.r), u8(canvasBackdrop.g), u8(canvasBackdrop.b), u8(canvasBackdrop.a));
		dl->AddRectFilled(p0, p1, backdropCol);
		for (int y = 0; y < th; y++)
		{
			for (int x = 0; x < tw; x++)
			{
				const size_t i = ((size_t)y * tw + x) * 4;
				const unsigned char r = s_navigatorThumbRgbaCache[i + 0];
				const unsigned char g = s_navigatorThumbRgbaCache[i + 1];
				const unsigned char b = s_navigatorThumbRgbaCache[i + 2];
				const unsigned char a = s_navigatorThumbRgbaCache[i + 3];
				if (a == 0) continue;
				const ImVec2 a0(p0.x + x, p0.y + y);
				const ImVec2 b0(p0.x + x + 1, p0.y + y + 1);
				dl->AddRectFilled(a0, b0, IM_COL32(r, g, b, a));
			}
		}
	}
#endif

	const ImVec2 rmin = ImGui::GetItemRectMin();
	const ImVec2 rmax = ImGui::GetItemRectMax();
	const float dispW = rmax.x - rmin.x;
	const float dispH = rmax.y - rmin.y;
	editor.navigatorMinimapHitRect = bbe::Rectangle(rmin.x, rmin.y, dispW, dispH);
	editor.navigatorMinimapHitRectValid = dispW > 0.f && dispH > 0.f;

	const float scaleX = dispW / canvasW;
	const float scaleY = dispH / canvasH;
	const bbe::Vector2 tlCanvas = editor.screenToCanvas({ 0.f, 0.f });
	const bbe::Vector2 brCanvas = editor.screenToCanvas({ (float)editor.viewport.width, (float)editor.viewport.height });
	const float vx1 = bbe::Math::clamp(rmin.x + tlCanvas.x * scaleX, rmin.x, rmax.x);
	const float vy1 = bbe::Math::clamp(rmin.y + tlCanvas.y * scaleY, rmin.y, rmax.y);
	const float vx2 = bbe::Math::clamp(rmin.x + brCanvas.x * scaleX, rmin.x, rmax.x);
	const float vy2 = bbe::Math::clamp(rmin.y + brCanvas.y * scaleY, rmin.y, rmax.y);
	ImGui::GetWindowDrawList()->AddRect(ImVec2(vx1, vy1), ImVec2(vx2, vy2), IM_COL32(0, 255, 0, 255), 0.f, 0, 1.f);

	if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		const ImVec2 mp = ImGui::GetIO().MousePos;
		const float canvasClickX = (mp.x - rmin.x) / dispW * canvasW;
		const float canvasClickY = (mp.y - rmin.y) / dispH * canvasH;
		editor.offset.x = (float)editor.viewport.width * 0.5f - canvasClickX * editor.zoomLevel;
		editor.offset.y = (float)editor.viewport.height * 0.5f - canvasClickY * editor.zoomLevel;
	}

	}

	}

	ImGui::End();
	ImGui::PopStyleColor(2);
}

void drawExamplePaintGui(PaintEditor &editor, bbe::PrimitiveBrush2D &brush, const bbe::Vector2 &mouseScreenPos)
{
#ifdef BBE_RENDERER_OPENGL
		s_toolIcons.refresh();
#endif
		bool anyNonNormalBlendMode = false;
		bbe::Color canvasBackdrop(1.f, 1.f, 1.f, 1.f);
		uint64_t navigatorContentHash = 0;

		{
		// Host dock space for Tools / Layers / Colors / Tool Options; central node is passthrough so the
		// canvas (drawn under ImGui) still receives mouse via io.WantCaptureMouse when not over a panel.
		ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

		editor.navigatorMinimapHitRectValid = false;

		const float PANEL_WIDTH = 236.f * editor.viewport.scale;
		const float menuBarH = ImGui::GetFrameHeight();
		const float workH = (float)editor.viewport.height - menuBarH;
		const ImVec2 sidePanelSize(PANEL_WIDTH, std::max(200.f, workH));

		ImGui::SetNextWindowPos(ImVec2(0, menuBarH), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(sidePanelSize, ImGuiCond_FirstUseEver);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
		ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

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
		{
			paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::Undo);
			if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::Undo, "Undo");
		}
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
		{
			paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::Redo);
			if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::Redo, "Redo");
		}
		ImGui::EndDisabled();

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
				{ "Brush",     PaintEditor::MODE_BRUSH,             ImTextureID_Invalid },
				{ "Eraser",    PaintEditor::MODE_ERASER,          ImTextureID_Invalid },
				{ "Rectangle", PaintEditor::MODE_RECTANGLE,       ImTextureID_Invalid },
				{ "Fill",      PaintEditor::MODE_FLOOD_FILL,      ImTextureID_Invalid },
				{ "Circle",    PaintEditor::MODE_CIRCLE,          ImTextureID_Invalid },
				{ "Text",      PaintEditor::MODE_TEXT,            ImTextureID_Invalid },
				{ "Line",      PaintEditor::MODE_LINE,            ImTextureID_Invalid },
				{ "Arrow",     PaintEditor::MODE_ARROW,           ImTextureID_Invalid },
				{ "Spray",     PaintEditor::MODE_SPRAY,           ImTextureID_Invalid },
				{ "Bezier",    PaintEditor::MODE_BEZIER,          ImTextureID_Invalid },
			};
			const ToolBtn selectionTools[] = {
				{ "Selection",   PaintEditor::MODE_SELECTION,         ImTextureID_Invalid },
				{ "Ellipse Sel", PaintEditor::MODE_ELLIPSE_SELECTION, ImTextureID_Invalid },
				{ "Lasso",       PaintEditor::MODE_LASSO,             ImTextureID_Invalid },
				{ "Wand",        PaintEditor::MODE_MAGIC_WAND,        ImTextureID_Invalid },
				{ "Poly Lasso",  PaintEditor::MODE_POLYGON_LASSO,     ImTextureID_Invalid },
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
					const PaintEditor::DigitHotkeyAction toolBind = PaintEditor::digitHotkeyActionForToolMode(tb.toolMode);
					paintEditorTryBindDigitHotkeyOnHover(editor, toolBind);
					if (clicked) editor.mode = tb.toolMode;
					if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, toolBind, tb.label);
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
				ImTextureID symTex = ImTextureID_Invalid;
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
				{
					const PaintEditor::DigitHotkeyAction symBind = PaintEditor::digitHotkeyActionForSymmetry(symModes[i].mode);
					paintEditorTryBindDigitHotkeyOnHover(editor, symBind);
					if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, symBind, symModes[i].tip);
				}
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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::SelectionCopy);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::SelectionCopy, "Copy selection to internal clipboard");
			}
			if (ImGui::Button("Cut",    ImVec2(-1, 0))) editor.cutSelection();
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::SelectionCut);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::SelectionCut, "Cut selection");
			}
			if (ImGui::Button("Delete", ImVec2(-1, 0))) editor.deleteSelection();
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::SelectionDelete);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::SelectionDelete, "Delete selection contents");
			}
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
				editor.copyFlattenedCanvasToClipboard();
		}
		else
#endif
		{
			if (ImGui::Button("Copy Canvas to Clipboard", ImVec2(-1, 0)))
				editor.copyFlattenedCanvasToClipboard();
		}
		{
			paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::ClipboardCopyCanvas);
			if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::ClipboardCopyCanvas, "Copy flattened visible canvas to the system clipboard as an image");
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!supportsClipboardImages || !clipboardHasImage || !editor.platform.getClipboardImage);
#ifdef BBE_RENDERER_OPENGL
		if (clipboardUseIconRow)
		{
			ImGui::SameLine();
			if (ImGui::ImageButton("##clipPasteNew", s_toolIcons.clipboardPasteNew.texId, ImVec2(clipIconSize, clipIconSize)))
				editor.requestReplaceDocumentPasteFromClipboard();
		}
		else
#endif
		{
			if (ImGui::Button("Paste as New Canvas", ImVec2(-1, 0)))
				editor.requestReplaceDocumentPasteFromClipboard();
		}
		{
			paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::ClipboardPasteNew);
			if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::ClipboardPasteNew, "Replace the document with the image from the clipboard (new single layer)");
		}
		ImGui::EndDisabled();
		if (!supportsClipboardImages)
			ImGui::TextDisabled("Not supported on this platform");

		ImGui::End();
		ImGui::PopStyleColor(2);

		ImGui::SetNextWindowPos(ImVec2(PANEL_WIDTH, menuBarH), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(sidePanelSize, ImGuiCond_FirstUseEver);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.15f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
		ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		const bool paletteLocksLayers = editor.canvas.get().paletteMode;
		if (paletteLocksLayers) ImGui::BeginDisabled(true);

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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::LayerNew);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::LayerNew, "New layer");
			}
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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::LayerDelete);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::LayerDelete, "Delete active layer");
			}
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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::LayerMoveUp);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::LayerMoveUp, "Move layer up (toward front)");
			}
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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::LayerMoveDown);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::LayerMoveDown, "Move layer down (toward back)");
			}
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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::LayerDuplicate);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::LayerDuplicate, "Duplicate active layer");
			}
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
			{
				paintEditorTryBindDigitHotkeyOnHover(editor, PaintEditor::DigitHotkeyAction::LayerMergeDown);
				if (ImGui::IsItemHovered()) paintEditorDigitHotkeyTooltip(editor, PaintEditor::DigitHotkeyAction::LayerMergeDown, "Merge active layer into the one below");
			}
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

		if (paletteLocksLayers) ImGui::EndDisabled();
		if (paletteLocksLayers) ImGui::TextDisabled("Palette Mode: single layer only.");

		ImGui::End();
		ImGui::PopStyleColor(2);

		drawPaintColorsPanel(editor, PANEL_WIDTH, menuBarH, sidePanelSize);

		drawPaintToolOptionsPanel(editor, PANEL_WIDTH * 3.f);

		}

		{
		anyNonNormalBlendMode = false;
		for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
		{
			const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
			if (layer.visible && layer.blendMode != bbe::BlendMode::Normal)
			{
				anyNonNormalBlendMode = true;
				break;
			}
		}
		navigatorContentHash = paintEditorNavigatorContentHash(editor, anyNonNormalBlendMode);
		if (navigatorContentHash != editor.navigatorContentHashStored)
		{
			if (anyNonNormalBlendMode)
			{
				editor.navigatorCachedFlattenVisible = editor.flattenVisibleLayers();
			}
			editor.navigatorContentHashStored = navigatorContentHash;
		}

		canvasBackdrop = bbe::Color(
			editor.canvas.get().canvasFallbackRgba[0],
			editor.canvas.get().canvasFallbackRgba[1],
			editor.canvas.get().canvasFallbackRgba[2],
			editor.canvas.get().canvasFallbackRgba[3]);

		}
		drawPaintNavigatorImGuiWindow(editor, navigatorContentHash, canvasBackdrop);

		{
			int mainTiMin = 0, mainTiMax = 0, mainTkMin = 0, mainTkMax = 0;
			paintEditorVisibleMainTileRange(editor, mainTiMin, mainTiMax, mainTkMin, mainTkMax);
		{
		for (int32_t i = mainTiMin; i <= mainTiMax; i++)
		{
			for (int32_t k = mainTkMin; k <= mainTkMax; k++)
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
					brush.drawImage(tileX, tileY, tileW, tileH, editor.navigatorCachedFlattenVisible);
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
				if (editor.workArea.getWidth() > 0 && editor.workArea.getHeight() > 0)
				{
					const float waX = tileX + (float)editor.workAreaCanvasOrigin.x * editor.zoomLevel;
					const float waY = tileY + (float)editor.workAreaCanvasOrigin.y * editor.zoomLevel;
					const float waW = (float)editor.workArea.getWidth() * editor.zoomLevel;
					const float waH = (float)editor.workArea.getHeight() * editor.zoomLevel;
					brush.drawImage(waX, waY, waW, waH, editor.workArea);
				}
			}
		}
		}
		{
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
		}
		{
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
		}

		{
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
				paintEditorQuantizeImageCopyIfPaletteMode(editor, rotatedImg);
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
			const bbe::Rectanglei ghostBounds = paintEditorUnionRecti(rect, displayRect);
			int gTiMin = 0, gTiMax = 0, gTkMin = 0, gTkMax = 0;
			paintEditorVisibleGhostTileRangeForCanvasRect(editor, ghostBounds, gTiMin, gTiMax, gTkMin, gTkMax);
			for (int32_t i = gTiMin; i <= gTiMax; i++)
			{
				for (int32_t k = gTkMin; k <= gTkMax; k++)
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
			bbe::Image rectPv = editor.rectangle.dragPreviewImage;
			paintEditorQuantizeImageCopyIfPaletteMode(editor, rectPv);
			drawInAllTiles(editor.rectangle.dragPreviewRect, rectPv);
		}
		else if (editor.circle.dragActive && editor.circle.dragPreviewRect.width > 0 && editor.circle.dragPreviewRect.height > 0)
		{
			bbe::Image circPv = editor.circle.dragPreviewImage;
			paintEditorQuantizeImageCopyIfPaletteMode(editor, circPv);
			drawInAllTiles(editor.circle.dragPreviewRect, circPv);
		}
		else if (editor.selection.moveActive || editor.selection.resizeActive)
		{
			// Palette mode: floating/preview pixels are quantized when the selection is created or transformed (see quantizeFloatingSelectionImagesIfPaletteMode).
			// Do not re-quantize every GUI frame here — full-image passes (especially with dither) made moves unusably slow and could desync preview vs. commit.
			if (editor.canvas.get().paletteMode && !editor.canvas.get().paletteColors.isEmpty())
			{
				const bbe::Image *src = nullptr;
				if (editor.selection.resizeActive)
				{
					editor.refreshPaletteModeSelectionResizeDrawCacheIfStale();
					src = &editor.selection.paletteResizeDrawCache;
				}
				else
				{
					src = &editor.selection.previewImage;
				}
				if (src->getWidth() > 0 && src->getHeight() > 0)
					drawInAllTiles(editor.selection.previewRect, *src, editor.selection.rotation);
			}
			else
			{
				bbe::Image previewImage = editor.selection.resizeActive ? editor.buildSelectionPreviewResultImage() : editor.selection.previewImage;
				paintEditorQuantizeImageCopyIfPaletteMode(editor, previewImage);
				drawInAllTiles(editor.selection.previewRect, previewImage, editor.selection.rotation);
			}
		}
		else if (editor.selection.floating)
		{
			if (!editor.antiAliasingEnabled && std::abs(editor.selection.rotation) > 0.01f && (editor.rectangle.draftActive || editor.circle.draftActive))
			{
				// AA-off + rotation: re-render from SDF so preview matches the committed result.
				const bbe::Colori color = editor.rectangle.draftActive ? editor.getRectangleDraftColor() : editor.getCircleDraftColor();
				bbe::Image img = editor.rectangle.draftActive
					? editor.createRectangleImage(editor.selection.rect.width, editor.selection.rect.height, color, editor.selection.rotation)
					: editor.createCircleImage(editor.selection.rect.width, editor.selection.rect.height, color, editor.selection.rotation);
				paintEditorQuantizeImageCopyIfPaletteMode(editor, img);
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
				int elTiMin = 0, elTiMax = 0, elTkMin = 0, elTkMax = 0;
				paintEditorVisibleGhostTileRangeForCanvasRect(editor, r, elTiMin, elTiMax, elTkMin, elTkMax);
				for (int32_t ti = elTiMin; ti <= elTiMax; ti++)
				{
					for (int32_t tk = elTkMin; tk <= elTkMax; tk++)
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
			const int32_t linePad = (int32_t)std::ceil(std::max(lwOuter, lwInner)) + 2;
			const bbe::Rectanglei lassoBounds = paintEditorBoundsOfLassoPath(editor, linePad);
			int lsTiMin = 0, lsTiMax = 0, lsTkMin = 0, lsTkMax = 0;
			paintEditorVisibleGhostTileRangeForCanvasRect(editor, lassoBounds, lsTiMin, lsTiMax, lsTkMin, lsTkMax);
			for (int32_t ti = lsTiMin; ti <= lsTiMax; ti++)
			{
				for (int32_t tk = lsTkMin; tk <= lsTkMax; tk++)
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
			const int32_t polyPad = (int32_t)std::ceil(std::max(lwOuter, lwInner)) + 4;
			const bbe::Rectanglei polyBounds = paintEditorBoundsOfPolygonLasso(editor, polyPad);
			int pgTiMin = 0, pgTiMax = 0, pgTkMin = 0, pgTkMax = 0;
			paintEditorVisibleGhostTileRangeForCanvasRect(editor, polyBounds, pgTiMin, pgTiMax, pgTkMin, pgTkMax);
			for (int32_t ti = pgTiMin; ti <= pgTiMax; ti++)
			{
				for (int32_t tk = pgTkMin; tk <= pgTkMax; tk++)
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
			int selTiMin = 0, selTiMax = 0, selTkMin = 0, selTkMax = 0;
			paintEditorVisibleGhostTileRangeForCanvasRect(editor, editor.selection.rect, selTiMin, selTiMax, selTkMin, selTkMax);
			for (int32_t ti = selTiMin; ti <= selTiMax; ti++)
			{
				for (int32_t tk = selTkMin; tk <= selTkMax; tk++)
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
		}

		{
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
				const float z = editor.zoomLevel;
				constexpr float line = 1.f;
				brush.setColorRGB(1.f, 0.35f, 0.35f, 0.9f);
				auto drawEraserStampScreenOutline = [&](const bbe::Rectanglei &stamp)
				{
					int erTiMin = 0, erTiMax = 0, erTkMin = 0, erTkMax = 0;
					paintEditorVisibleGhostTileRangeForCanvasRect(editor, stamp, erTiMin, erTiMax, erTkMin, erTkMax);
					for (int32_t ti = erTiMin; ti <= erTiMax; ti++)
					{
						for (int32_t tk = erTkMin; tk <= erTkMax; tk++)
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
				const float z = editor.zoomLevel;
				const float rad = (float)editor.sprayWidth * z;
				const bbe::List<bbe::Vector2> centers = editor.getSymmetryPositions(previewCenter);
				for (size_t si = 0; si < centers.getLength(); si++)
				{
					const int32_t sw = editor.sprayWidth;
					const int32_t pad = 4;
					const int32_t ext = sw + pad;
					const bbe::Rectanglei sprayCanvasBounds(
						(int32_t)std::floor(centers[si].x) - ext,
						(int32_t)std::floor(centers[si].y) - ext,
						std::max(1, ext * 2 + 1),
						std::max(1, ext * 2 + 1));
					int spTiMin = 0, spTiMax = 0, spTkMin = 0, spTkMax = 0;
					paintEditorVisibleGhostTileRangeForCanvasRect(editor, sprayCanvasBounds, spTiMin, spTiMax, spTkMin, spTkMax);
					for (int32_t ti = spTiMin; ti <= spTiMax; ti++)
					{
						for (int32_t tk = spTkMin; tk <= spTkMax; tk++)
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
		}

		{
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
		}

		{
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
		}

		}

		{
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
						editor.requestReplaceDocumentOpen(newPath);
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
				if (ImGui::MenuItem("Quit"))
				{
					editor.requestReplaceApplicationQuit();
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
				ImGui::Separator();
				const bool palOn = editor.canvas.get().paletteMode;
				if (ImGui::MenuItem("Palette Mode", nullptr, palOn))
				{
					if (!palOn) editor.preparePaletteModeSetupDialog();
					else editor.disablePaletteMode();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				auto toggleMenuItem = [&](const char *label, bool &value) { if (ImGui::MenuItem(label, nullptr, value)) value = !value; };
				toggleMenuItem("Draw Grid Lines", editor.drawGridLines);
				toggleMenuItem("Tiled", editor.tiled);
				toggleMenuItem("Navigator", editor.showNavigator);
				toggleMenuItem("Colors", editor.showColorsPanel);
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
		if (editor.openClipboardWriteFailedPopup)
		{
			ImGui::OpenPopup("Clipboard copy failed");
			editor.openClipboardWriteFailedPopup = false;
		}
		if (editor.openDropChoicePopup)
		{
			ImGui::OpenPopup("Dropped File(s)");
			editor.openDropChoicePopup = false;
		}
		if (editor.openUnsavedChangesPopup)
		{
			ImGui::OpenPopup("Unsaved changes");
			editor.openUnsavedChangesPopup = false;
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
		if (editor.paletteModeSetupOpenRequest)
		{
			ImGui::OpenPopup("Palette mode setup");
			editor.paletteModeSetupOpenRequest = false;
		}
		if (editor.openPaletteRemoveConfirmDialog)
		{
			ImGui::OpenPopup("Remove palette color");
			editor.openPaletteRemoveConfirmDialog = false;
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
				if (editor.runPendingReplaceAfterSuccessfulSave)
				{
					editor.runPendingReplaceAfterSuccessfulSave = false;
					editor.openUnsavedChangesPopup = true;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		{
			ImVec2 palCenter = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(palCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Palette mode setup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				const int32_t x = editor.paletteSetupDistinctX;
				ImGui::Text("The canvas will be flattened to one layer. Choose how to build the palette.");
				ImGui::Separator();
				ImGui::Checkbox("Dither (Floyd–Steinberg)", &editor.paletteSetupDither);
				ImGui::InputInt("Reduce to Y colors", &editor.paletteSetupReduceY, 0, 0);
				if (editor.paletteSetupReduceY < 1) editor.paletteSetupReduceY = 1;
				ImGui::Spacing();
				char bufAll[128];
				std::snprintf(bufAll, sizeof bufAll, "Use all %d existing colors", (int)x);
				if (ImGui::Button(bufAll, ImVec2(-1, 0)))
				{
					editor.applyPaletteModeActivation(true, editor.paletteSetupReduceY, editor.paletteSetupDither);
					ImGui::CloseCurrentPopup();
				}
				char bufRed[128];
				std::snprintf(bufRed, sizeof bufRed, "Reduce to %d colors", (int)editor.paletteSetupReduceY);
				if (ImGui::Button(bufRed, ImVec2(-1, 0)))
				{
					editor.applyPaletteModeActivation(false, editor.paletteSetupReduceY, editor.paletteSetupDither);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("Cancel", ImVec2(-1, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		{
			ImVec2 rmCenter = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(rmCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Remove palette color", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("This color is used by %lld pixels. They will be remapped to the nearest remaining palette color.",
							(long long)editor.paletteRemovePendingPixelCount);
				ImGui::Spacing();
				if (ImGui::Button("Remove and remap", ImVec2(160, 0)))
				{
					editor.paletteConfirmPendingRemove();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					editor.paletteRemovePendingIndex = -1;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		{
			ImVec2 unsavedCenter = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(unsavedCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				const bool pendingQuit = (editor.pendingDocumentReplace.kind == PaintEditor::PendingDocumentReplace::Kind::QuitApplication);
				ImGui::TextUnformatted(pendingQuit
					? "You have unsaved changes. Save before closing?"
					: "You have unsaved changes. Save before continuing?");
				ImGui::Spacing();
				if (ImGui::Button("Save", ImVec2(120, 0)))
				{
					if (!editor.path.isEmpty())
					{
						if (editor.saveDocumentToPath(editor.path))
						{
							editor.runPendingDocumentReplace();
							editor.clearPendingDocumentReplace();
							ImGui::CloseCurrentPopup();
						}
						else
						{
							editor.openSaveFailedPopup = true;
							editor.reshowUnsavedAfterSaveFail = true;
						}
					}
					else
					{
						editor.runPendingReplaceAfterSuccessfulSave = true;
						editor.openSaveChoicePopup = true;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Don't save", ImVec2(120, 0)))
				{
					editor.runPendingDocumentReplace();
					editor.clearPendingDocumentReplace();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					editor.clearPendingDocumentReplace();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		if (ImGui::BeginPopupModal("Save failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("The file could not be written (permission denied, read-only location, or full disk).");
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				if (editor.reshowUnsavedAfterSaveFail)
				{
					editor.reshowUnsavedAfterSaveFail = false;
					editor.openUnsavedChangesPopup = true;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Clipboard copy failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("The image could not be placed on the clipboard (the platform denied the request or the clipboard is unavailable).");
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
				const bbe::String dropPath = editor.pendingDroppedPaths[0];
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
				editor.requestReplaceDocumentOpen(dropPath);
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
			const ImVec2 helpSize(520.f * (editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f), 480.f * (editor.viewport.scale > 0.f ? editor.viewport.scale : 1.f));
			ImGui::SetNextWindowSize(helpSize, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
			if (ImGui::Begin("ExamplePaint Help", &editor.showHelpWindow, ImGuiWindowFlags_AlwaysVerticalScrollbar))
			{
				auto bulletList = [&](const char *title, std::initializer_list<const char *> items)
				{
					ImGui::SeparatorText(title);
					for (const char *item : items) ImGui::BulletText("%s", item);
				};
				bulletList("Tools", { "Digits 1–9 and 0 trigger customizable actions; hover a control in Tools (tools, pipette, symmetry, undo/redo, selection actions, clipboard) or Layers and press Ctrl+digit to assign", "Bindings are saved to ExamplePaintDigitHotkeys.dat (with ParanoiaConfig backups like ExampleMother)", "Defaults: 1 Brush, 2 Flood Fill, 3 Line, 4 Rectangle, 5 Selection, 6 Text, 7 Pipette, 8 Circle, 9 Arrow, 0 Bezier", "E Eraser, R Spray, O Ellipse selection, L Lasso, P Polygon Lasso, M Magic Wand" });
				bulletList("General", { "+/- changes brush width, eraser size, spray width (spray tool), wand or flood-fill tolerance, or text size for the active tool", "X swaps primary and secondary color", "Ctrl+D resets colors to black/white", "Drag and drop PNG or .bbepaint files to open as a document or add as a new layer", "With unsaved edits, New…, Open…, Paste as New Canvas, drop→Open as Document, Menu→Quit, or closing the window asks Save / Don't save / Cancel", "Space resets the camera", "Middle mouse pans", "Mouse wheel zooms", "Tools, Layers, Colors, and Tool options are separate floating windows: drag title bars to move; resize freely; layout is remembered (imgui.ini)", "Favorite swatches in the Colors window default to white and are saved to ExamplePaintFavoriteColors.dat", "Recent colors default to white, update as you draw, and are saved to ExamplePaintColorHistory.dat", "Most used (Colors window) ranks colors on the visible flattened canvas; fully transparent pixels are skipped" });
				bulletList("Edit", { "Ctrl+S saves", "Ctrl+Z / Ctrl+Y undo and redo", "Delete / Backspace deletes the current selection", "Edit → Mirror flips all layers (vertical or horizontal in the dialog)", "Edit → Rotate Canvas 90° turns all layers; canvas width and height swap", "Edit → Palette Mode enables or disables indexed color for the document (single layer, per-document palette)" });
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
					editor.requestReplaceDocumentNewBlank((uint32_t)newWidth, (uint32_t)newHeight);
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
}
