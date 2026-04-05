#include "ExamplePaintPalette.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace paintPalette
{
namespace {

float srgbByteToLinear(uint8_t v)
{
	const float c = (float)v / 255.f;
	if (c <= 0.04045f) return c / 12.92f;
	const float t = (c + 0.055f) / 1.055f;
	return std::pow(t, 2.4f);
}

float linearToSrgbByte(float l)
{
	float c;
	if (l <= 0.0031308f) c = 12.92f * l;
	else c = 1.055f * std::pow(l, 1.f / 2.4f) - 0.055f;
	return std::round(bbe::Math::clamp(c, 0.f, 1.f) * 255.f);
}

} // namespace

Oklab srgbByteToOklab(uint8_t r, uint8_t g, uint8_t b)
{
	const float R = srgbByteToLinear(r);
	const float G = srgbByteToLinear(g);
	const float B = srgbByteToLinear(b);

	const float l_ = std::cbrt(0.4122214708f * R + 0.5363325363f * G + 0.0514459929f * B);
	const float m_ = std::cbrt(0.2119034982f * R + 0.6806995451f * G + 0.1073969566f * B);
	const float s_ = std::cbrt(0.0883024619f * R + 0.2817188376f * G + 0.6299787005f * B);

	Oklab o;
	o.L = 0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_;
	o.a = 1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_;
	o.b = 0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_;
	return o;
}

float oklabDist2(const Oklab &x, const Oklab &y)
{
	const float dL = x.L - y.L;
	const float da = x.a - y.a;
	const float db = x.b - y.b;
	return dL * dL + da * da + db * db;
}

int32_t nearestPaletteIndexRgb(uint8_t r, uint8_t g, uint8_t b, const bbe::List<bbe::Colori> &palette)
{
	const size_t n = palette.getLength();
	if (n == 0) return 0;
	const Oklab o = srgbByteToOklab(r, g, b);
	int32_t best = 0;
	float bestD = std::numeric_limits<float>::infinity();
	uint32_t bestPack = 0xFFFFFFFFu;
	for (size_t i = 0; i < n; i++)
	{
		const bbe::Colori &p = palette[i];
		const float d = oklabDist2(o, srgbByteToOklab(p.r, p.g, p.b));
		const uint32_t pk = packedRgb(p.r, p.g, p.b);
		if (d < bestD - 1e-9f || (std::fabs(d - bestD) <= 1e-9f && pk < bestPack))
		{
			bestD = d;
			best = (int32_t)i;
			bestPack = pk;
		}
	}
	return best;
}

void deduplicatePalettePreserveOrderRemapIndices(bbe::List<bbe::Colori> &colors, int32_t &primaryIdx, int32_t &secondaryIdx)
{
	const size_t n = colors.getLength();
	if (n == 0)
	{
		primaryIdx = 0;
		secondaryIdx = 0;
		return;
	}
	primaryIdx = bbe::Math::clamp(primaryIdx, 0, (int32_t)n - 1);
	secondaryIdx = bbe::Math::clamp(secondaryIdx, 0, (int32_t)n - 1);

	bbe::List<bbe::Colori> src;
	src = std::move(colors);
	colors.clear();
	std::vector<uint32_t> firstSlotForRgb(1u << 24, 0xFFFFFFFFu);
	std::vector<int32_t> oldToNew(n, 0);
	for (size_t i = 0; i < n; i++)
	{
		const bbe::Colori &col = src[i];
		const uint32_t k = packedRgb(col.r, col.g, col.b) & 0xFFFFFFu;
		if (firstSlotForRgb[k] != 0xFFFFFFFFu)
		{
			oldToNew[i] = (int32_t)firstSlotForRgb[k];
		}
		else
		{
			const uint32_t ni = (uint32_t)colors.getLength();
			colors.add(col);
			firstSlotForRgb[k] = ni;
			oldToNew[i] = (int32_t)ni;
		}
	}
	primaryIdx = oldToNew[(size_t)primaryIdx];
	secondaryIdx = oldToNew[(size_t)secondaryIdx];
}

void ensurePaletteNonEmptyWhenModeOn(bbe::List<bbe::Colori> &colors, int32_t &primaryIdx, int32_t &secondaryIdx, bool paletteModeActive)
{
	if (!paletteModeActive) return;
	if (!colors.isEmpty()) return;
	colors.add(bbe::Colori(255, 255, 255, 255));
	primaryIdx = 0;
	secondaryIdx = 0;
}

void clampPalettePrimarySecondaryIndices(const bbe::List<bbe::Colori> &colors, int32_t &primaryIdx, int32_t &secondaryIdx)
{
	const size_t n = colors.getLength();
	if (n == 0)
	{
		primaryIdx = 0;
		secondaryIdx = 0;
		return;
	}
	const int32_t mx = (int32_t)n - 1;
	primaryIdx = bbe::Math::clamp(primaryIdx, 0, mx);
	secondaryIdx = bbe::Math::clamp(secondaryIdx, 0, mx);
}

void collectDistinctOpaqueRgb(const bbe::Image &img, std::vector<std::array<uint8_t, 3>> &outRgb)
{
	outRgb.clear();
	const int32_t w = img.getWidth();
	const int32_t h = img.getHeight();
	if (w <= 0 || h <= 0 || !img.isLoadedCpu()) return;

	std::vector<uint8_t> seen(1u << 24, 0);
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			const bbe::Colori c = img.getPixel((size_t)x, (size_t)y);
			if (c.a == 0) continue;
			const uint32_t key = ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | (uint32_t)c.b;
			if (seen[key]) continue;
			seen[key] = 1;
			outRgb.push_back({ c.r, c.g, c.b });
		}
	}
	std::sort(outRgb.begin(), outRgb.end(), [](const std::array<uint8_t, 3> &a, const std::array<uint8_t, 3> &b) {
		if (a[0] != b[0]) return a[0] < b[0];
		if (a[1] != b[1]) return a[1] < b[1];
		return a[2] < b[2];
	});
	outRgb.erase(std::unique(outRgb.begin(), outRgb.end(),
							  [](const std::array<uint8_t, 3> &a, const std::array<uint8_t, 3> &b) {
								  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
							  }),
				 outRgb.end());
}

