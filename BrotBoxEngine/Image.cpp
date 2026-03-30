#include "BBE/Image.h"
#include "BBE/Error.h"
#include "BBE/Math.h"
#include "BBE/Grid.h"
#include "BBE/Rectangle.h"
#if defined(__linux__)
#include "BBE/WaylandClipboard.h"
#endif
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace
{
	bool checkedMultiplySizeT(size_t &value, size_t factor)
	{
		if (factor != 0 && value > std::numeric_limits<size_t>::max() / factor)
		{
			return false;
		}

		value *= factor;
		return true;
	}

	std::vector<bbe::byte> encodeRawImageAsPng(const bbe::byte *imageData, int width, int height, size_t componentCount, size_t bytesPerChannel)
	{
		if (imageData == nullptr || bytesPerChannel != 1)
		{
			return {};
		}

		if (width <= 0 || height <= 0 || (componentCount != 1 && componentCount != 4))
		{
			return {};
		}

		const int componentCountInt = static_cast<int>(componentCount);
		int pngLength = 0;
		unsigned char *pngData = stbi_write_png_to_mem(
			imageData,
			width * componentCountInt,
			width,
			height,
			componentCountInt,
			&pngLength);
		if (pngData == nullptr || pngLength <= 0)
		{
			return {};
		}

		std::vector<bbe::byte> retVal(pngData, pngData + pngLength);
		std::free(pngData);
		return retVal;
	}

	bool toTiledPos(int32_t &x, int32_t &y, int32_t w, int32_t h, bool tiled)
	{
		if (w <= 0 || h <= 0) return false;
		if (tiled)
		{
			x = bbe::Math::mod<int32_t>(x, w);
			y = bbe::Math::mod<int32_t>(y, h);
			return true;
		}
		return x >= 0 && y >= 0 && x < w && y < h;
	}
}

bool bbe::Image::finishLoad(stbi_uc *pixels)
{
	m_format = ImageFormat::R8G8B8A8; // Is correct, even if texChannels == 3, because stbi is transforming the data for us on the fly.

	if (pixels == nullptr)
	{
		return false;
	}

	m_pdata.resizeCapacityAndLengthUninit(getSizeInBytes());
	memcpy(m_pdata.getRaw(), pixels, getSizeInBytes());

	stbi_image_free(pixels);
	return true;
}

bbe::Image::Image()
{
}

bbe::Image::Image(const char *path)
{
	load(path);
}

bbe::Image::Image(const bbe::String &path)
{
	load(path);
}

bbe::Image::Image(int width, int height)
{
	load(width, height);
}

bbe::Image::Image(int width, int height, const Color &c)
{
	load(width, height, c);
}

bbe::Image::Image(int width, int height, const void *data, ImageFormat format)
{
	load(width, height, data, format);
}

bool bbe::Image::loadRaw(const bbe::ByteBuffer &buffer)
{
	return loadRaw(buffer.getRaw(), buffer.getLength());
}

bool bbe::Image::loadRaw(const bbe::List<unsigned char> &rawData)
{
	return loadRaw(rawData.getRaw(), rawData.getLength());
}

