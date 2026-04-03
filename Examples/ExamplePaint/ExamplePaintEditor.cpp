#include "ExamplePaintEditor.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <random>
#include <vector>

#include <string>

namespace {

void mirrorAllLayersEach(PaintEditor &ed, void (bbe::Image::*mirror)())
{
	if (ed.getCanvasWidth() <= 0 || ed.getCanvasHeight() <= 0) return;
	ed.prepareForLayerTargetChange();
	for (size_t i = 0; i < ed.canvas.get().layers.getLength(); i++)
	{
		(ed.canvas.get().layers[i].image.*mirror)();
		ed.prepareLayer(ed.canvas.get().layers[i]);
	}
	ed.clearWorkArea();
	ed.submitCanvas();
}

void rotateAllLayers90(PaintEditor &ed, bool clockwise)
{
	if (ed.getCanvasWidth() <= 0 || ed.getCanvasHeight() <= 0) return;
	ed.prepareForLayerTargetChange();
	for (size_t i = 0; i < ed.canvas.get().layers.getLength(); i++)
	{
		bbe::Image &img = ed.canvas.get().layers[i].image;
		img = clockwise ? img.rotated90Clockwise() : img.rotated90CounterClockwise();
		ed.prepareLayer(ed.canvas.get().layers[i]);
	}
	ed.clearWorkArea();
	ed.submitCanvas();
}

bool selectionMaskMatchesRect(const bbe::Image &mask, const bbe::Rectanglei &rect)
{
	return rect.width > 0 && rect.height > 0 && mask.getWidth() == (size_t)rect.width && mask.getHeight() == (size_t)rect.height;
}

bool regionPixelOn(const bbe::Rectanglei &r, const bbe::Image &m, int32_t cx, int32_t cy)
{
	if (!r.isPointInRectangle({ cx, cy }, true)) return false;
	if (!selectionMaskMatchesRect(m, r)) return true;
	return m.getPixel((size_t)(cx - r.x), (size_t)(cy - r.y)).a >= 128;
}

void unionSelectionRegions(
	const bbe::Rectanglei &aRect, const bbe::Image &aMask,
	const bbe::Rectanglei &bRect, const bbe::Image &bMask,
	bbe::Rectanglei &outRect, bbe::Image &outMask)
{
	const int32_t aR = aRect.x + aRect.width - 1;
	const int32_t aB = aRect.y + aRect.height - 1;
	const int32_t bR = bRect.x + bRect.width - 1;
	const int32_t bB = bRect.y + bRect.height - 1;
	const int32_t L = bbe::Math::min(aRect.x, bRect.x);
	const int32_t T = bbe::Math::min(aRect.y, bRect.y);
	const int32_t R = bbe::Math::max(aR, bR);
	const int32_t B = bbe::Math::max(aB, bB);
	outRect = bbe::Rectanglei(L, T, R - L + 1, B - T + 1);
	outMask = bbe::Image(outRect.width, outRect.height, bbe::Color(0.f, 0.f, 0.f, 0.f));
	for (int32_t y = 0; y < outRect.height; y++)
	{
		for (int32_t x = 0; x < outRect.width; x++)
		{
			const int32_t cx = outRect.x + x;
			const int32_t cy = outRect.y + y;
			const bool on = regionPixelOn(aRect, aMask, cx, cy) || regionPixelOn(bRect, bMask, cx, cy);
			if (on) outMask.setPixel((size_t)x, (size_t)y, bbe::Colori(255, 255, 255, 255));
		}
	}
}

void subtractSelectionRegions(
	const bbe::Rectanglei &aRect, const bbe::Image &aMask,
	const bbe::Rectanglei &bRect, const bbe::Image &bMask,
	bbe::Rectanglei &outRect, bbe::Image &outMask)
{
	if (aRect.width <= 0 || aRect.height <= 0)
	{
		outRect = {};
		outMask = {};
		return;
	}
	int32_t minx = 0, miny = 0, maxx = -1, maxy = -1;
	bool any = false;
	for (int32_t y = 0; y < aRect.height; y++)
	{
		for (int32_t x = 0; x < aRect.width; x++)
		{
			const int32_t cx = aRect.x + x;
			const int32_t cy = aRect.y + y;
			if (!regionPixelOn(aRect, aMask, cx, cy)) continue;
			if (regionPixelOn(bRect, bMask, cx, cy)) continue;
			if (!any)
			{
				minx = maxx = cx;
				miny = maxy = cy;
				any = true;
			}
			else
			{
				minx = bbe::Math::min(minx, cx);
				maxx = bbe::Math::max(maxx, cx);
				miny = bbe::Math::min(miny, cy);
				maxy = bbe::Math::max(maxy, cy);
			}
		}
	}
	if (!any)
	{
		outRect = {};
		outMask = {};
		return;
	}
	const int32_t rw = maxx - minx + 1;
	const int32_t rh = maxy - miny + 1;
	outRect = bbe::Rectanglei(minx, miny, rw, rh);
	outMask = bbe::Image(rw, rh, bbe::Color(0.f, 0.f, 0.f, 0.f));
	for (int32_t y = miny; y <= maxy; y++)
	{
		for (int32_t x = minx; x <= maxx; x++)
		{
			if (regionPixelOn(aRect, aMask, x, y) && !regionPixelOn(bRect, bMask, x, y))
			{
				outMask.setPixel((size_t)(x - minx), (size_t)(y - miny), bbe::Colori(255, 255, 255, 255));
			}
		}
	}
}

void dropMaskIfFullySelected(bbe::Rectanglei &rect, bbe::Image &mask)
{
	if (!selectionMaskMatchesRect(mask, rect)) return;
	for (int32_t y = 0; y < rect.height; y++)
	{
		for (int32_t x = 0; x < rect.width; x++)
		{
			if (mask.getPixel((size_t)x, (size_t)y).a < 250) return;
		}
	}
	mask = {};
}

bool pointInPolygon(double px, double py, const std::vector<bbe::Vector2> &poly)
{
	if (poly.size() < 3) return false;
	bool inside = false;
	const size_t n = poly.size();
	for (size_t i = 0, j = n - 1; i < n; j = i++)
	{
		const double xi = (double)poly[i].x, yi = (double)poly[i].y;
		const double xj = (double)poly[j].x, yj = (double)poly[j].y;
		if (yj == yi) continue;
		const bool intersect = ((yi > py) != (yj > py)) && (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
		if (intersect) inside = !inside;
	}
	return inside;
}

// True if segment (x1,y1)-(x2,y2) meets the closed axis-aligned rectangle [rx0,rx1]×[ry0,ry1].
bool segmentIntersectsClosedRect(double x1, double y1, double x2, double y2, double rx0, double ry0, double rx1, double ry1)
{
	auto pin = [&](double x, double y) { return x >= rx0 && x <= rx1 && y >= ry0 && y <= ry1; };
	if (pin(x1, y1) || pin(x2, y2)) return true;

	const double dx = x2 - x1;
	const double dy = y2 - y1;
	double t0 = 0.0;
	double t1 = 1.0;
	constexpr double peps = 1e-15;
	auto clip = [&](double p, double q) -> bool
	{
		if (std::fabs(p) < peps) return q >= -1e-9;
		const double r = q / p;
		if (p < 0.0)
		{
			if (r > t1) return false;
			if (r > t0) t0 = r;
		}
		else
		{
			if (r < t0) return false;
			if (r < t1) t1 = r;
		}
		return true;
	};

	if (!clip(-dx, x1 - rx0)) return false;
	if (!clip(dx, rx1 - x1)) return false;
	if (!clip(-dy, y1 - ry0)) return false;
	if (!clip(dy, ry1 - y1)) return false;
	return t0 <= t1 + 1e-10;
}

// Pixel (cx,cy) = closed unit square [cx,cx+1]×[cy,cy+1]. Selected if the polygon interior (sampled) or boundary touches that square.
bool pixelTouchedByLassoPolygon(int32_t cx, int32_t cy, const std::vector<bbe::Vector2> &poly)
{
	const double x0 = (double)cx;
	const double y0 = (double)cy;
	const double x1 = (double)cx + 1.0;
	const double y1 = (double)cy + 1.0;
	const size_t n = poly.size();

	for (size_t i = 0; i < n; i++)
	{
		const double ax = (double)poly[i].x;
		const double ay = (double)poly[i].y;
		const double bx = (double)poly[(i + 1) % n].x;
		const double by = (double)poly[(i + 1) % n].y;
		if (segmentIntersectsClosedRect(ax, ay, bx, by, x0, y0, x1, y1)) return true;
		if (ax >= x0 && ax <= x1 && ay >= y0 && ay <= y1) return true;
	}

	constexpr double e = 1e-3;
	if (pointInPolygon(x0 + 0.5, y0 + 0.5, poly)) return true;
	if (pointInPolygon(x0 + e, y0 + e, poly)) return true;
	if (pointInPolygon(x1 - e, y0 + e, poly)) return true;
	if (pointInPolygon(x1 - e, y1 - e, poly)) return true;
	if (pointInPolygon(x0 + e, y1 - e, poly)) return true;
	return false;
}

/// Fills pixels whose closed unit squares intersect the closed polygon `path` (interior or edge). Tight bounding output.
bool buildLassoSelectionMask(const PaintEditor &ed, const std::vector<bbe::Vector2> &path, bbe::Rectanglei &outTightRect, bbe::Image &outMask)
{
	const int32_t W = ed.getCanvasWidth();
	const int32_t H = ed.getCanvasHeight();
	if (W <= 0 || H <= 0 || path.size() < 3) return false;

	float fminx = path[0].x, fmaxx = path[0].x, fminy = path[0].y, fmaxy = path[0].y;
	for (size_t i = 1; i < path.size(); i++)
	{
		fminx = bbe::Math::min(fminx, path[i].x);
		fmaxx = bbe::Math::max(fmaxx, path[i].x);
		fminy = bbe::Math::min(fminy, path[i].y);
		fmaxy = bbe::Math::max(fmaxy, path[i].y);
	}
	int32_t minx = (int32_t)std::floor((double)fminx);
	int32_t maxx = (int32_t)std::ceil((double)fmaxx) - 1;
	int32_t miny = (int32_t)std::floor((double)fminy);
	int32_t maxy = (int32_t)std::ceil((double)fmaxy) - 1;
	if (maxx < minx) maxx = minx;
	if (maxy < miny) maxy = miny;
	// Expand: thin strokes along pixel edges can intersect neighbors of the vertex bbox.
	minx -= 1;
	maxx += 1;
	miny -= 1;
	maxy += 1;
	bbe::Rectanglei bbox(minx, miny, maxx - minx + 1, maxy - miny + 1);
	bbe::Rectanglei clipR;
	if (!ed.clampRectToCanvas(bbox, clipR)) return false;

	std::vector<uint8_t> sel((size_t)clipR.width * (size_t)clipR.height, 0);
	bool any = false;
	int32_t tightL = 0, tightT = 0, tightR = 0, tightB = 0;

	for (int32_t y = 0; y < clipR.height; y++)
	{
		for (int32_t x = 0; x < clipR.width; x++)
		{
			const int32_t cx = clipR.x + x;
			const int32_t cy = clipR.y + y;
			if (!pixelTouchedByLassoPolygon(cx, cy, path)) continue;
			sel[(size_t)y * (size_t)clipR.width + (size_t)x] = 1;
			if (!any)
			{
				tightL = tightR = cx;
				tightT = tightB = cy;
				any = true;
			}
			else
			{
				tightL = bbe::Math::min(tightL, cx);
				tightR = bbe::Math::max(tightR, cx);
				tightT = bbe::Math::min(tightT, cy);
				tightB = bbe::Math::max(tightB, cy);
			}
		}
	}
	if (!any) return false;

	const int32_t rw = tightR - tightL + 1;
	const int32_t rh = tightB - tightT + 1;
	outTightRect = bbe::Rectanglei(tightL, tightT, rw, rh);
	outMask = bbe::Image(rw, rh, bbe::Color(0.f, 0.f, 0.f, 0.f));
	ed.prepareImageForCanvas(outMask);

	for (int32_t cy = tightT; cy <= tightB; cy++)
	{
		for (int32_t cx = tightL; cx <= tightR; cx++)
		{
			const int32_t lx = cx - clipR.x;
			const int32_t ly = cy - clipR.y;
			if (lx < 0 || ly < 0 || lx >= clipR.width || ly >= clipR.height) continue;
			if (!sel[(size_t)ly * (size_t)clipR.width + (size_t)lx]) continue;
			outMask.setPixel((size_t)(cx - tightL), (size_t)(cy - tightT), bbe::Colori(255, 255, 255, 255));
		}
	}
	return true;
}

/// Axis-aligned ellipse inscribed in `bbox` (pixel centers). Tight bounding output.
bool buildEllipseSelectionMask(const PaintEditor &ed, const bbe::Rectanglei &bbox, bbe::Rectanglei &outTightRect, bbe::Image &outMask)
{
	if (bbox.width <= 0 || bbox.height <= 0) return false;
	bbe::Rectanglei clipR;
	if (!ed.clampRectToCanvas(bbox, clipR)) return false;

	const double w = (double)clipR.width;
	const double h = (double)clipR.height;
	const double cx = (double)clipR.x + w * 0.5;
	const double cy = (double)clipR.y + h * 0.5;
	const double rx = w * 0.5;
	const double ry = h * 0.5;
	if (rx < 1e-9 || ry < 1e-9) return false;

	std::vector<uint8_t> sel((size_t)clipR.width * (size_t)clipR.height, 0);
	bool any = false;
	int32_t tightL = 0, tightT = 0, tightR = 0, tightB = 0;

	for (int32_t y = 0; y < clipR.height; y++)
	{
		for (int32_t x = 0; x < clipR.width; x++)
		{
			const double pxf = (double)clipR.x + (double)x + 0.5;
			const double pyf = (double)clipR.y + (double)y + 0.5;
			const double dx = (pxf - cx) / rx;
			const double dy = (pyf - cy) / ry;
			if (dx * dx + dy * dy > 1.0) continue;
			sel[(size_t)y * (size_t)clipR.width + (size_t)x] = 1;
			const int32_t cax = clipR.x + x;
			const int32_t cay = clipR.y + y;
			if (!any)
			{
				tightL = tightR = cax;
				tightT = tightB = cay;
				any = true;
			}
			else
			{
				tightL = bbe::Math::min(tightL, cax);
				tightR = bbe::Math::max(tightR, cax);
				tightT = bbe::Math::min(tightT, cay);
				tightB = bbe::Math::max(tightB, cay);
			}
		}
	}
	if (!any) return false;

	const int32_t rw = tightR - tightL + 1;
	const int32_t rh = tightB - tightT + 1;
	outTightRect = bbe::Rectanglei(tightL, tightT, rw, rh);
	outMask = bbe::Image(rw, rh, bbe::Color(0.f, 0.f, 0.f, 0.f));
	ed.prepareImageForCanvas(outMask);

	for (int32_t cy = tightT; cy <= tightB; cy++)
	{
		for (int32_t cx = tightL; cx <= tightR; cx++)
		{
			const int32_t lx = cx - clipR.x;
			const int32_t ly = cy - clipR.y;
			if (lx < 0 || ly < 0 || lx >= clipR.width || ly >= clipR.height) continue;
			if (!sel[(size_t)ly * (size_t)clipR.width + (size_t)lx]) continue;
			outMask.setPixel((size_t)(cx - tightL), (size_t)(cy - tightT), bbe::Colori(255, 255, 255, 255));
		}
	}
	return true;
}

bbe::Image copyLayerRectWithMask(const PaintEditor &editor, const bbe::Rectanglei &rect, const bbe::Image &mask)
{
	bbe::Image copied(rect.width, rect.height, bbe::Color(0.f, 0.f, 0.f, 0.f));
	editor.prepareImageForCanvas(copied);
	const bool useMask = selectionMaskMatchesRect(mask, rect);
	for (int32_t x = 0; x < rect.width; x++)
	{
		for (int32_t y = 0; y < rect.height; y++)
		{
			bbe::Colori p = editor.getActiveLayerImage().getPixel((size_t)(rect.x + x), (size_t)(rect.y + y));
			if (useMask)
			{
				const int32_t ma = mask.getPixel((size_t)x, (size_t)y).a;
				p.a = (bbe::byte)bbe::Math::clamp((int32_t)p.a * ma / 255, 0, 255);
			}
			copied.setPixel((size_t)x, (size_t)y, p);
		}
	}
	return copied;
}

void clearLayerRectWithMask(PaintEditor &editor, const bbe::Rectanglei &rect, const bbe::Image &mask)
{
	const bool useMask = selectionMaskMatchesRect(mask, rect);
	constexpr bbe::Colori kClear{0, 0, 0, 0};
	for (int32_t x = 0; x < rect.width; x++)
	{
		for (int32_t y = 0; y < rect.height; y++)
		{
			if (useMask && mask.getPixel((size_t)x, (size_t)y).a < 128) continue;
			editor.getActiveLayerImage().setPixel((size_t)(rect.x + x), (size_t)(rect.y + y), kClear);
		}
	}
}

bool colorWithinTolerance(const bbe::Colori &a, const bbe::Colori &b, int32_t tol)
{
	return bbe::Math::abs<int32_t>((int32_t)a.r - b.r) <= tol && bbe::Math::abs<int32_t>((int32_t)a.g - b.g) <= tol &&
		   bbe::Math::abs<int32_t>((int32_t)a.b - b.b) <= tol && bbe::Math::abs<int32_t>((int32_t)a.a - b.a) <= tol;
}

bbe::Image maskFromFloatingImageAlpha(const bbe::Image &floating, int32_t w, int32_t h)
{
	bbe::Image m(w, h, bbe::Color(0.f, 0.f, 0.f, 0.f));
	const int32_t fw = (int32_t)floating.getWidth();
	const int32_t fh = (int32_t)floating.getHeight();
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			if (x < fw && y < fh && floating.getPixel((size_t)x, (size_t)y).a >= 128)
			{
				m.setPixel((size_t)x, (size_t)y, bbe::Colori(255, 255, 255, 255));
			}
		}
	}
	return m;
}

} // namespace