uint32_t packedRgb(uint8_t r, uint8_t g, uint8_t b)
{
	return (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
}

const std::array<std::array<uint8_t, 3>, 16> &defaultPaletteRgb16()
{
	static const std::array<std::array<uint8_t, 3>, 16> k = { {
		{ { 0, 0, 0 } },
		{ { 255, 255, 255 } },
		{ { 128, 128, 128 } },
		{ { 192, 192, 192 } },
		{ { 128, 0, 0 } },
		{ { 255, 0, 0 } },
		{ { 255, 128, 0 } },
		{ { 255, 255, 0 } },
		{ { 128, 255, 0 } },
		{ { 0, 255, 0 } },
		{ { 0, 255, 128 } },
		{ { 0, 255, 255 } },
		{ { 0, 128, 255 } },
		{ { 0, 0, 255 } },
		{ { 128, 0, 255 } },
		{ { 255, 0, 255 } },
	} };
	return k;
}

void fillPaletteToCount(const std::vector<std::array<uint8_t, 3>> &existingUnique, int32_t targetCount, bbe::List<bbe::Colori> &outPalette)
{
	outPalette.clear();
	if (targetCount < 1) targetCount = 1;

	std::vector<uint8_t> used(1u << 24, 0);
	auto tryAdd = [&](uint8_t r, uint8_t g, uint8_t b) {
		if ((int32_t)outPalette.getLength() >= targetCount) return;
		const uint32_t key = packedRgb(r, g, b) & 0xFFFFFFu;
		if (used[key]) return;
		used[key] = 1;
		outPalette.add(bbe::Colori(r, g, b, 255));
	};

	for (const auto &rgb : existingUnique)
		tryAdd(rgb[0], rgb[1], rgb[2]);

	for (const auto &rgb : defaultPaletteRgb16())
		tryAdd(rgb[0], rgb[1], rgb[2]);

	// Deterministic extra spread: 3D grid + diagonal walks in RGB
	for (int32_t step = 32; (int32_t)outPalette.getLength() < targetCount && step >= 4; step /= 2)
	{
		for (int32_t r = 0; r <= 255 && (int32_t)outPalette.getLength() < targetCount; r += step)
		{
			for (int32_t g = 0; g <= 255 && (int32_t)outPalette.getLength() < targetCount; g += step)
			{
				for (int32_t b = 0; b <= 255 && (int32_t)outPalette.getLength() < targetCount; b += step)
				{
					tryAdd((uint8_t)bbe::Math::clamp(r, 0, 255), (uint8_t)bbe::Math::clamp(g, 0, 255), (uint8_t)bbe::Math::clamp(b, 0, 255));
				}
			}
		}
	}

	int32_t salt = 0;
	while ((int32_t)outPalette.getLength() < targetCount)
	{
		const uint32_t u = (uint32_t)(0x9E3779B1u + (uint32_t)salt * 0x85EBCA6Bu);
		tryAdd((uint8_t)(u & 255u), (uint8_t)((u >> 8) & 255u), (uint8_t)((u >> 16) & 255u));
		salt++;
	}
}

void buildReducedPalette(const std::vector<std::array<uint8_t, 3>> &distinctSorted, int32_t y, bbe::List<bbe::Colori> &outPalette)
{
	outPalette.clear();
	if (y < 1) y = 1;
	const size_t n = distinctSorted.size();
	if (n == 0)
	{
		fillPaletteToCount(distinctSorted, y, outPalette);
		return;
	}
	if (n <= (size_t)y)
	{
		fillPaletteToCount(distinctSorted, y, outPalette);
		return;
	}

	std::vector<std::array<uint8_t, 3>> chosen;
	chosen.reserve((size_t)y);

	auto inChosen = [&](uint8_t r, uint8_t g, uint8_t b) -> bool {
		for (const auto &c : chosen)
			if (c[0] == r && c[1] == g && c[2] == b) return true;
		return false;
	};

	auto minDistToChosen = [&](const std::array<uint8_t, 3> &c) -> float {
		const Oklab o = srgbByteToOklab(c[0], c[1], c[2]);
		float md = std::numeric_limits<float>::infinity();
		for (const auto &p : chosen)
		{
			const float d = oklabDist2(o, srgbByteToOklab(p[0], p[1], p[2]));
			md = bbe::Math::min(md, d);
		}
		return md;
	};

	// Seed: maximize min distance to black and white in OKLab (spread).
	{
		float bestScore = -1.f;
		std::array<uint8_t, 3> best = distinctSorted[0];
		const Oklab ob = srgbByteToOklab(0, 0, 0);
		const Oklab ow = srgbByteToOklab(255, 255, 255);
		for (const auto &c : distinctSorted)
		{
			const Oklab o = srgbByteToOklab(c[0], c[1], c[2]);
			const float s = bbe::Math::min(oklabDist2(o, ob), oklabDist2(o, ow));
			const uint32_t pk = packedRgb(c[0], c[1], c[2]);
			const uint32_t pb = packedRgb(best[0], best[1], best[2]);
			if (s > bestScore + 1e-9f || (std::fabs(s - bestScore) <= 1e-9f && pk < pb))
			{
				bestScore = s;
				best = c;
			}
		}
		chosen.push_back(best);
	}

	while ((int32_t)chosen.size() < y)
	{
		float bestMd = -1.f;
		std::array<uint8_t, 3> pick = distinctSorted[0];
		for (const auto &c : distinctSorted)
		{
			if (inChosen(c[0], c[1], c[2])) continue;
			const float md = minDistToChosen(c);
			const uint32_t pc = packedRgb(c[0], c[1], c[2]);
			const uint32_t pp = packedRgb(pick[0], pick[1], pick[2]);
			if (md > bestMd + 1e-9f || (std::fabs(md - bestMd) <= 1e-9f && pc < pp))
			{
				bestMd = md;
				pick = c;
			}
		}
		chosen.push_back(pick);
	}

	for (const auto &c : chosen)
		outPalette.add(bbe::Colori(c[0], c[1], c[2], 255));
}

void quantizeImageToPaletteInPlace(bbe::Image &img, const bbe::List<bbe::Colori> &palette, bool dither, int32_t alphaCutoff)
{
	const int32_t w = img.getWidth();
	const int32_t h = img.getHeight();
	if (w <= 0 || h <= 0 || !img.isLoadedCpu() || palette.getLength() == 0) return;

	if (!dither)
	{
		for (int32_t y = 0; y < h; y++)
		{
			for (int32_t x = 0; x < w; x++)
			{
				bbe::Colori c = img.getPixel((size_t)x, (size_t)y);
				if (c.a < alphaCutoff)
				{
					img.setPixel((size_t)x, (size_t)y, bbe::Colori(0, 0, 0, 0));
					continue;
				}
				const int32_t idx = nearestPaletteIndexRgb(c.r, c.g, c.b, palette);
				const bbe::Colori &p = palette[(size_t)idx];
				img.setPixel((size_t)x, (size_t)y, bbe::Colori(p.r, p.g, p.b, 255));
			}
		}
		return;
	}

	const size_t cells = (size_t)w * (size_t)h;
	std::vector<bbe::Colori> orig(cells);
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			orig[(size_t)y * (size_t)w + (size_t)x] = img.getPixel((size_t)x, (size_t)y);
		}
	}

	// Floyd–Steinberg on linear RGB, only where original alpha >= alphaCutoff; others stay transparent.
	std::vector<std::array<float, 3>> err(cells, { { 0.f, 0.f, 0.f } });
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			const size_t idx = (size_t)y * (size_t)w + (size_t)x;
			const bbe::Colori c = orig[idx];
			if (c.a < alphaCutoff)
			{
				img.setPixel((size_t)x, (size_t)y, bbe::Colori(0, 0, 0, 0));
				continue;
			}

			float lr = srgbByteToLinear(c.r) + err[idx][0];
			float lg = srgbByteToLinear(c.g) + err[idx][1];
			float lb = srgbByteToLinear(c.b) + err[idx][2];
			lr = bbe::Math::clamp01(lr);
			lg = bbe::Math::clamp01(lg);
			lb = bbe::Math::clamp01(lb);

			const uint8_t tr = (uint8_t)bbe::Math::clamp((int32_t)linearToSrgbByte(lr), 0, 255);
			const uint8_t tg = (uint8_t)bbe::Math::clamp((int32_t)linearToSrgbByte(lg), 0, 255);
			const uint8_t tb = (uint8_t)bbe::Math::clamp((int32_t)linearToSrgbByte(lb), 0, 255);
			const int32_t pi = nearestPaletteIndexRgb(tr, tg, tb, palette);
			const bbe::Colori &p = palette[(size_t)pi];
			img.setPixel((size_t)x, (size_t)y, bbe::Colori(p.r, p.g, p.b, 255));

			const float pr = srgbByteToLinear(p.r);
			const float pg = srgbByteToLinear(p.g);
			const float pb = srgbByteToLinear(p.b);
			const float er0 = lr - pr;
			const float er1 = lg - pg;
			const float er2 = lb - pb;

			auto addErr = [&](int32_t nx, int32_t ny, float f) {
				if (nx < 0 || ny < 0 || nx >= w || ny >= h) return;
				const bbe::Colori on = orig[(size_t)ny * (size_t)w + (size_t)nx];
				if (on.a < alphaCutoff) return;
				const size_t ni = (size_t)ny * (size_t)w + (size_t)nx;
				err[ni][0] += er0 * f;
				err[ni][1] += er1 * f;
				err[ni][2] += er2 * f;
			};

			addErr(x + 1, y, 7.f / 16.f);
			addErr(x - 1, y + 1, 3.f / 16.f);
			addErr(x, y + 1, 5.f / 16.f);
			addErr(x + 1, y + 1, 1.f / 16.f);
		}
	}
}