bool bbe::Image::loadRaw(const unsigned char *rawData, size_t dataLength)
{
	m_prendererData = nullptr;
	int texChannels = 0;
	stbi_uc *pixels = stbi_load_from_memory(rawData, (int)dataLength, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	return finishLoad(pixels);
}

void bbe::Image::load(const char *path)
{
	m_prendererData = nullptr;
	int texChannels = 0;
	stbi_uc *pixels = stbi_load(path, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	finishLoad(pixels);
}

void bbe::Image::load(const bbe::String &path)
{
	load(path.getRaw());
}

void bbe::Image::load(int width, int height)
{
	load(width, height, Color());
}

void bbe::Image::load(int width, int height, const Color &c)
{
	m_width = width;
	m_height = height;
	m_format = ImageFormat::R8G8B8A8;
	m_prendererData = nullptr;

	const size_t size = getSizeInBytes();
	m_pdata.resizeCapacityAndLengthUninit(size); //TODO use allocator
	for (size_t i = 0; i < size; i += 4)
	{
#ifdef _MSC_VER
		// MSVC doesn't understand that getSizeInBytes() will always
		// return a multiple of four, becuase m_format is R8G8B8A8.
#pragma warning(push)
#pragma warning(disable : 6386)
#endif
		m_pdata[i + 0] = (byte)(c.r * 255);
		m_pdata[i + 1] = (byte)(c.g * 255);
		m_pdata[i + 2] = (byte)(c.b * 255);
		m_pdata[i + 3] = (byte)(c.a * 255);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	}
}

void bbe::Image::load(int width, int height, const void *data, ImageFormat format)
{
	m_width = width;
	m_height = height;
	m_format = format;
	m_prendererData = nullptr;

	const size_t size = getSizeInBytes();
	if (size != 0 && data == nullptr)
	{
		bbe::Crash(bbe::Error::NullPointer);
	}

	m_pdata.resizeCapacityAndLengthUninit(size);
	memcpy(m_pdata.getRaw(), data, size);
}

int bbe::Image::getWidth() const
{
	return m_width;
}

int bbe::Image::getHeight() const
{
	return m_height;
}

bbe::Vector2i bbe::Image::getDimensions() const
{
	return Vector2i(getWidth(), getHeight());
}

size_t bbe::Image::getSizeInBytes() const
{
	if (m_width < 0 || m_height < 0)
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Image dimensions must not be negative.");
	}

	size_t size = static_cast<size_t>(m_width);
	if (!checkedMultiplySizeT(size, static_cast<size_t>(m_height)) || !checkedMultiplySizeT(size, getAmountOfChannels()) || !checkedMultiplySizeT(size, getBytesPerChannel()))
	{
		bbe::Crash(bbe::Error::OutOfMemory, "Image dimensions overflow size calculations.");
	}

	return size;
}

size_t bbe::Image::getAmountOfChannels() const
{
	switch (m_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8A8:
		return 4;
	case ImageFormat::R32FLOAT:
		return 1;
	case ImageFormat::R32G32B32A32FLOAT:
		return 4;
	default:
		bbe::Crash(bbe::Error::FormatNotSupported);
	}
}

size_t bbe::Image::getBytesPerChannel() const
{
	switch (m_format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8B8A8:
		return 1;
	case ImageFormat::R32FLOAT:
		return 4;
	case ImageFormat::R32G32B32A32FLOAT:
		return 4;
	default:
		bbe::Crash(bbe::Error::FormatNotSupported);
	}
}

bbe::Colori bbe::Image::getPixel(size_t x, size_t y) const
{
	if (!isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch (m_format)
	{
	case ImageFormat::R8:
		return Colori(m_pdata[index], m_pdata[index], m_pdata[index], 255);
	case ImageFormat::R8G8B8A8:
		return Colori(m_pdata[index], m_pdata[index + 1], m_pdata[index + 2], m_pdata[index + 3]);
	default:
		bbe::Crash(bbe::Error::FormatNotSupported);
	}
}

bbe::Colori bbe::Image::sampleBilinearPremultiplied(float sx, float sy) const
{
	if (!isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	const int32_t w = getWidth();
	const int32_t h = getHeight();
	if (w <= 0 || h <= 0)
	{
		return bbe::Colori(0, 0, 0, 0);
	}

	const int32_t ix0 = (int32_t)std::floor(sx);
	const int32_t iy0 = (int32_t)std::floor(sy);
	const float fx = sx - (float)ix0;
	const float fy = sy - (float)iy0;
	const float iFx = 1.f - fx;
	const float iFy = 1.f - fy;

	auto get = [&](int32_t x, int32_t y) -> bbe::Colori
	{
		if (x < 0 || y < 0 || x >= w || y >= h) return bbe::Colori(0, 0, 0, 0);
		return getPixel((size_t)x, (size_t)y);
	};

	const bbe::Colori c00 = get(ix0, iy0);
	const bbe::Colori c10 = get(ix0 + 1, iy0);
	const bbe::Colori c01 = get(ix0, iy0 + 1);
	const bbe::Colori c11 = get(ix0 + 1, iy0 + 1);

	const float a00 = c00.a, a10 = c10.a, a01 = c01.a, a11 = c11.a;
	const float pa = (a00 * iFx + a10 * fx) * iFy + (a01 * iFx + a11 * fx) * fy;
	if (pa < 0.5f) return bbe::Colori(0, 0, 0, 0);

	const float pr = (c00.r * a00 * iFx + c10.r * a10 * fx) * iFy + (c01.r * a01 * iFx + c11.r * a11 * fx) * fy;
	const float pg = (c00.g * a00 * iFx + c10.g * a10 * fx) * iFy + (c01.g * a01 * iFx + c11.g * a11 * fx) * fy;
	const float pb = (c00.b * a00 * iFx + c10.b * a10 * fx) * iFy + (c01.b * a01 * iFx + c11.b * a11 * fx) * fy;

	const float invA = 1.f / pa;
	return bbe::Colori(
		(bbe::byte)(pr * invA + 0.5f),
		(bbe::byte)(pg * invA + 0.5f),
		(bbe::byte)(pb * invA + 0.5f),
		(bbe::byte)(pa + 0.5f)
	);
}

void bbe::Image::blendOver(const bbe::Image &src, const bbe::Vector2i &dstPos, bool tiled)
{
	if (!isLoadedCpu() || !src.isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_format != bbe::ImageFormat::R8G8B8A8 || src.m_format != bbe::ImageFormat::R8G8B8A8)
	{
		bbe::Crash(bbe::Error::FormatNotSupported);
	}

	const int32_t dstW = getWidth();
	const int32_t dstH = getHeight();
	const int32_t srcW = src.getWidth();
	const int32_t srcH = src.getHeight();
	if (dstW <= 0 || dstH <= 0 || srcW <= 0 || srcH <= 0) return;

	for (int32_t sy = 0; sy < srcH; sy++)
	{
		for (int32_t sx = 0; sx < srcW; sx++)
		{
			int32_t dx = dstPos.x + sx;
			int32_t dy = dstPos.y + sy;
			if (!toTiledPos(dx, dy, dstW, dstH, tiled)) continue;

			const bbe::Colori sc = src.getPixel((size_t)sx, (size_t)sy);
			if (sc.a == 0) continue;
			const bbe::Colori dc = getPixel((size_t)dx, (size_t)dy);
			setPixel((size_t)dx, (size_t)dy, dc.blendTo(sc));
		}
	}
}

void bbe::Image::blend(const bbe::Image &src, float opacity, bbe::BlendMode mode)
{
	if (!isLoadedCpu() || !src.isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_format != bbe::ImageFormat::R8G8B8A8 || src.m_format != bbe::ImageFormat::R8G8B8A8)
	{
		bbe::Crash(bbe::Error::FormatNotSupported);
	}
	if (src.getWidth() != getWidth() || src.getHeight() != getHeight())
	{
		bbe::Crash(bbe::Error::IllegalArgument, "blend(): image sizes must match.");
	}

	const int32_t w = getWidth();
	const int32_t h = getHeight();
	if (w <= 0 || h <= 0) return;

	opacity = bbe::Math::clamp01(opacity);
	if (opacity <= 0.f) return;

	for (int32_t y = 0; y < h; y++)
	{
		for (int32_t x = 0; x < w; x++)
		{
			const bbe::Colori s = src.getPixel((size_t)x, (size_t)y);
			if (s.a == 0) continue;
			const bbe::Colori d = getPixel((size_t)x, (size_t)y);
			setPixel((size_t)x, (size_t)y, d.blendTo(s, opacity, mode));
		}
	}
}

void bbe::Image::blendOverRotated(const bbe::Image &src, const bbe::Rectanglei &dstRect, float rotation, bool tiled, bool antiAlias)
{
	if (!isLoadedCpu() || !src.isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_format != bbe::ImageFormat::R8G8B8A8 || src.m_format != bbe::ImageFormat::R8G8B8A8)
	{
		bbe::Crash(bbe::Error::FormatNotSupported);
	}

	const int32_t dstW = getWidth();
	const int32_t dstH = getHeight();
	const int32_t srcW = src.getWidth();
	const int32_t srcH = src.getHeight();
	if (dstW <= 0 || dstH <= 0 || srcW <= 0 || srcH <= 0) return;

	const float cx = dstRect.x + dstRect.width / 2.f;
	const float cy = dstRect.y + dstRect.height / 2.f;
	const float cosA = std::cos(-rotation);
	const float sinA = std::sin(-rotation);

	const bbe::Rectangle srcRect(cx - srcW * 0.5f, cy - srcH * 0.5f, (float)srcW, (float)srcH);
	const bbe::Rectanglei bb = srcRect.computeRotatedBoundsAfterRotation(rotation, bbe::Vector2(cx, cy));
	const float srcCX = (srcW - 1) / 2.f;
	const float srcCY = (srcH - 1) / 2.f;

	for (int32_t y = bb.y; y <= bb.y + bb.height - 1; y++)
	{
		for (int32_t x = bb.x; x <= bb.x + bb.width - 1; x++)
		{
			const float dx = x + 0.5f - cx;
			const float dy = y + 0.5f - cy;
			const float srcX = dx * cosA - dy * sinA + srcCX;
			const float srcY = dx * sinA + dy * cosA + srcCY;

			bbe::Colori sp = src.sampleBilinearPremultiplied(srcX, srcY);
			if (!antiAlias)
			{
				if (sp.a == 0) continue;
				sp.a = 255;
			}
			if (sp.a == 0) continue;

			int32_t tx = x;
			int32_t ty = y;
			if (!toTiledPos(tx, ty, dstW, dstH, tiled)) continue;

			const bbe::Colori dc = getPixel((size_t)tx, (size_t)ty);
			setPixel((size_t)tx, (size_t)ty, dc.blendTo(sp));
		}
	}
}

bool bbe::Image::drawBrushStamp(const bbe::Vector2 &pos, const bbe::Colori &color, int32_t brushRadius, ImageBrushShape shape, bool tiled, bool antiAlias)
{
	if (!isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_format != bbe::ImageFormat::R8G8B8A8)
	{
		bbe::Crash(bbe::Error::FormatNotSupported);
	}
	if (brushRadius < 0) return false;
	const int32_t w = getWidth();
	const int32_t h = getHeight();
	if (w <= 0 || h <= 0) return false;

	const float effX = antiAlias ? pos.x : std::floor(pos.x) + 0.5f;
	const float effY = antiAlias ? pos.y : std::floor(pos.y) + 0.5f;

	bool changed = false;
	for (int32_t oy = -brushRadius; oy <= brushRadius; oy++)
	{
		for (int32_t ox = -brushRadius; ox <= brushRadius; ox++)
		{
			const float logicalX = std::floor(effX + (float)ox) + 0.5f;
			const float logicalY = std::floor(effY + (float)oy) + 0.5f;
			const float dx = logicalX - effX;
			const float dy = logicalY - effY;

			float strength;
			if (shape == bbe::ImageBrushShape::Square)
			{
				const float maxD = std::max(std::fabsf(dx), std::fabsf(dy));
				strength = bbe::Math::clamp01((float)brushRadius - maxD);
			}
			else
			{
				strength = bbe::Math::clamp01((float)brushRadius - bbe::Math::sqrt(dx * dx + dy * dy));
			}
			if (strength <= 0.f) continue;

			int32_t px = (int32_t)std::floor(pos.x + (float)ox);
			int32_t py = (int32_t)std::floor(pos.y + (float)oy);
			if (!toTiledPos(px, py, w, h, tiled)) continue;

			bbe::Colori nc = color;
			nc.a = (bbe::byte)(nc.MAXIMUM_VALUE * strength);
			const bbe::Colori oc = getPixel((size_t)px, (size_t)py);
			if (nc.a > oc.a)
			{
				setPixel((size_t)px, (size_t)py, nc);
				changed = true;
			}
		}
	}
	return changed;
}

bool bbe::Image::drawLineCapsule(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color, int32_t brushRadius, ImageBrushShape shape, bool tiled, bool antiAlias)
{
	if (!isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_format != bbe::ImageFormat::R8G8B8A8)
	{
		bbe::Crash(bbe::Error::FormatNotSupported);
	}
	if (brushRadius < 0) return false;

	// Keep crisp results: tiled, square brushes, or AA-off uses stamp-based iteration.
	if (tiled || shape == bbe::ImageBrushShape::Square || !antiAlias)
	{
		bool changed = false;
		bbe::GridIterator gi(from, to);
		while (gi.hasNext())
		{
			const bbe::Vector2 p = gi.next().as<float>();
			changed |= drawBrushStamp(p, color, brushRadius, shape, tiled, antiAlias);
		}
		return changed;
	}

	const int32_t w = getWidth();
	const int32_t h = getHeight();
	if (w <= 0 || h <= 0) return false;

	const float ax = to.x - from.x;
	const float ay = to.y - from.y;
	const float lenSq = ax * ax + ay * ay;

	const float margin = (float)brushRadius + 1.f;
	const int32_t xMin = (int32_t)std::floor(std::min(from.x, to.x) - margin);
	const int32_t xMax = (int32_t)std::ceil(std::max(from.x, to.x) + margin);
	const int32_t yMin = (int32_t)std::floor(std::min(from.y, to.y) - margin);
	const int32_t yMax = (int32_t)std::ceil(std::max(from.y, to.y) + margin);

	bool changed = false;
	for (int32_t y = yMin; y <= yMax; y++)
	{
		for (int32_t x = xMin; x <= xMax; x++)
		{
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
			if (strength <= 0.f) continue;

			int32_t px = x;
			int32_t py = y;
			if (!toTiledPos(px, py, w, h, false)) continue;

			bbe::Colori nc = color;
			nc.a = (bbe::byte)(nc.MAXIMUM_VALUE * strength);
			const bbe::Colori oc = getPixel((size_t)px, (size_t)py);
			if (nc.a > oc.a)
			{
				setPixel((size_t)px, (size_t)py, nc);
				changed = true;
			}
		}
	}
	return changed;
}

void bbe::Image::fillTriangle(const bbe::Vector2 &v0, const bbe::Vector2 &v1, const bbe::Vector2 &v2, const bbe::Colori &color, bool tiled, bool antiAlias)
{
	if (!isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}
	if (m_format != bbe::ImageFormat::R8G8B8A8)
	{
		bbe::Crash(bbe::Error::FormatNotSupported);
	}

	const int32_t w = getWidth();
	const int32_t h = getHeight();
	if (w <= 0 || h <= 0) return;

	auto edgeDist = [](float ax, float ay, float bx, float by, float px, float py) -> float
	{
		const float ex = bx - ax, ey = by - ay;
		const float len = bbe::Math::sqrt(ex * ex + ey * ey);
		if (len < 1e-6f) return 0.f;
		return (ex * (py - ay) - ey * (px - ax)) / len;
	};

	// Ensure CCW winding.
	const float area2 = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
	const bbe::Vector2 a = v0;
	const bbe::Vector2 b = area2 >= 0.f ? v1 : v2;
	const bbe::Vector2 c = area2 >= 0.f ? v2 : v1;

	const int32_t x0 = (int32_t)bbe::Math::floor(bbe::Math::min(bbe::Math::min(a.x, b.x), c.x) - 1.f);
	const int32_t x1 = (int32_t)bbe::Math::ceil(bbe::Math::max(bbe::Math::max(a.x, b.x), c.x) + 1.f);
	const int32_t y0 = (int32_t)bbe::Math::floor(bbe::Math::min(bbe::Math::min(a.y, b.y), c.y) - 1.f);
	const int32_t y1 = (int32_t)bbe::Math::ceil(bbe::Math::max(bbe::Math::max(a.y, b.y), c.y) + 1.f);

	for (int32_t y = y0; y <= y1; y++)
	{
		for (int32_t x = x0; x <= x1; x++)
		{
			const float px = x + 0.5f;
			const float py = y + 0.5f;
			const float d0 = edgeDist(a.x, a.y, b.x, b.y, px, py);
			const float d1 = edgeDist(b.x, b.y, c.x, c.y, px, py);
			const float d2 = edgeDist(c.x, c.y, a.x, a.y, px, py);
			const float minD = d0 < d1 ? (d0 < d2 ? d0 : d2) : (d1 < d2 ? d1 : d2);
			const float alphaAA = bbe::Math::clamp01(minD + 0.5f);
			if (alphaAA <= 0.f) continue;

			const float finalAlpha = antiAlias ? alphaAA : (alphaAA > 0.5f ? 1.0f : 0.0f);
			if (finalAlpha <= 0.f) continue;

			int32_t tx = x;
			int32_t ty = y;
			if (!toTiledPos(tx, ty, w, h, tiled)) continue;

			bbe::Colori pix = color;
			pix.a = (bbe::byte)(pix.a * finalAlpha);
			const bbe::Colori old = getPixel((size_t)tx, (size_t)ty);
			if (pix.a > old.a)
			{
				setPixel((size_t)tx, (size_t)ty, pix);
			}
		}
	}
}

void bbe::Image::setPixel(const bbe::Vector2i &pos, const bbe::Colori &c)
{
	setPixel(pos.x, pos.y, c);
}

void bbe::Image::setPixel(size_t x, size_t y, const bbe::Colori &c)
{
	if (!isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch (m_format)
	{
	case ImageFormat::R8:
		m_pdata[index] = c.r;
		break;
	case ImageFormat::R8G8B8A8:
		m_pdata[index + 0] = c.r;
		m_pdata[index + 1] = c.g;
		m_pdata[index + 2] = c.b;
		m_pdata[index + 3] = c.a;
		break;
	default:
		bbe::Crash(bbe::Error::FormatNotSupported);
	}

	m_prendererData = nullptr;
}

size_t bbe::Image::getIndexForRawAccess(size_t x, size_t y) const
{
	if (m_width < 0 || m_height < 0)
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Image dimensions must not be negative.");
	}

	const size_t width = static_cast<size_t>(m_width);
	const size_t height = static_cast<size_t>(m_height);
	if (x >= width || y >= height)
	{
		bbe::Crash(bbe::Error::IllegalArgument, "Image pixel access is out of bounds.");
	}

	size_t index = y;
	if (!checkedMultiplySizeT(index, width))
	{
		bbe::Crash(bbe::Error::OutOfMemory);
	}
	if (index > std::numeric_limits<size_t>::max() - x)
	{
		bbe::Crash(bbe::Error::OutOfMemory);
	}
	index += x;
	if (!checkedMultiplySizeT(index, getAmountOfChannels()))
	{
		bbe::Crash(bbe::Error::OutOfMemory);
	}

	return index;
}

int64_t bbe::Image::distance(const Image &other) const
{
	if (other.getWidth() != getWidth() || other.getHeight() != getHeight())
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	int64_t retVal = 0;

	for (size_t i = 0; i < getWidth(); i++)
	{
		for (size_t k = 0; k < getHeight(); k++)
		{
			auto myColor = getPixel(i, k);
			auto otherColor = other.getPixel(i, k);
			retVal += myColor.distance(otherColor);
		}
	}
	return retVal;
}

bbe::ImageRepeatMode bbe::Image::getRepeatMode() const
{
	return m_repeatMode;
}

const bbe::Image &bbe::Image::white()
{
	static bbe::Image image;
	if (!image.isLoadedCpu() && !image.isLoadedGpu())
	{
		byte pixel[] = { 255, 255, 255, 255 };
		image.load(1, 1, pixel, bbe::ImageFormat::R8G8B8A8);
	}
	return image;
}

const bbe::Image &bbe::Image::black()
{
	static bbe::Image image;
	if (!image.isLoadedCpu() && !image.isLoadedGpu())
	{
		byte pixel[] = { 0, 0, 0, 255 };
		image.load(1, 1, pixel, bbe::ImageFormat::R8G8B8A8);
	}
	return image;
}

void bbe::Image::setRepeatMode(ImageRepeatMode irm)
{
	if (m_prendererData != nullptr)
	{
		bbe::Crash(bbe::Error::AlreadyUploaded);
	}

	m_repeatMode = irm;
}

bbe::ImageFilterMode bbe::Image::getFilterMode() const
{
	return m_filterMode;
}

void bbe::Image::setFilterMode(ImageFilterMode ifm)
{
	if (m_prendererData != nullptr)
	{
		bbe::Crash(bbe::Error::AlreadyUploaded);
	}

	m_filterMode = ifm;
}

void bbe::Image::keepAfterUpload()
{
	keep = true;
}

size_t bbe::Image::getBytesPerPixel() const
{
	return getBytesPerChannel() * getAmountOfChannels();
}

void bbe::Image::flipHorizontally()
{
	bbe::List<byte> rowBuffer;
	const size_t bytesPerRow = getWidth() * getBytesPerPixel();
	rowBuffer.resizeCapacity(bytesPerRow);
	for (size_t row = 0; row < getHeight() / 2; row++)
	{
		const size_t lowerRow = getHeight() - 1 - row;
		void *rowPtr = m_pdata.getRaw() + row * bytesPerRow;
		void *rowLowerPtr = m_pdata.getRaw() + lowerRow * bytesPerRow;
		memcpy(rowBuffer.getRaw(), rowPtr, bytesPerRow);
		memcpy(rowPtr, rowLowerPtr, bytesPerRow);
		memcpy(rowLowerPtr, rowBuffer.getRaw(), bytesPerRow);
	}
}

bool bbe::Image::supportsClipboardImages()
{
#ifdef _WIN32
	return true;
#elif defined(__linux__)
	return bbe::INTERNAL::waylandClipboard::isSupported();
#else
	return false;
#endif
}

#ifdef __linux__
bool bbe::Image::isImageInClipbaord()
{
	return bbe::INTERNAL::waylandClipboard::isImageInClipboard();
}

bbe::Image bbe::Image::getClipboardImage()
{
	const bbe::ByteBuffer data = bbe::INTERNAL::waylandClipboard::getClipboardImageData();
	if (data.getLength() == 0)
	{
		return bbe::Image();
	}

	bbe::Image retVal;
	if (!retVal.loadRaw(data))
	{
		return bbe::Image();
	}

	return retVal;
}

void bbe::Image::copyToClipboard() const
{
	const std::vector<bbe::byte> pngData = encodeRawImageAsPng(
		m_pdata.getRaw(),
		getWidth(),
		getHeight(),
		getAmountOfChannels(),
		getBytesPerChannel());
	if (pngData.empty())
	{
		return;
	}

	bbe::INTERNAL::waylandClipboard::setClipboardImageData(pngData.data(), pngData.size(), "image/png");
}
#elif !defined(_WIN32)
bool bbe::Image::isImageInClipbaord()
{
	return false;
}

bbe::Image bbe::Image::getClipboardImage()
{
	return bbe::Image();
}

void bbe::Image::copyToClipboard() const
{
}
#endif

#ifdef _WIN32
bool bbe::Image::isImageInClipbaord()
{
	return ::IsClipboardFormatAvailable(CF_BITMAP);
}

bbe::Image bbe::Image::getClipboardImage()
{
	if (!isImageInClipbaord()) return bbe::Image();
	if (!OpenClipboard(0)) return bbe::Image();

	bbe::Image retVal;
	HBITMAP hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
	if (hBitmap && hBitmap != INVALID_HANDLE_VALUE)
	{
		BITMAP bitmap;
		GetObject(hBitmap, sizeof(bitmap), &bitmap);

		if (bitmap.bmPlanes == 1 && bitmap.bmBitsPixel == 32)
		{
			retVal = bbe::Image(bitmap.bmWidth, bitmap.bmHeight);

			BITMAPINFO info = {};
			HDC dc = CreateCompatibleDC(0);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(dc, hBitmap);

			info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			info.bmiHeader.biWidth = bitmap.bmWidth;
			info.bmiHeader.biHeight = bitmap.bmHeight;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = bitmap.bmBitsPixel;
			info.bmiHeader.biCompression = BI_RGB;
			info.bmiHeader.biSizeImage = bitmap.bmWidth * bitmap.bmHeight * 4;

			bbe::List<byte> &bytes = retVal.m_pdata;
			GetDIBits(dc, hBitmap, 0, bitmap.bmHeight, bytes.getRaw(), &info, DIB_RGB_COLORS);

			//Windows gives us BGR, but we want RGB, so flip R and B channels:
			for (size_t i = 0; i < bytes.getLength(); i += 4)
			{
				auto temp = bytes[i];
				bytes[i] = bytes[i + 2];
				bytes[i + 2] = temp;
			}

			retVal.flipHorizontally();

			SelectObject(dc, oldBitmap);

			DeleteDC(dc);
		}
	}

	CloseClipboard();

	return retVal;
}

HBITMAP bbe::Image::toBitmap() const
{
	// See: https://learn.microsoft.com/en-us/windows/win32/menurc/using-cursors#creating-a-cursor
	BITMAPV5HEADER bi = {};
	bi.bV5Size = sizeof(BITMAPV5HEADER);
	bi.bV5Width = getWidth();
	bi.bV5Height = getHeight();
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	// The following mask specification specifies a supported 32 BPP
	// alpha format for Windows XP.
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;
	bi.bV5AlphaMask = 0xFF000000;

	// Create the DIB section with an alpha channel.
	HDC hdc = GetDC(NULL);
	void *lpBits;
	HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS,
									   &lpBits, NULL, (DWORD)0);
	ReleaseDC(NULL, hdc);

	// Set the alpha values for each pixel in the cursor so that
	// the complete cursor is semi-transparent.
	DWORD *lpdwPixel = (DWORD *)lpBits;
	for (int32_t y = getHeight() - 1; y >= 0; y--)
	{
		for (int32_t x = 0; x < getWidth(); x++)
		{
			const Colori c = getPixel(x, y);

			*lpdwPixel = c.b;
			*lpdwPixel |= c.g << 8;
			*lpdwPixel |= c.r << 16;
			*lpdwPixel |= c.a << 24;

			lpdwPixel++;
		}
	}

	return hBitmap;
}

void bbe::Image::copyToClipboard() const
{
	// TODO: Investigate - Why is all this crap even needed... why can't we just put the bitmap into the clipboard directly?
	HBITMAP hBitmap = toBitmap();
	HBITMAP hBitmap_copy = CreateBitmap(getWidth(), getHeight(), 1, 32, NULL);
	HDC srcDC = CreateCompatibleDC(GetDC(NULL));
	HDC newDC = CreateCompatibleDC(GetDC(NULL));
	HBITMAP srcBitmap = (HBITMAP)SelectObject(srcDC, hBitmap);
	HBITMAP newBitmap = (HBITMAP)SelectObject(newDC, hBitmap_copy);
	BitBlt(newDC, 0, 0, getWidth(), getHeight(), srcDC, 0, 0, SRCCOPY);
	SelectObject(srcDC, srcBitmap);
	SelectObject(newDC, newBitmap);
	DeleteDC(srcDC);
	DeleteDC(newDC);

	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hBitmap_copy);
	CloseClipboard();
}

HICON bbe::Image::toIcon() const
{
	HBITMAP hBitmap = toBitmap();

	HBITMAP hMonoBitmap = CreateBitmap(getWidth(), getHeight(), 1, 1, NULL);
	ICONINFO ii = {};
	ii.fIcon = TRUE;
	ii.hbmMask = hMonoBitmap;
	ii.hbmColor = hBitmap;

	// Create the alpha cursor with the alpha DIB section.
	HICON hAlphaCursor = CreateIconIndirect(&ii);

	DeleteObject(hBitmap);
	DeleteObject(hMonoBitmap);

	return hAlphaCursor;
}
#endif

void bbe::Image::writeToFile(const char *path) const
{
	const bbe::String lowerPath = bbe::String(path).toLowerCase();
	if (lowerPath.endsWith(".png"))
	{
		stbi_write_png(path, m_width, m_height, (int)getAmountOfChannels(), m_pdata.getRaw(), 0);
	}
	else if (lowerPath.endsWith(".bmp"))
	{
		stbi_write_bmp(path, m_width, m_height, (int)getAmountOfChannels(), m_pdata.getRaw());
	}
	else if (lowerPath.endsWith(".tga"))
	{
		stbi_write_tga(path, m_width, m_height, (int)getAmountOfChannels(), m_pdata.getRaw());
	}
	else if (lowerPath.endsWith(".jpg"))
	{
		stbi_write_jpg(path, m_width, m_height, (int)getAmountOfChannels(), m_pdata.getRaw(), 90);
	}
	else
	{
		bbe::Crash(bbe::Error::NotImplemented);
	}
}

void bbe::Image::writeToFile(const bbe::String &path) const
{
	writeToFile(path.getRaw());
}

static void floodFillStep(bbe::Image &image, bbe::List<bbe::Vector2i> &posToCheck, bbe::Vector2i /*copy*/ pos, const bbe::Colori &from, const bbe::Colori &to, bool tiled)
{
	if (pos.x < 0 || pos.x >= image.getWidth() || pos.y < 0 || pos.y >= image.getHeight())
	{
		if (!tiled) return;
		if (pos.x < 0) pos.x = image.getWidth() - 1;
		if (pos.y < 0) pos.y = image.getHeight() - 1;
		if (pos.x >= image.getWidth()) pos.x = 0;
		if (pos.y >= image.getHeight()) pos.y = 0;
	}
	if (image.getPixel(pos) != from) return;
	image.setPixel(pos, to);
	posToCheck.add(pos);
}

void bbe::Image::floodFill(const bbe::Vector2i &pos, const bbe::Colori &to, bool fillDiagonal, bool tiled)
{
	const bbe::Colori from = getPixel(pos);
	if (from == to) return;
	bbe::List<bbe::Vector2i> posToCheck;
	floodFillStep(*this, posToCheck, pos, from, to, tiled);

	while (posToCheck.getLength() > 0)
	{
		const bbe::Vector2i pos = posToCheck.popBack();
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x + 1, pos.y), from, to, tiled);
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x - 1, pos.y), from, to, tiled);
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x, pos.y + 1), from, to, tiled);
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x, pos.y - 1), from, to, tiled);
		if (fillDiagonal)
		{
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x + 1, pos.y + 1), from, to, tiled);
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x - 1, pos.y + 1), from, to, tiled);
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x + 1, pos.y - 1), from, to, tiled);
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x - 1, pos.y - 1), from, to, tiled);
		}
	}
}