bbe::Colori PaintEditor::getColor(bool useRight) const
{
	return bbe::Color(useRight ? rightColor : leftColor).asByteColor();
}

bool PaintEditor::isSelectionLikeTool(int32_t toolMode)
{
	return toolMode == MODE_SELECTION || toolMode == MODE_MAGIC_WAND || toolMode == MODE_LASSO || toolMode == MODE_POLYGON_LASSO ||
		   toolMode == MODE_ELLIPSE_SELECTION;
}

void PaintEditor::clampMagicWandTolerance()
{
	magicWandTolerance = bbe::Math::clamp(magicWandTolerance, 0, 255);
}

void PaintEditor::clampFloodFillTolerance()
{
	floodFillTolerance = bbe::Math::clamp(floodFillTolerance, 0, 255);
}

bool PaintEditor::hasSelectionPixelMask() const
{
	return selection.hasSelection && selectionMaskMatchesRect(selection.mask, selection.rect);
}

void PaintEditor::applyMagicWandAt(const bbe::Vector2i &pixel, bool additive)
{
	clampMagicWandTolerance();
	const int32_t W = getCanvasWidth();
	const int32_t H = getCanvasHeight();
	if (W <= 0 || H <= 0) return;
	if (pixel.x < 0 || pixel.y < 0 || pixel.x >= W || pixel.y >= H) return;

	const bool subtractWand = isPointInSelection(pixel);

	bbe::Image flat;
	if (selection.floating && additive)
	{
		const bbe::Rectanglei fr = selection.rect;
		flat = flattenVisibleLayers();
		flat.blendOver(selection.floatingImage, fr.getPos(), tiled);

		bbe::Image fromFloat = maskFromFloatingImageAlpha(selection.floatingImage, fr.width, fr.height);
		selection.floating = false;
		selection.floatingImage = {};
		selection.hasSelection = true;
		selection.rect = fr;
		selection.mask = std::move(fromFloat);
		prepareImageForCanvas(selection.mask);
		dropMaskIfFullySelected(selection.rect, selection.mask);
	}
	else
	{
		if (selection.floating)
		{
			commitFloatingSelection();
		}
		flat = flattenVisibleLayers();
	}
	const bbe::Colori ref = flat.getPixel((size_t)pixel.x, (size_t)pixel.y);
	const int32_t tol = magicWandTolerance;

	std::vector<uint8_t> vis((size_t)W * (size_t)H, 0);
	std::deque<bbe::Vector2i> q;
	q.push_back(pixel);
	vis[(size_t)pixel.y * (size_t)W + (size_t)pixel.x] = 1;

	int32_t minx = pixel.x, maxx = pixel.x, miny = pixel.y, maxy = pixel.y;

	while (!q.empty())
	{
		const bbe::Vector2i p = q.front();
		q.pop_front();
		static const int32_t dirs[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (size_t di = 0; di < 4; di++)
		{
			const int32_t nx = p.x + dirs[di][0];
			const int32_t ny = p.y + dirs[di][1];
			if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
			const size_t idx = (size_t)ny * (size_t)W + (size_t)nx;
			if (vis[idx]) continue;
			if (!colorWithinTolerance(flat.getPixel((size_t)nx, (size_t)ny), ref, tol)) continue;
			vis[idx] = 1;
			q.push_back({ nx, ny });
			minx = bbe::Math::min(minx, nx);
			maxx = bbe::Math::max(maxx, nx);
			miny = bbe::Math::min(miny, ny);
			maxy = bbe::Math::max(maxy, ny);
		}
	}

	const int32_t rw = maxx - minx + 1;
	const int32_t rh = maxy - miny + 1;
	bbe::Image newMask(rw, rh, bbe::Color(0.f, 0.f, 0.f, 0.f));
	prepareImageForCanvas(newMask);
	for (int32_t y = miny; y <= maxy; y++)
	{
		for (int32_t x = minx; x <= maxx; x++)
		{
			if (!vis[(size_t)y * (size_t)W + (size_t)x]) continue;
			newMask.setPixel((size_t)(x - minx), (size_t)(y - miny), bbe::Colori(255, 255, 255, 255));
		}
	}

	const bbe::Rectanglei newRect(minx, miny, rw, rh);
	if (subtractWand)
	{
		if (!selection.hasSelection) return;
		bbe::Rectanglei outR;
		bbe::Image outM;
		subtractSelectionRegions(selection.rect, selection.mask, newRect, newMask, outR, outM);
		prepareImageForCanvas(outM);
		selection.rect = outR;
		selection.mask = std::move(outM);
		selection.hasSelection = outR.width > 0 && outR.height > 0;
		dropMaskIfFullySelected(selection.rect, selection.mask);
		return;
	}

	if (!additive || !selection.hasSelection)
	{
		selection.hasSelection = true;
		selection.rect = newRect;
		selection.mask = std::move(newMask);
		dropMaskIfFullySelected(selection.rect, selection.mask);
		return;
	}

	bbe::Rectanglei outR;
	bbe::Image outM;
	unionSelectionRegions(selection.rect, selection.mask, newRect, newMask, outR, outM);
	selection.rect = outR;
	selection.mask = std::move(outM);
	selection.hasSelection = outR.width > 0 && outR.height > 0;
	dropMaskIfFullySelected(selection.rect, selection.mask);
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
		const SelectionHitZone hitZone = getSelectionHitZone(canvasPos);
		if (hitZone == SelectionHitZone::ROTATION && selection.hasSelection)
		{
			beginRotationDrag(mousePixel);
		}
		else if (isSelectionResizeHit(hitZone) && !selectionAdditiveModifier)
		{
			beginSelectionResize(hitZone);
		}
		else if (hitZone == SelectionHitZone::INSIDE && !selectionAdditiveModifier)
		{
			beginSelectionMove(mousePixel);
		}
		else
		{
			// Outside, Ctrl+inside, or Ctrl on handles: new rect (additive) / clear / empty drag.
			pointerDownSelectionDefaultMarqueePath(mousePixel);
		}
		break;
	}
	case MODE_ELLIPSE_SELECTION:
	{
		if (button != PointerButton::Primary) break;
		const SelectionHitZone hitZone = getSelectionHitZone(canvasPos);
		if (hitZone == SelectionHitZone::ROTATION && selection.hasSelection)
		{
			beginRotationDrag(mousePixel);
		}
		else if (isSelectionResizeHit(hitZone) && !selectionAdditiveModifier)
		{
			beginSelectionResize(hitZone);
		}
		else if (hitZone == SelectionHitZone::INSIDE && !selectionAdditiveModifier)
		{
			beginSelectionMove(mousePixel);
		}
		else
		{
			pointerDownSelectionDefaultMarqueePath(mousePixel);
		}
		break;
	}
	case MODE_LASSO:
	{
		if (button != PointerButton::Primary) break;
		const SelectionHitZone hitZone = getSelectionHitZone(canvasPos);
		if (hitZone == SelectionHitZone::ROTATION && selection.hasSelection)
		{
			beginRotationDrag(mousePixel);
		}
		else if (isSelectionResizeHit(hitZone) && !selectionAdditiveModifier)
		{
			beginSelectionResize(hitZone);
		}
		else if (hitZone == SelectionHitZone::INSIDE && !selectionAdditiveModifier)
		{
			beginSelectionMove(mousePixel);
		}
		else
		{
			pointerDownLassoMarqueePath(canvasPos);
		}
		break;
	}
	case MODE_POLYGON_LASSO:
	{
		if (button == PointerButton::Secondary)
		{
			if (selection.polygonLassoVertices.size() >= 3)
			{
				finishPolygonLassoSelection();
			}
			else
			{
				cancelPolygonLassoDraft();
			}
			break;
		}
		if (button != PointerButton::Primary) break;
		const SelectionHitZone hitZone = getSelectionHitZone(canvasPos);
		if (hitZone == SelectionHitZone::ROTATION && selection.hasSelection)
		{
			beginRotationDrag(mousePixel);
		}
		else if (isSelectionResizeHit(hitZone) && !selectionAdditiveModifier)
		{
			beginSelectionResize(hitZone);
		}
		else if (hitZone == SelectionHitZone::INSIDE && !selectionAdditiveModifier)
		{
			beginSelectionMove(mousePixel);
		}
		else
		{
			if (!selection.polygonLassoVertices.empty())
			{
				if (isClickClosePolygonLasso(canvasPos))
				{
					finishPolygonLassoSelection();
				}
				else
				{
					appendPolygonLassoVertex(mousePixel);
				}
			}
			else
			{
				pointerDownPolygonLassoMarqueePath(mousePixel);
			}
		}
		break;
	}
	case MODE_MAGIC_WAND:
	{
		if (button != PointerButton::Primary && button != PointerButton::Secondary) break;
		const SelectionHitZone hitZone = getSelectionHitZone(canvasPos);
		if (button == PointerButton::Primary && hitZone == SelectionHitZone::ROTATION && selection.hasSelection)
		{
			beginRotationDrag(mousePixel);
		}
		else if (button == PointerButton::Primary && isSelectionResizeHit(hitZone) && !selectionAdditiveModifier)
		{
			beginSelectionResize(hitZone);
		}
		else if (button == PointerButton::Primary && isSelectionResizeHit(hitZone) && selectionAdditiveModifier)
		{
			// Ctrl: edge/corner resize disabled for the wand tool.
		}
		else if (button == PointerButton::Primary && hitZone == SelectionHitZone::INSIDE && !selectionAdditiveModifier)
		{
			beginSelectionMove(mousePixel);
		}
		else if (button == PointerButton::Primary && hitZone == SelectionHitZone::INSIDE && selectionAdditiveModifier)
		{
			// Ctrl: move disabled; wand sample still runs from ExamplePaint if applicable.
		}
		else
		{
			const bool hadFloating = selection.floating;
			const SelectionHitZone hzBefore = hitZone;

			if (selection.floating && !selectionAdditiveModifier)
			{
				commitFloatingSelection();
			}

			if (hadFloating && !selectionAdditiveModifier)
			{
				skipMagicWandSampleOnce = true;
			}
			else if (!selectionAdditiveModifier && selection.hasSelection && hzBefore == SelectionHitZone::NONE)
			{
				clearMarqueePreservingClipboard();
				skipMagicWandSampleOnce = true;
			}
		}
		break;
	}
	case MODE_RECTANGLE:
	case MODE_CIRCLE:
	{
		ShapeDragState &shape = (mode == MODE_CIRCLE) ? circle : rectangle;
		const bool isCircle = (mode == MODE_CIRCLE);
		const bool handled = handleFloatingDraftInteraction(shape.draftActive, canvasPos, button);
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
		endpointPointerDown(line, button, canvasPos);
		break;
	case MODE_ARROW:
		endpointPointerDown(arrow, button, canvasPos);
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
	case MODE_MAGIC_WAND:
	case MODE_LASSO:
	case MODE_POLYGON_LASSO:
	case MODE_ELLIPSE_SELECTION:
	{
		if (!pointerPrimaryDown) break;
		if (selection.rotationHandleActive)
		{
			updateRotationDrag(mousePixel);
		}
		if (mode == MODE_SELECTION && selection.dragActive)
		{
			buildSelectionRect(selection.dragStart, mousePixel, selection.previewRect);
		}
		if (mode == MODE_ELLIPSE_SELECTION && selection.dragActive)
		{
			buildEllipseMarqueeRect(selection.dragStart, mousePixel, selection.previewRect);
		}
		if (mode == MODE_LASSO && selection.lassoDragActive)
		{
			appendLassoPoint(canvasPos);
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
		endpointPointerMove(line, canvasPos);
		break;
	case MODE_ARROW:
		endpointPointerMove(arrow, canvasPos);
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
			bbe::Rectanglei newRect;
			const bool ok = buildSelectionRect(selection.dragStart, mousePixel, newRect);
			const bool merge = selection.mergeBackupHadSelection && selectionAdditiveModifier;
			selection.dragActive = false;
			selection.previewRect = {};
			if (merge && ok && newRect.width > 0 && newRect.height > 0)
			{
				bbe::Rectanglei outR;
				bbe::Image outM;
				unionSelectionRegions(selection.mergeBackupRect, selection.mergeBackupMask, newRect, {}, outR, outM);
				selection.rect = outR;
				selection.mask = std::move(outM);
				selection.hasSelection = true;
				dropMaskIfFullySelected(selection.rect, selection.mask);
			}
			else if (merge && (!ok || newRect.width <= 0 || newRect.height <= 0))
			{
				selection.rect = selection.mergeBackupRect;
				selection.mask = std::move(selection.mergeBackupMask);
				selection.hasSelection = selection.mergeBackupHadSelection;
			}
			else
			{
				selection.hasSelection = ok;
				if (ok) selection.rect = newRect;
				else selection.rect = {};
				selection.mask = {};
			}
			selection.mergeBackupHadSelection = false;
			selection.mergeBackupRect = {};
			selection.mergeBackupMask = {};
		}
		break;
	}
	case MODE_ELLIPSE_SELECTION:
	{
		if (button != PointerButton::Primary) break;
		if (selection.dragActive)
		{
			finishEllipseMarqueeDrag(mousePixel);
			selection.dragActive = false;
			selection.previewRect = {};
		}
		break;
	}
	case MODE_LASSO:
	{
		if (button != PointerButton::Primary) break;
		if (selection.lassoDragActive)
		{
			finishLassoDrag(canvasPos);
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

void PaintEditor::redrawEndpointDraft(EndpointDraftState &state)
{
	if (!state.draftActive) return;
	clearWorkArea();
	const bool forArrow = (&state == &arrow);
	if (forArrow) drawArrowSymmetry(state.start, state.end, getColor(state.draftUsesRightColor));
	else touchLineSymmetry(state.end, state.start, getColor(state.draftUsesRightColor), brushWidth);
}

void PaintEditor::endpointPointerDown(EndpointDraftState &state, PointerButton button, const bbe::Vector2 &mouseCanvas)
{
	const bbe::Vector2 pos = lineEndpointCanvasPos(state, mouseCanvas);
	const bool forArrow = (&state == &arrow);
	// Adjust active draft by dragging endpoints or commit/cancel.
	if (state.draftActive)
	{
		if (button == PointerButton::Primary)
		{
			const float handleRadius = 6.f / zoomLevel;
			const float distToStart = (pos - state.start).getLength();
			const float distToEnd = (pos - state.end).getLength();
			if (distToStart <= handleRadius && distToStart <= distToEnd) state.dragEndpoint = 1;
			else if (distToEnd <= handleRadius) state.dragEndpoint = 2;
			else
			{
				if (forArrow) finalizeArrowDraft();
				else finalizeLineDraft();
				return;
			}
			redrawEndpointDraft(state);
			return;
		}
		if (button == PointerButton::Secondary)
		{
			if (forArrow) finalizeArrowDraft();
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
		state.start = pos;
		clearWorkArea();
	}
}

void PaintEditor::endpointPointerMove(EndpointDraftState &state, const bbe::Vector2 &mouseCanvas)
{
	const bbe::Vector2 pos = lineEndpointCanvasPos(state, mouseCanvas);
	const bool forArrow = (&state == &arrow);
	if (state.draftActive)
	{
		if (state.dragEndpoint != 0 && pointerPrimaryDown)
		{
			(state.dragEndpoint == 1 ? state.start : state.end) = pos;
		}
		redrawEndpointDraft(state);
		return;
	}
	if (!state.dragInProgress) return;

	clearWorkArea();
	if (forArrow) drawArrowSymmetry(state.start, pos, getColor(state.dragUsesRightColor));
	else touchLineSymmetry(pos, state.start, getColor(state.dragUsesRightColor), brushWidth);
}

void PaintEditor::endpointPointerUp(EndpointDraftState &state, PointerButton button, const bbe::Vector2 &mouseCanvas)
{
	const bbe::Vector2 pos = lineEndpointCanvasPos(state, mouseCanvas);
	if (state.draftActive)
	{
		if (button == PointerButton::Primary) state.dragEndpoint = 0;
		return;
	}
	if (!state.dragInProgress) return;

	const bool releaseMatches = (state.dragUsesRightColor && button == PointerButton::Secondary) || (!state.dragUsesRightColor && button == PointerButton::Primary);
	if (!releaseMatches) return;

	state.end = pos;
	state.dragInProgress = false;

	if (endpointApplyStrokeOnRelease)
	{
		clearWorkArea();
		const bool forArrow = (&state == &arrow);
		if (forArrow) drawArrowSymmetry(state.start, state.end, getColor(state.dragUsesRightColor));
		else touchLineSymmetry(state.end, state.start, getColor(state.dragUsesRightColor), brushWidth);
		applyWorkArea();
		submitCanvas();
		state.draftActive = false;
		state.dragEndpoint = 0;
		return;
	}

	state.draftActive = true;
	state.draftUsesRightColor = state.dragUsesRightColor;
}

bool PaintEditor::handleFloatingDraftInteraction(bool draftActive, const bbe::Vector2 &canvasPos, PointerButton button)
{
	if (!draftActive) return false;
	const bbe::Vector2i mousePixel = toCanvasPixel(canvasPos);
	if (button == PointerButton::Primary)
	{
		const SelectionHitZone hitZone = getSelectionHitZone(canvasPos);
		if (hitZone == SelectionHitZone::ROTATION) beginRotationDrag(mousePixel);
		else if (isSelectionResizeHit(hitZone) && !selectionAdditiveModifier) beginSelectionResize(hitZone);
		else if (isSelectionResizeHit(hitZone) && selectionAdditiveModifier)
		{
			// Ctrl: no resize on floating shape drafts.
		}
		else if (hitZone == SelectionHitZone::INSIDE && !selectionAdditiveModifier) beginSelectionMove(mousePixel);
		else if (hitZone == SelectionHitZone::INSIDE && selectionAdditiveModifier)
		{
			// Ctrl: no move on floating shape drafts.
		}
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

void PaintEditor::clearMarqueePreservingClipboard()
{
	bbe::Image savedClip = std::move(selection.clipboard);
	selection = SelectionState{};
	selection.clipboard = std::move(savedClip);
}

void PaintEditor::deselectAll()
{
	if (selection.rotationHandleActive) selection.rotationHandleActive = false;
	if (selection.moveActive || selection.resizeActive) applySelectionTransform();
	if (selection.dragActive)
	{
		selection.dragActive = false;
		selection.previewRect = {};
		selection.previewImage = {};
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupRect = {};
		selection.mergeBackupMask = {};
	}
	if (selection.lassoDragActive)
	{
		selection.lassoDragActive = false;
		selection.lassoPath.clear();
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupRect = {};
		selection.mergeBackupMask = {};
	}
	if (!selection.polygonLassoVertices.empty())
	{
		selection.polygonLassoVertices.clear();
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupRect = {};
		selection.mergeBackupMask = {};
	}
	if (selection.floating) commitFloatingSelection();
	clearMarqueePreservingClipboard();
}

bool PaintEditor::consumeMagicWandSuppressedPick()
{
	const bool s = skipMagicWandSampleOnce;
	skipMagicWandSampleOnce = false;
	return s;
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
	const float *const cf = canvas.get().canvasFallbackRgba;
	bbe::Image flattened(getCanvasWidth(), getCanvasHeight(), bbe::Color(cf[0], cf[1], cf[2], cf[3]));
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
	const float *const cf = canvas.get().canvasFallbackRgba;
	bbe::Colori color = bbe::Color(cf[0], cf[1], cf[2], cf[3]).asByteColor();
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

void PaintEditor::mirrorAllLayersHorizontally()
{
	mirrorAllLayersEach(*this, &bbe::Image::mirrorHorizontally);
}

void PaintEditor::mirrorAllLayersVertically()
{
	mirrorAllLayersEach(*this, &bbe::Image::mirrorVertically);
}

void PaintEditor::rotateAllLayers90Clockwise()
{
	rotateAllLayers90(*this, true);
}

void PaintEditor::rotateAllLayers90CounterClockwise()
{
	rotateAllLayers90(*this, false);
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
			const bool importIsV3 = (magic == LAYERED_FILE_MAGIC_V3);
			const bool importIsV2 = (magic == LAYERED_FILE_MAGIC) || importIsV3;
			const bool importIsV1 = (magic == LAYERED_FILE_MAGIC_V1);
			if (!importIsV2 && !importIsV1) continue;
			int32_t width = 0, height = 0;
			uint32_t layerCount = 0;
			int32_t storedActiveLayerIndex = 0;
			span.read(width);
			span.read(height);
			span.read(layerCount);
			span.read(storedActiveLayerIndex);
			if (importIsV3)
			{
				for (int c = 0; c < 4; c++) span.read(canvas.get().canvasFallbackRgba[c]);
			}
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

bbe::Vector2 PaintEditor::lineEndpointCanvasPos(const EndpointDraftState &state, const bbe::Vector2 &canvas) const
{
	if (antiAliasingEnabled || &state != &line) return canvas;
	const bbe::Vector2i p = toCanvasPixel(canvas);
	return bbe::Vector2((float)p.x + 0.5f, (float)p.y + 0.5f);
}

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

bool PaintEditor::buildEllipseMarqueeRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
{
	const bbe::Vector2i p2 = constrainSquareEnabled ? constrainToSquare(pos1, pos2) : pos2;
	return buildSelectionRect(pos1, p2, outRect);
}

void PaintEditor::finishEllipseMarqueeDrag(const bbe::Vector2i &mousePixel)
{
	bbe::Rectanglei bbox;
	const bool bboxOk = buildEllipseMarqueeRect(selection.dragStart, mousePixel, bbox);
	bbe::Rectanglei newRect;
	bbe::Image newMask;
	const bool hasShape = bboxOk && buildEllipseSelectionMask(*this, bbox, newRect, newMask);
	const bool merge = selection.mergeBackupHadSelection && selectionAdditiveModifier;

	if (!hasShape)
	{
		if (merge)
		{
			selection.rect = selection.mergeBackupRect;
			selection.mask = std::move(selection.mergeBackupMask);
			selection.hasSelection = selection.mergeBackupHadSelection;
		}
		else
		{
			selection.hasSelection = false;
			selection.rect = {};
			selection.mask = {};
		}
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupRect = {};
		selection.mergeBackupMask = {};
		return;
	}

	prepareImageForCanvas(newMask);
	dropMaskIfFullySelected(newRect, newMask);

	if (merge)
	{
		bbe::Rectanglei outR;
		bbe::Image outM;
		unionSelectionRegions(selection.mergeBackupRect, selection.mergeBackupMask, newRect, newMask, outR, outM);
		prepareImageForCanvas(outM);
		selection.rect = outR;
		selection.mask = std::move(outM);
		selection.hasSelection = outR.width > 0 && outR.height > 0;
		dropMaskIfFullySelected(selection.rect, selection.mask);
	}
	else
	{
		selection.hasSelection = true;
		selection.rect = newRect;
		selection.mask = std::move(newMask);
		dropMaskIfFullySelected(selection.rect, selection.mask);
	}
	selection.mergeBackupHadSelection = false;
	selection.mergeBackupRect = {};
	selection.mergeBackupMask = {};
}

bbe::Rectanglei PaintEditor::buildRawRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2) const
{
	const int32_t left = bbe::Math::min(pos1.x, pos2.x);
	const int32_t top = bbe::Math::min(pos1.y, pos2.y);
	const int32_t right = bbe::Math::max(pos1.x, pos2.x);
	const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
	return bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1);
}

bool PaintEditor::isPointInSelection(const bbe::Vector2i &point) const
{
	return selection.hasSelection && regionPixelOn(selection.rect, selection.mask, point.x, point.y);
}

bool PaintEditor::isSelectionResizeHit(const SelectionHitZone hitZone) const { return bbe::editor::isResizeZone((bbe::editor::RectSelectionHitZone)hitZone); }

PaintEditor::SelectionHitZone PaintEditor::getSelectionHitZone(const bbe::Vector2 &pointCanvas) const
{
	if (!selection.hasSelection)
	{
		return SelectionHitZone::NONE;
	}
	const bool allowRotationHandle = !selection.dragActive && !selection.lassoDragActive && selection.polygonLassoVertices.empty();
	const float slopCanvas = 6.f / zoomLevel;
	const float rotationStemLenCanvas = 30.f / zoomLevel;
	const float rotationHitRadiusCanvas = 8.f / zoomLevel;
	const SelectionHitZone z = (SelectionHitZone)bbe::editor::hitTestCanvas(selection.rect, pointCanvas, slopCanvas, allowRotationHandle, rotationStemLenCanvas, rotationHitRadiusCanvas);
	const int32_t maskX = (int32_t)bbe::Math::floor(pointCanvas.x);
	const int32_t maskY = (int32_t)bbe::Math::floor(pointCanvas.y);
	if (z == SelectionHitZone::INSIDE && hasSelectionPixelMask() && !regionPixelOn(selection.rect, selection.mask, maskX, maskY))
	{
		return SelectionHitZone::NONE;
	}
	return z;
}

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
	constexpr bbe::Colori kClear{0, 0, 0, 0};
	for (int32_t x = 0; x < rect.width; x++)
	{
		for (int32_t y = 0; y < rect.height; y++)
		{
			getActiveLayerImage().setPixel((size_t)(rect.x + x), (size_t)(rect.y + y), kClear);
		}
	}
}

void PaintEditor::storeSelectionInClipboard()
{
	if (!selection.hasSelection) return;
	if (selection.floating)
	{
		selection.clipboard = selection.floatingImage;
	}
	else if (hasSelectionPixelMask())
	{
		selection.clipboard = copyLayerRectWithMask(*this, selection.rect, selection.mask);
	}
	else
	{
		selection.clipboard = copyCanvasRect(selection.rect);
	}
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
	if (hasSelectionPixelMask())
	{
		clearLayerRectWithMask(*this, selection.rect, selection.mask);
	}
	else
	{
		clearCanvasRect(selection.rect);
	}
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
		const bbe::Color fillColor(0.f, 0.f, 0.f, 0.f);
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
		{
			if (mode == MODE_ELLIPSE_SELECTION)
			{
				finishEllipseMarqueeDrag(toCanvasPixel(lastPointerCanvasPos));
			}
			else
			{
				selection.hasSelection = buildSelectionRect(selection.dragStart, toCanvasPixel(lastPointerCanvasPos), selection.rect);
			}
		}
		selection.dragActive = false;
		selection.previewRect = {};
	}

	if (selection.lassoDragActive)
	{
		if (hasPointerPos)
		{
			finishLassoDrag(lastPointerCanvasPos);
		}
		else
		{
			selection.lassoDragActive = false;
			selection.lassoPath.clear();
			if (selection.mergeBackupHadSelection && selectionAdditiveModifier)
			{
				selection.rect = selection.mergeBackupRect;
				selection.mask = std::move(selection.mergeBackupMask);
				selection.hasSelection = selection.mergeBackupHadSelection;
			}
			selection.mergeBackupHadSelection = false;
			selection.mergeBackupRect = {};
			selection.mergeBackupMask = {};
		}
	}

	if (!selection.polygonLassoVertices.empty())
	{
		if (selection.polygonLassoVertices.size() >= 3)
		{
			finishPolygonLassoSelection();
		}
		else
		{
			cancelPolygonLassoDraft();
		}
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

namespace {

constexpr float kPi = 3.14159265f;

void pushRingVertex(std::vector<bbe::Vector2> &ring, const bbe::Vector2 &v)
{
	if (!ring.empty() && (v - ring.back()).getLengthSq() < 1e-8f) return;
	ring.push_back(v);
}

void appendLineToRing(std::vector<bbe::Vector2> &ring, const bbe::Vector2 &a, const bbe::Vector2 &b, float stepPx)
{
	const float len = (b - a).getLength();
	if (len < 1e-6f)
	{
		pushRingVertex(ring, a);
		return;
	}
	const int n = bbe::Math::max(1, (int)std::ceil((double)len / (double)stepPx));
	for (int i = 0; i <= n; ++i)
	{
		const float t = (float)i / (float)n;
		pushRingVertex(ring, a + (b - a) * t);
	}
}

void appendArcToRing(std::vector<bbe::Vector2> &ring, const bbe::Vector2 &c, float r, float a0, float a1, float stepPx)
{
	const float arcLen = std::abs(a1 - a0) * r;
	const int n = bbe::Math::max(2, (int)std::ceil((double)arcLen / (double)stepPx));
	for (int i = 0; i <= n; ++i)
	{
		const float t = (float)i / (float)n;
		const float ang = a0 + (a1 - a0) * t;
		pushRingVertex(ring, c + bbe::Vector2(std::cos(ang), std::sin(ang)) * r);
	}
}

void buildRoundedRectOutlineRing(float width, float height, float cornerR, float stepPx, std::vector<bbe::Vector2> &ring)
{
	ring.clear();
	if (width <= 0.f || height <= 0.f) return;
	const float hw = width * 0.5f;
	const float hh = height * 0.5f;
	float R = bbe::Math::max(0.f, cornerR);
	R = bbe::Math::min(R, bbe::Math::min(hw, hh));
	if (R < 1e-4f)
	{
		appendLineToRing(ring, { 0.f, -hh }, { hw, -hh }, stepPx);
		appendLineToRing(ring, { hw, -hh }, { hw, hh }, stepPx);
		appendLineToRing(ring, { hw, hh }, { -hw, hh }, stepPx);
		appendLineToRing(ring, { -hw, hh }, { -hw, -hh }, stepPx);
		appendLineToRing(ring, { -hw, -hh }, { 0.f, -hh }, stepPx);
	}
	else
	{
		appendLineToRing(ring, { 0.f, -hh }, { hw - R, -hh }, stepPx);
		appendArcToRing(ring, { hw - R, -hh + R }, R, -kPi * 0.5f, 0.f, stepPx);
		appendLineToRing(ring, { hw, -hh + R }, { hw, hh - R }, stepPx);
		appendArcToRing(ring, { hw - R, hh - R }, R, 0.f, kPi * 0.5f, stepPx);
		appendLineToRing(ring, { hw - R, hh }, { -hw + R, hh }, stepPx);
		appendArcToRing(ring, { -hw + R, hh - R }, R, kPi * 0.5f, kPi, stepPx);
		appendLineToRing(ring, { -hw, hh - R }, { -hw, -hh + R }, stepPx);
		appendArcToRing(ring, { -hw + R, -hh + R }, R, kPi, kPi * 1.5f, stepPx);
		appendLineToRing(ring, { -hw + R, -hh }, { 0.f, -hh }, stepPx);
	}
	if (ring.size() >= 2 && (ring.back() - ring.front()).getLengthSq() < 1e-5f)
	{
		ring.pop_back();
	}
}

void buildEllipseOutlineRing(float rx, float ry, float stepPx, std::vector<bbe::Vector2> &ring)
{
	ring.clear();
	if (rx <= 0.f || ry <= 0.f) return;
	const float circApprox = 2.f * kPi * std::sqrt(0.5f * (rx * rx + ry * ry));
	const int n = bbe::Math::max(64, (int)std::ceil((double)circApprox / (double)stepPx));
	for (int i = 0; i < n; ++i)
	{
		const float t = (float)i / (float)n;
		const float theta = -kPi * 0.5f + t * (2.f * kPi);
		ring.push_back({ rx * std::cos(theta), ry * std::sin(theta) });
	}
}

float closedPolylinePerimeter(const std::vector<bbe::Vector2> &v)
{
	const size_t nv = v.size();
	if (nv < 2) return 0.f;
	float p = 0.f;
	for (size_t i = 0; i < nv; ++i)
		p += (v[(i + 1) % nv] - v[i]).getLength();
	return p;
}

bbe::Vector2 pointOnClosedPolyline(const std::vector<bbe::Vector2> &v, float s)
{
	const size_t nv = v.size();
	if (nv < 2) return nv ? v[0] : bbe::Vector2();
	const float P = closedPolylinePerimeter(v);
	if (P < 1e-6f) return v[0];
	float t = std::fmod(s, P);
	if (t < 0.f) t += P;
	for (size_t i = 0; i < nv; ++i)
	{
		const bbe::Vector2 &a = v[i];
		const bbe::Vector2 &b = v[(i + 1) % nv];
		const float len = (b - a).getLength();
		if (i == nv - 1)
		{
			const float u = len > 1e-6f ? bbe::Math::clamp01(t / len) : 0.f;
			return a + (b - a) * u;
		}
		if (t <= len + 1e-4f)
		{
			const float u = len > 1e-6f ? t / len : 0.f;
			return a + (b - a) * bbe::Math::clamp01(u);
		}
		t -= len;
	}
	return v[0];
}

void mapRingToBmp(
	const std::vector<bbe::Vector2> &ringLocal,
	std::vector<bbe::Vector2> &outBmp,
	bool expandedBb,
	float bbHalfW,
	float bbHalfH,
	float rotation)
{
	outBmp.clear();
	outBmp.reserve(ringLocal.size());
	const float cosA = std::cos(rotation);
	const float sinA = std::sin(rotation);
	for (const bbe::Vector2 &loc : ringLocal)
	{
		if (expandedBb)
		{
			const float bcx = loc.x * cosA - loc.y * sinA;
			const float bcy = loc.x * sinA + loc.y * cosA;
			outBmp.push_back({ bcx + bbHalfW, bcy + bbHalfH });
		}
		else
			outBmp.push_back({ loc.x + bbHalfW, loc.y + bbHalfH });
	}
}

/// Closest point on closed polyline to `p`; returns squared distance and arc length from first vertex along the loop to that closest point.
void closestPointOnClosedPolylineDistSqAndS(const std::vector<bbe::Vector2> &v, const bbe::Vector2 &p, float &outDistSq, float &outS)
{
	const size_t n = v.size();
	if (n < 2)
	{
		outDistSq = 1e30f;
		outS = 0.f;
		return;
	}
	float bestD2 = 1e30f;
	float bestS = 0.f;
	float cum = 0.f;
	for (size_t i = 0; i < n; ++i)
	{
		const bbe::Vector2 &a = v[i];
		const bbe::Vector2 &b = v[(i + 1) % n];
		const bbe::Vector2 ab = b - a;
		const float lenSq = ab.getLengthSq();
		float t = 0.f;
		bbe::Vector2 q;
		if (lenSq < 1e-12f)
		{
			q = a;
		}
		else
		{
			t = ((p - a) * ab) / lenSq;
			t = bbe::Math::clamp01(t);
			q = a + ab * t;
		}
		const float d2 = (p - q).getLengthSq();
		const float segLen = std::sqrt(lenSq < 1e-12f ? 0.f : lenSq);
		const float sAt = cum + t * segLen;
		if (d2 < bestD2)
		{
			bestD2 = d2;
			bestS = sAt;
		}
		cum += segLen;
	}
	outDistSq = bestD2;
	outS = bestS;
}

bbe::Image compositeSolidWithSeamlessStripesAlongMidline(
	const bbe::Image &solid,
	const std::vector<bbe::Vector2> &polyBmp,
	float strokeWidthF,
	float stripeNominalPeriodPx)
{
	const int32_t imgW = (int32_t)solid.getWidth();
	const int32_t imgH = (int32_t)solid.getHeight();
	if (imgW <= 0 || imgH <= 0 || polyBmp.size() < 2) return {};

	const float P = closedPolylinePerimeter(polyBmp);
	if (P < 1e-4f) return {};

	const float periodTarget = bbe::Math::max(stripeNominalPeriodPx, 4.f);
	const int nPeriods = bbe::Math::max(1, (int)std::lround((double)P / (double)periodTarget));
	const float period = P / (float)nPeriods;
	const float dashLen = period * 0.5f;

	// Midline approximation vs true SDF offset: allow slack so outer annulus pixels are not dropped.
	const float bandR = strokeWidthF * 0.5f + 2.5f;
	const float bandR2 = bandR * bandR;

	bbe::Image out(imgW, imgH, bbe::Color(0.f, 0.f, 0.f, 0.f));
	for (int32_t py = 0; py < imgH; ++py)
	{
		for (int32_t px = 0; px < imgW; ++px)
		{
			const bbe::Colori s = solid.getPixel((size_t)px, (size_t)py);
			if (s.a == 0) continue;

			float d2 = 0.f;
			float arcS = 0.f;
			closestPointOnClosedPolylineDistSqAndS(polyBmp, { (float)px + 0.5f, (float)py + 0.5f }, d2, arcS);
			if (d2 > bandR2) continue;

			float ph = std::fmod(arcS, period);
			if (ph < 0.f) ph += period;
			if (ph < dashLen) out.setPixel((size_t)px, (size_t)py, s);
		}
	}
	return out;
}

/// Capsule rasterization with hard coverage (matches strokedRoundedRect non-AA thresholding: no grey fringe from soft brush stamps).
void drawLineCapsuleOpaque(bbe::Image &img, const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color, int32_t brushRadius)
{
	if (brushRadius < 0 || !img.isLoadedCpu()) return;
	const int32_t w = (int32_t)img.getWidth();
	const int32_t h = (int32_t)img.getHeight();
	if (w <= 0 || h <= 0) return;

	const float ax = to.x - from.x;
	const float ay = to.y - from.y;
	const float lenSq = ax * ax + ay * ay;

	const float margin = (float)brushRadius + 1.f;
	const int32_t xMin = (int32_t)std::floor(std::min(from.x, to.x) - margin);
	const int32_t xMax = (int32_t)std::ceil(std::max(from.x, to.x) + margin);
	const int32_t yMin = (int32_t)std::floor(std::min(from.y, to.y) - margin);
	const int32_t yMax = (int32_t)std::ceil(std::max(from.y, to.y) + margin);

	for (int32_t y = yMin; y <= yMax; y++)
	{
		for (int32_t x = xMin; x <= xMax; x++)
		{
			if (x < 0 || y < 0 || x >= w || y >= h) continue;

			const float pcx = (float)x + 0.5f;
			const float pcy = (float)y + 0.5f;
			float dist;
			if (lenSq < 1e-6f)
			{
				const float dx = pcx - from.x;
				const float dy = pcy - from.y;
				dist = bbe::Math::sqrt(dx * dx + dy * dy);
			}
			else
			{
				const float bx = pcx - from.x;
				const float by = pcy - from.y;
				const float t = bbe::Math::clamp01((bx * ax + by * ay) / lenSq);
				const float cx = pcx - (from.x + t * ax);
				const float cy = pcy - (from.y + t * ay);
				dist = bbe::Math::sqrt(cx * cx + cy * cy);
			}

			const float strength = bbe::Math::clamp01((float)brushRadius - dist);
			// Use >= 0.5: strength == 0.5 hits often on axis-aligned edges (e.g. brush 1), and strict > drew nothing.
			if (strength < 0.5f) continue;

			img.setPixel((size_t)x, (size_t)y, color);
		}
	}
}

void drawSeamlessStripedClosedLoop(
	bbe::Image &img,
	const std::vector<bbe::Vector2> &ring,
	const bbe::Colori &color,
	int32_t brushRadius,
	float desiredPeriod,
	float dashFraction,
	bool antiAlias,
	bool expandedBb,
	float bbHalfW,
	float bbHalfH,
	float rotation,
	bool forceOpaqueCapsules,
	bool axisAlignedSquareCapsules)
{
	const float P = closedPolylinePerimeter(ring);
	if (P < 1e-4f || brushRadius < 0) return;
	const float periodTarget = bbe::Math::max(desiredPeriod, 4.f);
	const int nPeriods = bbe::Math::max(1, (int)std::lround((double)P / (double)periodTarget));
	const float period = P / (float)nPeriods;
	const float dashLen = period * bbe::Math::clamp01(dashFraction);
	const float step = antiAlias ? 0.55f : 0.9f;
	const int N = bbe::Math::max(24, (int)std::ceil((double)P / (double)step));

	const float cosA = std::cos(rotation);
	const float sinA = std::sin(rotation);

	auto toBmp = [&](const bbe::Vector2 &loc) -> bbe::Vector2 {
		if (expandedBb)
		{
			const float bcx = loc.x * cosA - loc.y * sinA;
			const float bcy = loc.x * sinA + loc.y * cosA;
			return { bcx + bbHalfW, bcy + bbHalfH };
		}
		return { loc.x + bbHalfW, loc.y + bbHalfH };
	};

	bool haveLast = false;
	bbe::Vector2 lastBmp;
	for (int i = 0; i <= N; ++i)
	{
		const bool isClosure = (i == N);
		const float sGeom = isClosure ? 0.f : (P * (float)i / (float)N);
		const float sPhase = isClosure ? (P - 1e-4f) : (P * (float)i / (float)N);
		float phase = std::fmod(sPhase, period);
		if (phase < 0.f) phase += period;
		const bool on = phase < dashLen;

		const bbe::Vector2 bmp = toBmp(pointOnClosedPolyline(ring, sGeom));

		if (on)
		{
			if (haveLast)
			{
				// Square stamps stay axis-aligned: on H/V sides the stroke band is rectangular. Circles scallop inward (round spots) when stroked thick.
				if (axisAlignedSquareCapsules)
				{
					if (antiAlias && !forceOpaqueCapsules)
						img.drawLineCapsule(lastBmp, bmp, color, brushRadius, bbe::ImageBrushShape::Square, false, true);
					else
						img.drawLineCapsule(lastBmp, bmp, color, brushRadius, bbe::ImageBrushShape::Square, false, false);
				}
				else if (antiAlias && !forceOpaqueCapsules)
					img.drawLineCapsule(lastBmp, bmp, color, brushRadius, bbe::ImageBrushShape::Circle, false, antiAlias);
				else
					drawLineCapsuleOpaque(img, lastBmp, bmp, color, brushRadius);
			}
			lastBmp = bmp;
			haveLast = true;
		}
		else
		{
			haveLast = false;
		}
	}
}

bbe::Image paintStripedRoundedRectOutline(
	int32_t width,
	int32_t height,
	int32_t strokeWidth,
	int32_t cornerRadius,
	const bbe::Colori &color,
	float rotation,
	bool antiAlias,
	float stripeNominalPeriodPx)
{
	if (width <= 0 || height <= 0) return {};
	const float hw = width * 0.5f;
	const float hh = height * 0.5f;
	const bool expandedBb = !antiAlias && std::abs(rotation) > 0.01f;
	float bbHalfW = hw;
	float bbHalfH = hh;
	int32_t imgW = width;
	int32_t imgH = height;
	float rotUse = rotation;
	if (expandedBb)
	{
		const float cosA = std::cos(rotation);
		const float sinA = std::sin(rotation);
		const float newHW = std::abs(hw * cosA) + std::abs(hh * sinA);
		const float newHH = std::abs(hw * sinA) + std::abs(hh * cosA);
		imgW = (int32_t)std::ceil(newHW * 2.f);
		imgH = (int32_t)std::ceil(newHH * 2.f);
		bbHalfW = imgW * 0.5f;
		bbHalfH = imgH * 0.5f;
	}

	const float stepPx = antiAlias ? 0.55f : 0.95f;
	const float T = (float)strokeWidth;
	const float wMid = (float)width - T;
	const float hMid = (float)height - T;
	if (wMid < 1.f || hMid < 1.f) return {};

	// Mid-stroke guide (offset ~T/2 from outer): matches the SDF annulus; outer ring sat on bbox edges and clipped.
	const float inset = T * 0.5f;
	const float cornerMid = bbe::Math::max(0.f, (float)cornerRadius - inset);
	std::vector<bbe::Vector2> ring;
	buildRoundedRectOutlineRing(wMid, hMid, cornerMid, stepPx, ring);
	if (ring.size() < 2) return {};

	bbe::Image solid = bbe::Image::strokedRoundedRect(width, height, color, strokeWidth, cornerRadius, rotation, antiAlias);
	if ((int32_t)solid.getWidth() != imgW || (int32_t)solid.getHeight() != imgH)
	{
		std::vector<bbe::Vector2> ringOuter;
		buildRoundedRectOutlineRing((float)width, (float)height, (float)cornerRadius, stepPx, ringOuter);
		bbe::Image img(imgW, imgH, bbe::Color(0.f, 0.f, 0.f, 0.f));
		drawSeamlessStripedClosedLoop(
			img,
			ringOuter,
			color,
			strokeWidth,
			stripeNominalPeriodPx,
			0.5f,
			antiAlias,
			expandedBb,
			bbHalfW,
			bbHalfH,
			rotUse,
			false,
			true);
		return img;
	}

	std::vector<bbe::Vector2> polyBmp;
	mapRingToBmp(ring, polyBmp, expandedBb, bbHalfW, bbHalfH, rotUse);
	return compositeSolidWithSeamlessStripesAlongMidline(solid, polyBmp, T, stripeNominalPeriodPx);
}

bbe::Image paintStripedEllipseOutline(
	int32_t width,
	int32_t height,
	int32_t strokeWidth,
	const bbe::Colori &color,
	float rotation,
	bool antiAlias,
	float stripeNominalPeriodPx)
{
	if (width <= 0 || height <= 0) return {};
	const float rx = width * 0.5f;
	const float ry = height * 0.5f;
	const bool expandedBb = !antiAlias && std::abs(rotation) > 0.01f;
	float bbHalfW = rx;
	float bbHalfH = ry;
	int32_t imgW = width;
	int32_t imgH = height;
	if (expandedBb)
	{
		const float cosA = std::cos(rotation);
		const float sinA = std::sin(rotation);
		const float newHW = std::abs(rx * cosA) + std::abs(ry * sinA);
		const float newHH = std::abs(rx * sinA) + std::abs(ry * cosA);
		imgW = (int32_t)std::ceil(newHW * 2.f);
		imgH = (int32_t)std::ceil(newHH * 2.f);
		bbHalfW = imgW * 0.5f;
		bbHalfH = imgH * 0.5f;
	}

	const float stepPx = antiAlias ? 0.55f : 0.95f;
	const float T = (float)strokeWidth;
	const float rxMid = bbe::Math::max(0.5f, rx - T * 0.5f);
	const float ryMid = bbe::Math::max(0.5f, ry - T * 0.5f);
	std::vector<bbe::Vector2> ring;
	buildEllipseOutlineRing(rxMid, ryMid, stepPx, ring);
	if (ring.size() < 2) return {};

	bbe::Image solid = bbe::Image::strokedEllipse(width, height, color, strokeWidth, rotation, antiAlias);
	if ((int32_t)solid.getWidth() != imgW || (int32_t)solid.getHeight() != imgH)
	{
		std::vector<bbe::Vector2> ringOuter;
		buildEllipseOutlineRing(rx, ry, stepPx, ringOuter);
		bbe::Image img(imgW, imgH, bbe::Color(0.f, 0.f, 0.f, 0.f));
		drawSeamlessStripedClosedLoop(
			img,
			ringOuter,
			color,
			strokeWidth,
			stripeNominalPeriodPx,
			0.5f,
			antiAlias,
			expandedBb,
			bbHalfW,
			bbHalfH,
			rotation,
			false,
			false);
		return img;
	}

	std::vector<bbe::Vector2> polyBmp;
	mapRingToBmp(ring, polyBmp, expandedBb, bbHalfW, bbHalfH, rotation);
	return compositeSolidWithSeamlessStripesAlongMidline(solid, polyBmp, T, stripeNominalPeriodPx);
}

} // namespace

bbe::Image PaintEditor::createRectangleImage(int32_t width, int32_t height, const bbe::Colori &strokeColor, float rotation, bool strokeUsesRightColor) const
{
	const auto solidStroke = [&]() {
		return bbe::Image::strokedRoundedRect(width, height, strokeColor, brushWidth, cornerRadius, rotation, antiAliasingEnabled);
	};
	const float stripePeriod = (float)bbe::Math::clamp(shapeStripePeriodPx, 4, 512);
	const auto stripedStroke = [&]() {
		return paintStripedRoundedRectOutline(width, height, brushWidth, cornerRadius, strokeColor, rotation, antiAliasingEnabled, stripePeriod);
	};

	if (shapeFillWithSecondary)
	{
		const bbe::Colori fillColor = getColor(!strokeUsesRightColor);
		bbe::Image img = bbe::Image::strokedRoundedRect(width, height, fillColor, 0, cornerRadius, rotation, antiAliasingEnabled);
		bbe::Image stroke = shapeStripedStroke ? stripedStroke() : solidStroke();
		img.blend(stroke, 1.0f, bbe::BlendMode::Normal);
		return img;
	}
	if (shapeStripedStroke) return stripedStroke();
	return solidStroke();
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
	const auto solidStroke = [&]() {
		return bbe::Image::strokedEllipse(width, height, strokeColor, brushWidth, rotation, antiAliasingEnabled);
	};
	const float stripePeriod = (float)bbe::Math::clamp(shapeStripePeriodPx, 4, 512);
	const auto stripedStroke = [&]() {
		return paintStripedEllipseOutline(width, height, brushWidth, strokeColor, rotation, antiAliasingEnabled, stripePeriod);
	};

	if (shapeFillWithSecondary)
	{
		const bbe::Colori fillColor = getColor(!strokeUsesRightColor);
		bbe::Image img = bbe::Image::strokedEllipse(width, height, fillColor, 0, rotation, antiAliasingEnabled);
		bbe::Image stroke = shapeStripedStroke ? stripedStroke() : solidStroke();
		img.blend(stroke, 1.0f, bbe::BlendMode::Normal);
		return img;
	}
	if (shapeStripedStroke) return stripedStroke();
	return solidStroke();
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

void PaintEditor::pointerDownSelectionDefaultMarqueePath(const bbe::Vector2i &mousePixel)
{
	const bool hadFloating = selection.floating;
	const bool hadMarquee = selection.hasSelection;

	if (selectionAdditiveModifier)
	{
		selection.mergeBackupHadSelection = hadMarquee;
		if (hadMarquee)
		{
			selection.mergeBackupRect = selection.rect;
			selection.mergeBackupMask = selection.mask;
		}
		else
		{
			selection.mergeBackupMask = {};
		}
	}
	else
	{
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupMask = {};
	}

	if (selection.floating)
	{
		commitFloatingSelection();
	}

	if (selectionAdditiveModifier)
	{
		selection.dragActive = true;
		selection.dragStart = mousePixel;
		selection.hasSelection = false;
		selection.rect = {};
		selection.previewRect = {};
		selection.mask = {};
	}
	else if (hadMarquee || hadFloating)
	{
		// Outside click: commit already ran for floating; clear marquee in one press (previously
		// hadFloating skipped the hadMarquee && !hadFloating branch, requiring a second click).
		clearMarqueePreservingClipboard();
	}
	else
	{
		selection.dragActive = true;
		selection.dragStart = mousePixel;
		selection.hasSelection = false;
		selection.rect = {};
		selection.previewRect = {};
		selection.mask = {};
	}
}

void PaintEditor::pointerDownLassoMarqueePath(const bbe::Vector2 &canvasPos)
{
	const bool hadFloating = selection.floating;
	const bool hadMarquee = selection.hasSelection;

	if (selectionAdditiveModifier)
	{
		selection.mergeBackupHadSelection = hadMarquee;
		if (hadMarquee)
		{
			selection.mergeBackupRect = selection.rect;
			selection.mergeBackupMask = selection.mask;
		}
		else
		{
			selection.mergeBackupMask = {};
		}
	}
	else
	{
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupMask = {};
	}

	if (selection.floating)
	{
		commitFloatingSelection();
	}

	selection.dragActive = false;
	if (selectionAdditiveModifier)
	{
		selection.lassoDragActive = true;
		selection.lassoPath.clear();
		appendLassoPoint(canvasPos);
		selection.hasSelection = false;
		selection.rect = {};
		selection.previewRect = {};
		selection.mask = {};
	}
	else if (hadMarquee || hadFloating)
	{
		clearMarqueePreservingClipboard();
	}
	else
	{
		selection.lassoDragActive = true;
		selection.lassoPath.clear();
		appendLassoPoint(canvasPos);
		selection.hasSelection = false;
		selection.rect = {};
		selection.previewRect = {};
		selection.mask = {};
	}
}

void PaintEditor::appendLassoPoint(const bbe::Vector2 &p)
{
	const int32_t W = getCanvasWidth();
	const int32_t H = getCanvasHeight();
	if (W <= 0 || H <= 0) return;
	const bbe::Vector2 c(
		bbe::Math::clamp(p.x, 0.f, (float)W),
		bbe::Math::clamp(p.y, 0.f, (float)H));
	if (!selection.lassoPath.empty() && (selection.lassoPath.back() - c).getLength() < 1e-5f) return;
	selection.lassoPath.push_back(c);
}

void PaintEditor::commitSelectionFromClosedLassoPath(std::vector<bbe::Vector2> path)
{
	const bool merge = selection.mergeBackupHadSelection && selectionAdditiveModifier;
	bbe::Rectanglei newRect;
	bbe::Image newMask;
	const bool ok = buildLassoSelectionMask(*this, path, newRect, newMask);

	if (!ok)
	{
		if (merge)
		{
			selection.rect = selection.mergeBackupRect;
			selection.mask = std::move(selection.mergeBackupMask);
			selection.hasSelection = selection.mergeBackupHadSelection;
		}
		else
		{
			selection.hasSelection = false;
			selection.rect = {};
			selection.mask = {};
		}
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupRect = {};
		selection.mergeBackupMask = {};
		return;
	}

	prepareImageForCanvas(newMask);
	dropMaskIfFullySelected(newRect, newMask);

	if (merge)
	{
		bbe::Rectanglei outR;
		bbe::Image outM;
		unionSelectionRegions(selection.mergeBackupRect, selection.mergeBackupMask, newRect, newMask, outR, outM);
		prepareImageForCanvas(outM);
		selection.rect = outR;
		selection.mask = std::move(outM);
		selection.hasSelection = outR.width > 0 && outR.height > 0;
		dropMaskIfFullySelected(selection.rect, selection.mask);
	}
	else
	{
		selection.hasSelection = true;
		selection.rect = newRect;
		selection.mask = std::move(newMask);
		dropMaskIfFullySelected(selection.rect, selection.mask);
	}
	selection.mergeBackupHadSelection = false;
	selection.mergeBackupRect = {};
	selection.mergeBackupMask = {};
}

void PaintEditor::finishLassoDrag(const bbe::Vector2 &canvasPos)
{
	appendLassoPoint(canvasPos);
	std::vector<bbe::Vector2> path = std::move(selection.lassoPath);
	selection.lassoPath.clear();
	selection.lassoDragActive = false;
	commitSelectionFromClosedLassoPath(std::move(path));
}

void PaintEditor::pointerDownPolygonLassoMarqueePath(const bbe::Vector2i &mousePixel)
{
	const bool hadFloating = selection.floating;
	const bool hadMarquee = selection.hasSelection;

	if (selectionAdditiveModifier)
	{
		selection.mergeBackupHadSelection = hadMarquee;
		if (hadMarquee)
		{
			selection.mergeBackupRect = selection.rect;
			selection.mergeBackupMask = selection.mask;
		}
		else
		{
			selection.mergeBackupMask = {};
		}
	}
	else
	{
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupMask = {};
	}

	if (selection.floating)
	{
		commitFloatingSelection();
	}

	selection.dragActive = false;
	selection.lassoDragActive = false;
	selection.lassoPath.clear();

	if (selectionAdditiveModifier)
	{
		selection.polygonLassoVertices.clear();
		appendPolygonLassoVertex(mousePixel);
		selection.hasSelection = false;
		selection.rect = {};
		selection.previewRect = {};
		selection.mask = {};
	}
	else if (hadMarquee || hadFloating)
	{
		clearMarqueePreservingClipboard();
	}
	else
	{
		selection.polygonLassoVertices.clear();
		appendPolygonLassoVertex(mousePixel);
		selection.hasSelection = false;
		selection.rect = {};
		selection.previewRect = {};
		selection.mask = {};
	}
}

void PaintEditor::appendPolygonLassoVertex(const bbe::Vector2i &p)
{
	const int32_t W = getCanvasWidth();
	const int32_t H = getCanvasHeight();
	if (W <= 0 || H <= 0) return;
	const bbe::Vector2i c(
		bbe::Math::clamp(p.x, 0, W),
		bbe::Math::clamp(p.y, 0, H));
	if (!selection.polygonLassoVertices.empty() && selection.polygonLassoVertices.back().x == c.x && selection.polygonLassoVertices.back().y == c.y) return;
	selection.polygonLassoVertices.push_back(c);
}

bool PaintEditor::isClickClosePolygonLasso(const bbe::Vector2 &canvasPos) const
{
	if (selection.polygonLassoVertices.size() < 3) return false;
	const bbe::Vector2 first((float)selection.polygonLassoVertices[0].x + 0.5f, (float)selection.polygonLassoVertices[0].y + 0.5f);
	const float slop = 8.f / zoomLevel;
	return (canvasPos - first).getLength() <= slop;
}

void PaintEditor::finishPolygonLassoSelection()
{
	if (selection.polygonLassoVertices.size() < 3) return;
	const int32_t W = getCanvasWidth();
	const int32_t H = getCanvasHeight();
	if (W <= 0 || H <= 0) return;
	std::vector<bbe::Vector2> path;
	path.reserve(selection.polygonLassoVertices.size());
	for (const bbe::Vector2i &v : selection.polygonLassoVertices)
	{
		// Pixel centers + same clamp as appendLassoPoint so geometry matches the preview and freehand lasso.
		path.push_back({
			bbe::Math::clamp((float)v.x + 0.5f, 0.f, (float)W),
			bbe::Math::clamp((float)v.y + 0.5f, 0.f, (float)H)
		});
	}
	selection.polygonLassoVertices.clear();
	commitSelectionFromClosedLassoPath(std::move(path));
}

void PaintEditor::cancelPolygonLassoDraft()
{
	selection.polygonLassoVertices.clear();
	if (selection.mergeBackupHadSelection)
	{
		selection.rect = selection.mergeBackupRect;
		selection.mask = std::move(selection.mergeBackupMask);
		selection.hasSelection = selection.mergeBackupHadSelection;
	}
	selection.mergeBackupHadSelection = false;
	selection.mergeBackupRect = {};
	selection.mergeBackupMask = {};
}

void PaintEditor::polygonLassoBackspace()
{
	if (selection.polygonLassoVertices.empty()) return;
	selection.polygonLassoVertices.pop_back();
	if (selection.polygonLassoVertices.empty() && selection.mergeBackupHadSelection)
	{
		selection.rect = selection.mergeBackupRect;
		selection.mask = std::move(selection.mergeBackupMask);
		selection.hasSelection = selection.mergeBackupHadSelection;
		selection.mergeBackupHadSelection = false;
		selection.mergeBackupRect = {};
		selection.mergeBackupMask = {};
	}
}

void PaintEditor::confirmPolygonLassoIfReady()
{
	if (mode != MODE_POLYGON_LASSO) return;
	if (selection.polygonLassoVertices.size() >= 3)
	{
		finishPolygonLassoSelection();
	}
}

void PaintEditor::liftSelectionToFloatingIfNeeded()
{
	if (selection.floating || !selection.hasSelection) return;
	if (selection.rect.width <= 0 || selection.rect.height <= 0) return;

	bbe::Image lifted = hasSelectionPixelMask() ? copyLayerRectWithMask(*this, selection.rect, selection.mask) : copyCanvasRect(selection.rect);
	prepareImageForCanvas(lifted);
	if (hasSelectionPixelMask())
		clearLayerRectWithMask(*this, selection.rect, selection.mask);
	else
		clearCanvasRect(selection.rect);
	selection.floating = true;
	selection.floatingImage = std::move(lifted);
	prepareImageForCanvas(selection.floatingImage);
	selection.mask = {};
	submitCanvas();
}

void PaintEditor::beginSelectionMove(const bbe::Vector2i &mousePixel)
{
	liftSelectionToFloatingIfNeeded();

	selection.moveActive = true;
	selection.moveOffset = mousePixel - selection.rect.getPos();
	selection.interactionStartRect = selection.rect;
	selection.previewRect = selection.rect;
	selection.previewImage = selection.floatingImage;
	// Must retain CPU pixels after GPU draw (see OpenGLImage ctor); blendOver needs isLoadedCpu().
	prepareImageForCanvas(selection.previewImage);
}

void PaintEditor::beginRotationDrag(const bbe::Vector2i &mousePixel)
{
	liftSelectionToFloatingIfNeeded();

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
	liftSelectionToFloatingIfNeeded();

	selection.resizeActive = true;
	selection.resizeZone = hitZone;
	selection.interactionStartRect = selection.rect;
	selection.previewRect = selection.rect;
	selection.previewImage = selection.floatingImage;
	prepareImageForCanvas(selection.previewImage);
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
			selection.mask = {};
		}
		clearSelectionInteractionState();
		return;
	}

	if (rectChanged)
	{
		if (hasSelectionPixelMask())
		{
			clearLayerRectWithMask(*this, selection.rect, selection.mask);
		}
		else
		{
			clearCanvasRect(selection.rect);
		}
		selection.rect = selection.previewRect;
		selection.floating = true;
		selection.floatingImage = buildSelectionPreviewResultImage();
		prepareImageForCanvas(selection.floatingImage);
		selection.mask = {};
		rectangle.draftActive = false;
		rectangle.draftUsesRightColor = false;
		circle.draftActive = false;
		circle.draftUsesRightColor = false;
		selection.hasSelection = true;
	}

	clearSelectionInteractionState();
}

void PaintEditor::nudgeSelectionByPixels(int32_t dx, int32_t dy)
{
	if ((dx | dy) == 0) return;
	if (!selection.hasSelection) return;
	if (selection.dragActive || selection.lassoDragActive) return;
	if (selection.moveActive || selection.resizeActive || selection.rotationHandleActive) return;
	if (!selection.polygonLassoVertices.empty()) return;

	liftSelectionToFloatingIfNeeded();

	if (selection.rect.width <= 0 || selection.rect.height <= 0) return;

	// Match mouse move: allow the rect to extend past the canvas; commit crops via clampRectToCanvas.
	selection.rect.x += dx;
	selection.rect.y += dy;
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
	if (selection.hasSelection) return -1;
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

	const bbe::Color fillColor(0.f, 0.f, 0.f, 0.f);

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

void PaintEditor::clampEraserSize()
{
	eraserSize = bbe::Math::clamp(eraserSize, 1, 512);
}

void PaintEditor::clampSprayWidth()
{
	sprayWidth = bbe::Math::clamp(sprayWidth, 1, 512);
}

void PaintEditor::clampSprayDensity()
{
	sprayDensity = bbe::Math::clamp(sprayDensity, 1, 256);
}

void PaintEditor::clampShapeStripePeriod()
{
	shapeStripePeriodPx = bbe::Math::clamp(shapeStripePeriodPx, 4, 512);
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
	buffer.writeNullString(LAYERED_FILE_MAGIC_V3);

	int32_t width = getCanvasWidth();
	int32_t height = getCanvasHeight();
	uint32_t layerCount = (uint32_t)canvas.get().layers.getLength();
	int32_t storedActiveLayerIndex = activeLayerIndex;
	buffer.write(width);
	buffer.write(height);
	buffer.write(layerCount);
	buffer.write(storedActiveLayerIndex);
	for (int c = 0; c < 4; c++)
	{
		float fc = canvas.get().canvasFallbackRgba[c];
		buffer.write(fc);
	}

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
	const bool isV3 = (magic == LAYERED_FILE_MAGIC_V3);
	const bool isV2 = (magic == LAYERED_FILE_MAGIC) || isV3;
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
	if (isV3)
	{
		for (int c = 0; c < 4; c++) span.read(document.canvasFallbackRgba[c]);
	}
	else
	{
		// V1/V2 had no stored backdrop; keep previous transparent-composite behavior.
		document.canvasFallbackRgba[0] = 0.f;
		document.canvasFallbackRgba[1] = 0.f;
		document.canvasFallbackRgba[2] = 0.f;
		document.canvasFallbackRgba[3] = 0.f;
	}

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
		if (saveDocumentToPath(newPath))
		{
			path = newPath;
		}
		else
		{
			openSaveFailedPopup = true;
		}
	}
}

void PaintEditor::requestSave()
{
	if (!path.isEmpty())
	{
		if (!saveDocumentToPath(path))
		{
			openSaveFailedPopup = true;
		}
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

void PaintEditor::applyEraserWorkArea()
{
	const int32_t cw = getCanvasWidth();
	const int32_t ch = getCanvasHeight();
	if (cw <= 0 || ch <= 0) return;
	bbe::Image &layer = getActiveLayerImage();
	for (int32_t y = 0; y < ch; ++y)
	{
		for (int32_t x = 0; x < cw; ++x)
		{
			if (workArea.getPixel((size_t)x, (size_t)y).a == 0) continue;
			bbe::Colori c = layer.getPixel((size_t)x, (size_t)y);
			c.a = 0;
			layer.setPixel((size_t)x, (size_t)y, c);
		}
	}
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
	int32_t iw = width > (uint32_t)INT32_MAX ? INT32_MAX : (int32_t)width;
	int32_t ih = height > (uint32_t)INT32_MAX ? INT32_MAX : (int32_t)height;
	if (iw < 1) iw = 1;
	if (ih < 1) ih = 1;
	canvas.get().layers.clear();
	canvas.get().canvasFallbackRgba[0] = 1.f;
	canvas.get().canvasFallbackRgba[1] = 1.f;
	canvas.get().canvasFallbackRgba[2] = 1.f;
	canvas.get().canvasFallbackRgba[3] = 1.f;
	canvas.get().layers.add(makeLayer("Layer 1", iw, ih, bbe::Color(0.f, 0.f, 0.f, 0.f)));
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
	setCanvasFallbackFromImageAlpha(img);
	canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, bbe::BlendMode::Normal, std::move(img) });
	activeLayerIndex = 0;
	this->path = path;
	setupCanvas();
	return true;
}

void PaintEditor::setCanvasFallbackFromImageAlpha(const bbe::Image &image)
{
	const int32_t w = image.getWidth();
	const int32_t h = image.getHeight();
	if (w <= 0 || h <= 0)
	{
		canvas.get().canvasFallbackRgba[0] = 1.f;
		canvas.get().canvasFallbackRgba[1] = 1.f;
		canvas.get().canvasFallbackRgba[2] = 1.f;
		canvas.get().canvasFallbackRgba[3] = 1.f;
		return;
	}
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			if (image.getPixel((size_t)x, (size_t)y).a < 255)
			{
				canvas.get().canvasFallbackRgba[0] = 0.f;
				canvas.get().canvasFallbackRgba[1] = 0.f;
				canvas.get().canvasFallbackRgba[2] = 0.f;
				canvas.get().canvasFallbackRgba[3] = 0.f;
				return;
			}
		}
	}
	canvas.get().canvasFallbackRgba[0] = 1.f;
	canvas.get().canvasFallbackRgba[1] = 1.f;
	canvas.get().canvasFallbackRgba[2] = 1.f;
	canvas.get().canvasFallbackRgba[3] = 1.f;
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

bbe::Rectanglei PaintEditor::getEraserPixelRect(const bbe::Vector2 &canvasPos) const
{
	const int32_t n = eraserSize;
	return bbe::Rectanglei(
		(int32_t)std::floor(canvasPos.x - n * 0.5f + 0.5f),
		(int32_t)std::floor(canvasPos.y - n * 0.5f + 0.5f),
		n,
		n);
}

void PaintEditor::appendEraserStampRectsAlongSegment(const bbe::Vector2 &pNew, const bbe::Vector2 &pOld, bbe::List<bbe::Rectanglei> &out) const
{
	const float dist = (pNew - pOld).getLength();
	if (dist < 1e-5f) return;
	const int steps = bbe::Math::max(1, (int)std::ceil((double)dist));
	// Skip i=0 (pOld): that position was already stamped on the previous frame.
	for (int i = 1; i <= steps; ++i)
	{
		const bbe::Vector2 p = pOld + (pNew - pOld) * ((float)i / (float)steps);
		const auto sym = getSymmetryPositions(p);
		for (size_t s = 0; s < sym.getLength(); s++)
			out.add(getEraserPixelRect(sym[s]));
	}
}

namespace {

// Opaque white matches a typical bottom layer fill, so the in-flight mask would disappear there.
// applyEraserWorkArea only tests alpha != 0; RGB is for preview visibility only.
constexpr bbe::Colori kEraserWorkAreaMark(255, 64, 255, 230);

bool eraseStampAtCanvasPointNoSymmetry(
	bbe::Image &workArea,
	const bbe::Rectanglei &stamp,
	bool tiled,
	int32_t cw,
	int32_t ch)
{
	const int32_t eraserN = stamp.width;
	if (eraserN < 1 || stamp.height != eraserN || cw <= 0 || ch <= 0) return false;
	const int32_t left = stamp.x;
	const int32_t top = stamp.y;
	const bbe::Colori mark = kEraserWorkAreaMark;
	bool changed = false;
	for (int32_t yy = top; yy < top + eraserN; ++yy)
	{
		for (int32_t xx = left; xx < left + eraserN; ++xx)
		{
			int32_t px = xx;
			int32_t py = yy;
			if (tiled)
			{
				px = bbe::Math::mod<int32_t>(px, cw);
				py = bbe::Math::mod<int32_t>(py, ch);
			}
			else if (px < 0 || py < 0 || px >= cw || py >= ch) continue;
			workArea.setPixel((size_t)px, (size_t)py, mark);
			changed = true;
		}
	}
	return changed;
}

} // namespace

bool PaintEditor::eraseStampAtCanvasWithSymmetry(const bbe::Vector2 &canvasPos)
{
	const int32_t cw = getCanvasWidth();
	const int32_t ch = getCanvasHeight();
	bool changed = false;
	const auto positions = getSymmetryPositions(canvasPos);
	for (size_t i = 0; i < positions.getLength(); i++)
		changed |= eraseStampAtCanvasPointNoSymmetry(workArea, getEraserPixelRect(positions[i]), tiled, cw, ch);
	return changed;
}

bool PaintEditor::eraseLineOnWorkAreaWithSymmetry(const bbe::Vector2 &pNew, const bbe::Vector2 &pOld)
{
	const float dist = (pNew - pOld).getLength();
	if (dist < 1e-5f) return false;
	const int steps = bbe::Math::max(1, (int)std::ceil((double)dist));
	bool changed = false;
	for (int i = 1; i <= steps; ++i)
	{
		const bbe::Vector2 p = pOld + (pNew - pOld) * ((float)i / (float)steps);
		changed |= eraseStampAtCanvasWithSymmetry(p);
	}
	return changed;
}

namespace {

thread_local std::mt19937 g_sprayRng{std::random_device{}()};

// Composites one spray speck onto the work-area image (straight alpha). dropletAlpha is a random stain strength in 1..255, scaled by brushColor.a.
bool sprayBlendDropletOntoWorkArea(bbe::Image &workArea, float xf, float yf, const bbe::Colori &brushColor, bbe::byte dropletAlpha, bool tiled)
{
	int32_t px = (int32_t)std::floor(xf);
	int32_t py = (int32_t)std::floor(yf);
	const int32_t w = workArea.getWidth();
	const int32_t h = workArea.getHeight();
	if (w <= 0 || h <= 0) return false;
	if (tiled)
	{
		px = bbe::Math::mod<int32_t>(px, w);
		py = bbe::Math::mod<int32_t>(py, h);
	}
	else if (px < 0 || py < 0 || px >= w || py >= h) return false;

	const uint32_t ba = (uint32_t)brushColor.a;
	if (ba == 0) return false;
	const uint32_t combinedA = (uint32_t(dropletAlpha) * ba + 127u) / 255u;
	if (combinedA == 0) return false;
	const bbe::Colori src(
		brushColor.r,
		brushColor.g,
		brushColor.b,
		(bbe::byte)bbe::Math::min<uint32_t>(combinedA, 255u));

	const bbe::Colori dst = workArea.getPixel((size_t)px, (size_t)py);
	const bbe::Colori out = dst.blendTo(src, 1.f, bbe::BlendMode::Normal);
	workArea.setPixel((size_t)px, (size_t)py, out);
	return true;
}

} // namespace

bool PaintEditor::sprayBurstAtCanvasWithSymmetry(const bbe::Vector2 &canvasPos, bool leftDown, bool rightDown)
{
	const int32_t cw = getCanvasWidth();
	const int32_t ch = getCanvasHeight();
	if (cw <= 0 || ch <= 0) return false;
	const bbe::Colori color = activeDrawColor(leftDown, rightDown);
	std::uniform_real_distribution<float> angDist(0.f, bbe::Math::TAU);
	std::uniform_real_distribution<float> radDist(0.f, 1.f);
	std::uniform_int_distribution<int> dropletAlphaDist(1, 255);
	const float radius = (float)sprayWidth;
	const int droplets = sprayDensity;
	bool changed = false;
	const auto centers = getSymmetryPositions(canvasPos);
	for (size_t s = 0; s < centers.getLength(); s++)
	{
		const bbe::Vector2 &c = centers[s];
		for (int i = 0; i < droplets; i++)
		{
			const float t = radDist(g_sprayRng);
			// Bias samples toward the center: r = R * t^2 (t ~ U[0,1]) vs sqrt(t)*R for area-uniform disk.
			const float r = radius > 0.f ? t * t * radius : 0.f;
			const float ang = angDist(g_sprayRng);
			const bbe::Vector2 p(c.x + std::cos(ang) * r, c.y + std::sin(ang) * r);
			const bbe::byte da = (bbe::byte)dropletAlphaDist(g_sprayRng);
			changed |= sprayBlendDropletOntoWorkArea(workArea, p.x, p.y, color, da, tiled);
		}
	}
	return changed;
}

bool PaintEditor::sprayLineOnWorkAreaWithSymmetry(const bbe::Vector2 &pNew, const bbe::Vector2 &pOld, bool leftDown, bool rightDown)
{
	const float dist = (pNew - pOld).getLength();
	if (dist < 1e-5f) return false;
	const int steps = bbe::Math::max(1, (int)std::ceil((double)dist));
	bool changed = false;
	for (int i = 1; i <= steps; ++i)
	{
		const bbe::Vector2 p = pOld + (pNew - pOld) * ((float)i / (float)steps);
		changed |= sprayBurstAtCanvasWithSymmetry(p, leftDown, rightDown);
	}
	return changed;
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