void replaceRgbInImageInPlace(bbe::Image &img, const std::array<uint8_t, 3> &fromRgb, const std::array<uint8_t, 3> &toRgb, int32_t alphaCutoff)
{
	const int32_t w = img.getWidth();
	const int32_t h = img.getHeight();
	if (w <= 0 || h <= 0 || !img.isLoadedCpu()) return;
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			bbe::Colori c = img.getPixel((size_t)x, (size_t)y);
			if (c.a < alphaCutoff) continue;
			if (c.r == fromRgb[0] && c.g == fromRgb[1] && c.b == fromRgb[2])
				img.setPixel((size_t)x, (size_t)y, bbe::Colori(toRgb[0], toRgb[1], toRgb[2], 255));
		}
	}
}

int64_t countPixelsWithRgb(const bbe::Image &img, const std::array<uint8_t, 3> &rgb, int32_t alphaCutoff)
{
	int64_t n = 0;
	const int32_t w = img.getWidth();
	const int32_t h = img.getHeight();
	if (w <= 0 || h <= 0 || !img.isLoadedCpu()) return 0;
	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			const bbe::Colori c = img.getPixel((size_t)x, (size_t)y);
			if (c.a < alphaCutoff) continue;
			if (c.r == rgb[0] && c.g == rgb[1] && c.b == rgb[2]) n++;
		}
	}
	return n;
}