#ifdef _WIN32
bbe::Image bbe::Image::screenshot(int x, int y, int width, int height)
{
	HDC screenDC = GetDC(0);
	HBITMAP bitmap = CreateCompatibleBitmap(screenDC, width, height);
	HDC memDC = CreateCompatibleDC(screenDC);
	HBITMAP memBitmap = static_cast<HBITMAP>(SelectObject(memDC, bitmap));
	BitBlt(memDC, 0, 0, width, height, screenDC, x, y, SRCCOPY);
	bitmap = static_cast<HBITMAP>(SelectObject(memDC, memBitmap));

	BITMAPINFO info = {};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = width * height * 4;

	bbe::List<unsigned char> p;
	p.resizeCapacityAndLengthUninit(info.bmiHeader.biSizeImage);
	GetDIBits(memDC, bitmap, 0, height, &p[0], &info, DIB_RGB_COLORS);
	HBITMAP OldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
	SelectObject(memDC, OldBitmap);

	for (size_t i = 0; i < p.getLength(); i += 4)
	{
		auto temp = p[i];
		p[i] = p[i + 2];
		p[i + 2] = temp;
	}

	bbe::Image image(width, height, p.getRaw(), bbe::ImageFormat::R8G8B8A8);
	for (size_t i = 0; i < width; i++)
	{
		for (size_t k = 0; k < height / 2; k++)
		{
			auto temp = image.getPixel(i, k);
			image.setPixel(i, k, image.getPixel(i, height - k - 1));
			image.setPixel(i, height - k - 1, temp);
		}
	}

	DeleteDC(memDC);
	DeleteDC(screenDC);
	DeleteObject(bitmap);
	DeleteObject(memBitmap);
	return image;
}
#endif

bool bbe::Image::isLoadedCpu() const
{
	return m_pdata.getLength() > 0;
}

bbe::Colori bbe::Image::getPixel(const bbe::Vector2i &pos) const
{
	return getPixel(pos.x, pos.y);
}

bool bbe::Image::isLoadedGpu() const
{
	return m_prendererData != nullptr;
}
