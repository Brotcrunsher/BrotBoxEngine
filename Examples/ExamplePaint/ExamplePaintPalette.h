#pragma once

#include "BBE/BrotBoxEngine.h"
#include <cstdint>
#include <vector>

namespace paintPalette
{

struct Oklab
{
	float L = 0.f;
	float a = 0.f;
	float b = 0.f;
};

Oklab srgbByteToOklab(uint8_t r, uint8_t g, uint8_t b);
float oklabDist2(const Oklab &x, const Oklab &y);

/// Nearest palette index by OKLab distance (RGB channels only). \p palette entries use RGB; alpha ignored.
/// Ties use smaller \ref packedRgb (order-independent; reordering the list does not change the chosen color).
int32_t nearestPaletteIndexRgb(uint8_t r, uint8_t g, uint8_t b, const bbe::List<bbe::Colori> &palette);

/// Remove duplicate RGB entries (keeps first occurrence order). Remaps \p primaryIdx / \p secondaryIdx from old indices to new.
void deduplicatePalettePreserveOrderRemapIndices(bbe::List<bbe::Colori> &colors, int32_t &primaryIdx, int32_t &secondaryIdx);

/// If \p paletteModeActive and \p colors is empty, inserts opaque white and sets both indices to 0.
void ensurePaletteNonEmptyWhenModeOn(bbe::List<bbe::Colori> &colors, int32_t &primaryIdx, int32_t &secondaryIdx, bool paletteModeActive);

/// Clamp indices to [0, length-1] or 0 if empty.
void clampPalettePrimarySecondaryIndices(const bbe::List<bbe::Colori> &colors, int32_t &primaryIdx, int32_t &secondaryIdx);

/// Non-debug self-checks for palette math, dedupe, nearest-color order invariance, and quantization invariance under reorder.
/// Returns false on failure; optional \p outMessage describes the first failure.
bool runPaletteRegressionSelfChecks(bbe::String *outMessage = nullptr);

/// Collect distinct RGB for pixels with alpha > 0, sorted by packed RGB (deterministic).
void collectDistinctOpaqueRgb(const bbe::Image &img, std::vector<std::array<uint8_t, 3>> &outRgb);

uint32_t packedRgb(uint8_t r, uint8_t g, uint8_t b);

/// Exact 16-color default ramp (RGB).
const std::array<std::array<uint8_t, 3>, 16> &defaultPaletteRgb16();

/// Append defaults + generated spread until \p targetCount unique RGB entries (deterministic).
void fillPaletteToCount(const std::vector<std::array<uint8_t, 3>> &existingUnique, int32_t targetCount, bbe::List<bbe::Colori> &outPalette);

/// Build Y-color palette when image has more than Y distinct opaque RGBs (farthest-point in OKLab).
void buildReducedPalette(const std::vector<std::array<uint8_t, 3>> &distinctSorted, int32_t y, bbe::List<bbe::Colori> &outPalette);

/// Alpha < \p alphaCutoff becomes transparent; else opaque nearest palette. If \p dither, Floyd–Steinberg on linear RGB (opaque pixels only).
void quantizeImageToPaletteInPlace(bbe::Image &img, const bbe::List<bbe::Colori> &palette, bool dither, int32_t alphaCutoff = 50);

/// Replace exact RGB \p fromRgb with \p toRgb for all opaque pixels (alpha >= alphaCutoff).
void replaceRgbInImageInPlace(bbe::Image &img, const std::array<uint8_t, 3> &fromRgb, const std::array<uint8_t, 3> &toRgb, int32_t alphaCutoff = 50);

/// Count opaque pixels matching exact RGB (alpha >= cutoff).
int64_t countPixelsWithRgb(const bbe::Image &img, const std::array<uint8_t, 3> &rgb, int32_t alphaCutoff = 50);

/// Remap pixels that used removed color to nearest remaining palette entry (OKLab).
void remapRemovedPaletteColorInPlace(bbe::Image &img, const std::array<uint8_t, 3> &removedRgb, const bbe::List<bbe::Colori> &newPalette,
									 int32_t alphaCutoff = 50);

} // namespace paintPalette