void remapRemovedPaletteColorInPlace(bbe::Image &img, const std::array<uint8_t, 3> &removedRgb, const bbe::List<bbe::Colori> &newPalette, int32_t alphaCutoff)
{
	const int32_t w = img.getWidth();
	const int32_t h = img.getHeight();
	if (w <= 0 || h <= 0 || !img.isLoadedCpu() || newPalette.getLength() == 0) return;

	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			bbe::Colori c = img.getPixel((size_t)x, (size_t)y);
			if (c.a < alphaCutoff) continue;
			if (c.r != removedRgb[0] || c.g != removedRgb[1] || c.b != removedRgb[2]) continue;
			const int32_t ni = nearestPaletteIndexRgb(c.r, c.g, c.b, newPalette);
			const bbe::Colori &p = newPalette[(size_t)ni];
			img.setPixel((size_t)x, (size_t)y, bbe::Colori(p.r, p.g, p.b, 255));
		}
	}
}

bool runPaletteRegressionSelfChecks(bbe::String *outMessage)
{
	auto fail = [&](const char *msg) -> bool {
		if (outMessage) *outMessage = msg;
		return false;
	};

	// Dedupe + index remap
	{
		bbe::List<bbe::Colori> cols;
		cols.add(bbe::Colori(10, 20, 30, 255));
		cols.add(bbe::Colori(10, 20, 30, 255));
		cols.add(bbe::Colori(40, 50, 60, 255));
		int32_t pri = 99;
		int32_t sec = -5;
		deduplicatePalettePreserveOrderRemapIndices(cols, pri, sec);
		if (cols.getLength() != 2u) return fail("dedupe length");
		if (cols[0].r != 10 || cols[1].g != 50) return fail("dedupe colors");
		if (pri != 1 || sec != 0) return fail("dedupe remap");
	}

	// Empty palette + mode on -> white
	{
		bbe::List<bbe::Colori> cols;
		int32_t pri = 3, sec = 7;
		ensurePaletteNonEmptyWhenModeOn(cols, pri, sec, true);
		if (cols.getLength() != 1u) return fail("empty repair length");
		if (cols[0].r != 255 || pri != 0 || sec != 0) return fail("empty repair white");
	}

	// Nearest-color order independence (same chosen RGB after reversing list order)
	{
		bbe::List<bbe::Colori> orderA, orderB;
		orderA.add(bbe::Colori(200, 0, 0, 255));
		orderA.add(bbe::Colori(0, 200, 0, 255));
		orderA.add(bbe::Colori(0, 0, 200, 255));
		orderB.add(bbe::Colori(0, 0, 200, 255));
		orderB.add(bbe::Colori(0, 200, 0, 255));
		orderB.add(bbe::Colori(200, 0, 0, 255));
		for (int tr = 0; tr < 256; tr += 31)
		{
			for (int tg = 0; tg < 256; tg += 37)
			{
				for (int tb = 0; tb < 256; tb += 41)
				{
					const int32_t ia = nearestPaletteIndexRgb((uint8_t)tr, (uint8_t)tg, (uint8_t)tb, orderA);
					const int32_t ib = nearestPaletteIndexRgb((uint8_t)tr, (uint8_t)tg, (uint8_t)tb, orderB);
					const bbe::Colori &ca = orderA[(size_t)ia];
					const bbe::Colori &cb = orderB[(size_t)ib];
					if (ca.r != cb.r || ca.g != cb.g || ca.b != cb.b) return fail("nearest order independence");
				}
			}
		}
	}

	// Quantization invariant under palette reorder (nearest path, no dither)
	{
		bbe::List<bbe::Colori> p1, p2;
		p1.add(bbe::Colori(0, 0, 0, 255));
		p1.add(bbe::Colori(255, 255, 255, 255));
		p1.add(bbe::Colori(128, 64, 192, 255));
		p2.add(bbe::Colori(128, 64, 192, 255));
		p2.add(bbe::Colori(0, 0, 0, 255));
		p2.add(bbe::Colori(255, 255, 255, 255));
		bbe::Image a(12, 10, bbe::Color(0.f, 0.f, 0.f, 0.f));
		bbe::Image b(12, 10, bbe::Color(0.f, 0.f, 0.f, 0.f));
		for (int32_t y = 0; y < 10; y++)
		{
			for (int32_t x = 0; x < 12; x++)
			{
				const uint8_t v = (uint8_t)((x * 17 + y * 31) & 255);
				a.setPixel((size_t)x, (size_t)y, bbe::Colori(v, (uint8_t)(v ^ 90u), (uint8_t)(v / 2u), 255));
				b.setPixel((size_t)x, (size_t)y, bbe::Colori(v, (uint8_t)(v ^ 90u), (uint8_t)(v / 2u), 255));
			}
		}
		quantizeImageToPaletteInPlace(a, p1, false, 50);
		quantizeImageToPaletteInPlace(b, p2, false, 50);
		for (int32_t y = 0; y < 10; y++)
		{
			for (int32_t x = 0; x < 12; x++)
			{
				const bbe::Colori ca = a.getPixel((size_t)x, (size_t)y);
				const bbe::Colori cb = b.getPixel((size_t)x, (size_t)y);
				if (ca.r != cb.r || ca.g != cb.g || ca.b != cb.b || ca.a != cb.a) return fail("quantize reorder invariant");
			}
		}
	}

	return true;
}

} // namespace paintPalette
