#include "BBE/Image.h"
#include "BBE/Exceptions.h"
#include "BBE/Math.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#ifdef _WIN32
#include <Windows.h>
#endif

void bbe::Image::finishLoad(stbi_uc* pixels)
{
	m_format = ImageFormat::R8G8B8A8; // Is correct, even if texChannels == 3, because stbi is transforming the data for us on the fly.

	if (pixels == nullptr)
	{
		throw LoadException();
	}

	m_pdata.resizeCapacityAndLengthUninit(getSizeInBytes());
	memcpy(m_pdata.getRaw(), pixels, getSizeInBytes());

	stbi_image_free(pixels);
}

bbe::Image::Image()
{
}

bbe::Image::Image(const char * path)
{
	load(path);
}

bbe::Image::Image(const bbe::String& path)
{
	load(path);
}

bbe::Image::Image(int width, int height)
{
	load(width, height);
}

bbe::Image::Image(int width, int height, const Color & c)
{
	load(width, height, c);
}

bbe::Image::Image(int width, int height, const void* data, ImageFormat format)
{
	load(width, height, data, format);
}

void bbe::Image::loadRaw(const bbe::ByteBuffer& buffer)
{
	loadRaw(buffer.getRaw(), buffer.getLength());
}

void bbe::Image::loadRaw(const bbe::List<unsigned char>& rawData)
{
	loadRaw(rawData.getRaw(), rawData.getLength());
}

void bbe::Image::loadRaw(const unsigned char* rawData, size_t dataLength)
{
	m_prendererData = nullptr;
	int texChannels = 0;
	stbi_uc* pixels = stbi_load_from_memory(rawData, (int)dataLength, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	finishLoad(pixels);
}

void bbe::Image::load(const char * path)
{
	m_prendererData = nullptr;
	int texChannels = 0;
	stbi_uc *pixels = stbi_load(path, &m_width, &m_height, &texChannels, STBI_rgb_alpha);
	finishLoad(pixels);
}

void bbe::Image::load(const bbe::String& path)
{
	load(path.getRaw());
}

void bbe::Image::load(int width, int height)
{
	load(width, height, Color());
}

void bbe::Image::load(int width, int height, const Color & c)
{
	m_width = width;
	m_height = height;
	m_format = ImageFormat::R8G8B8A8;
	m_prendererData = nullptr;

	const size_t size = getSizeInBytes();
	m_pdata.resizeCapacityAndLengthUninit(size); //TODO use allocator
	for (int i = 0; i < size; i+=4)
	{
#ifdef _MSC_VER
		// MSVC doesn't understand that getSizeInBytes() will always
		// return a multiple of four, becuase m_format is R8G8B8A8.
#pragma warning( push )
#pragma warning( disable : 6386)
#endif
		m_pdata[i + 0] = (byte)(c.r * 255);
		m_pdata[i + 1] = (byte)(c.g * 255);
		m_pdata[i + 2] = (byte)(c.b * 255);
		m_pdata[i + 3] = (byte)(c.a * 255);
#ifdef _MSC_VER
#pragma warning( pop ) 
#endif
	}
}

void bbe::Image::load(int width, int height, const void* data, ImageFormat format)
{
	m_width = width;
	m_height = height;
	m_format = format;
	m_prendererData = nullptr;

	m_pdata.resizeCapacityAndLengthUninit(getSizeInBytes());
	memcpy(m_pdata.getRaw(), data, getSizeInBytes());
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
	return static_cast<size_t>(getWidth() * getHeight() * getAmountOfChannels() * getBytesPerChannel());
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
		throw FormatNotSupportedException();
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
		throw FormatNotSupportedException();
	}
}

bbe::Color bbe::Image::getPixel(size_t x, size_t y) const
{
	if (!isLoadedCpu())
	{
		throw NotInitializedException();
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch(m_format)
	{
	case ImageFormat::R8:
		return Color(m_pdata[index] / 255.f, m_pdata[index] / 255.f, m_pdata[index] / 255.f, 1.0f);
	case ImageFormat::R8G8B8A8:
		return Color(m_pdata[index] / 255.f, m_pdata[index + 1] / 255.f, m_pdata[index + 2] / 255.f, m_pdata[index + 3] / 255.f);
	default:
		throw FormatNotSupportedException();
	}
	
}

void bbe::Image::setPixel(size_t x, size_t y, Color c)
{
	if (!isLoadedCpu())
	{
		throw NotInitializedException();
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch (m_format)
	{
	case ImageFormat::R8:
		m_pdata[index + 0] = c.r * 255.f;
		m_pdata[index + 1] = c.r * 255.f;
		m_pdata[index + 2] = c.r * 255.f;
		break;
	case ImageFormat::R8G8B8A8:
		m_pdata[index + 0] = c.r * 255.f;
		m_pdata[index + 1] = c.g * 255.f;
		m_pdata[index + 2] = c.b * 255.f;
		m_pdata[index + 3] = c.a * 255.f;
		break;
	default:
		throw FormatNotSupportedException();
	}
}

size_t bbe::Image::getIndexForRawAccess(size_t x, size_t y) const
{
	return (y * m_width + x) * getAmountOfChannels();
}

bbe::ImageRepeatMode bbe::Image::getRepeatMode() const
{
	return m_repeatMode;
}

const bbe::Image& bbe::Image::white()
{
	static bbe::Image image;
	if (!image.isLoadedCpu() && !image.isLoadedGpu())
	{
		byte pixel[] = { 255, 255, 255, 255 };
		image.load(1, 1, pixel, bbe::ImageFormat::R8G8B8A8);
	}
	return image;
}

const bbe::Image& bbe::Image::black()
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
		throw AlreadyUploadedException();
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
		throw AlreadyUploadedException();
	}

	m_filterMode = ifm;
}

void bbe::Image::keepAfterUpload()
{
	keep = true;
}

HICON bbe::Image::toIcon() const
{
	// See: https://learn.microsoft.com/en-us/windows/win32/menurc/using-cursors#creating-a-cursor
	const DWORD iconWidth = getWidth();
	const DWORD iconHeight = getHeight();

	BITMAPV5HEADER bi = {};
	bi.bV5Size = sizeof(BITMAPV5HEADER);
	bi.bV5Width = iconWidth;
	bi.bV5Height = iconHeight;
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
	void* lpBits;
	HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
		&lpBits, NULL, (DWORD)0);
	ReleaseDC(NULL, hdc);

	// Create an empty mask bitmap.
	HBITMAP hMonoBitmap = CreateBitmap(iconWidth, iconHeight, 1, 1, NULL);

	// Set the alpha values for each pixel in the cursor so that
	// the complete cursor is semi-transparent.
	DWORD* lpdwPixel = (DWORD*)lpBits;
	for (DWORD x = 0; x < iconWidth; x++)
	{
		for (DWORD y = 0; y < iconHeight; y++)
		{
			const Color c = getPixel(x, y);
			const byte r = c.r * 255;
			const byte g = c.g * 255;
			const byte b = c.b * 255;
			const byte a = c.a * 255;

			*lpdwPixel  = b;
			*lpdwPixel |= g << 8;
			*lpdwPixel |= r << 16;
			*lpdwPixel |= a << 24;

			lpdwPixel++;
		}
		}

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

bool bbe::Image::isLoadedCpu() const
{
	return m_pdata.getLength() > 0;
}

bool bbe::Image::isLoadedGpu() const
{
	return m_prendererData != nullptr;
}
