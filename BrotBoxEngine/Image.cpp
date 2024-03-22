#include "BBE/Image.h"
#include "BBE/Exceptions.h"
#include "BBE/Math.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

bbe::Colori bbe::Image::getPixel(size_t x, size_t y) const
{
	if (!isLoadedCpu())
	{
		throw NotInitializedException();
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch(m_format)
	{
	case ImageFormat::R8:
		return Colori(m_pdata[index], m_pdata[index], m_pdata[index], 255);
	case ImageFormat::R8G8B8A8:
		return Colori(m_pdata[index], m_pdata[index + 1], m_pdata[index + 2], m_pdata[index + 3]);
	default:
		throw FormatNotSupportedException();
	}
	
}

void bbe::Image::setPixel(const bbe::Vector2i& pos, const bbe::Colori& c)
{
	setPixel(pos.x, pos.y, c);
}

void bbe::Image::setPixel(size_t x, size_t y, const bbe::Colori& c)
{
	if (!isLoadedCpu())
	{
		throw NotInitializedException();
	}

	const size_t index = getIndexForRawAccess(x, y);
	switch (m_format)
	{
	case ImageFormat::R8:
		m_pdata[index + 0] = c.r;
		m_pdata[index + 1] = c.r;
		m_pdata[index + 2] = c.r;
		break;
	case ImageFormat::R8G8B8A8:
		m_pdata[index + 0] = c.r;
		m_pdata[index + 1] = c.g;
		m_pdata[index + 2] = c.b;
		m_pdata[index + 3] = c.a;
		break;
	default:
		throw FormatNotSupportedException();
	}

	m_prendererData = nullptr;
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
		void* rowPtr      = m_pdata.getRaw() + row      * bytesPerRow;
		void* rowLowerPtr = m_pdata.getRaw() + lowerRow * bytesPerRow;
		memcpy(rowBuffer.getRaw(), rowPtr,             bytesPerRow);
		memcpy(rowPtr,             rowLowerPtr,        bytesPerRow);
		memcpy(rowLowerPtr,        rowBuffer.getRaw(), bytesPerRow);
	}
}

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

			bbe::List<byte>& bytes = retVal.m_pdata;
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
	void* lpBits;
	HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
		&lpBits, NULL, (DWORD)0);
	ReleaseDC(NULL, hdc);


	// Set the alpha values for each pixel in the cursor so that
	// the complete cursor is semi-transparent.
	DWORD* lpdwPixel = (DWORD*)lpBits;
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

void bbe::Image::writeToFile(const char* path) const
{
	const bbe::String lowerPath = bbe::String(path).toLowerCase();
	if (lowerPath.endsWith(".png"))
	{
		stbi_write_png(path, m_width, m_height, getAmountOfChannels(), m_pdata.getRaw(), 0);
	}
	else if (lowerPath.endsWith(".bmp"))
	{
		stbi_write_bmp(path, m_width, m_height, getAmountOfChannels(), m_pdata.getRaw());
	}
	else if (lowerPath.endsWith(".tga"))
	{
		stbi_write_tga(path, m_width, m_height, getAmountOfChannels(), m_pdata.getRaw());
	}
	else if (lowerPath.endsWith(".jpg"))
	{
		stbi_write_jpg(path, m_width, m_height, getAmountOfChannels(), m_pdata.getRaw(), 90);
	}
	else
	{
		throw bbe::NotImplementedException();
	}
}

void bbe::Image::writeToFile(const bbe::String& path) const
{
	writeToFile(path.getRaw());
}

static void floodFillStep(bbe::Image& image, bbe::List<bbe::Vector2i>& posToCheck, const bbe::Vector2i& pos, const bbe::Colori& from, const bbe::Colori& to)
{
	if (pos.x < 0 || pos.x >= image.getWidth() || pos.y < 0 || pos.y >= image.getHeight()) return;
	if (image.getPixel(pos) != from) return;
	image.setPixel(pos, to);
	posToCheck.add(pos);
}

void bbe::Image::floodFill(const bbe::Vector2i& pos, const bbe::Colori& to, bool fillDiagonal)
{
	const bbe::Colori from = getPixel(pos);
	if (from == to) return;
	bbe::List<bbe::Vector2i> posToCheck;
	floodFillStep(*this, posToCheck, pos, from, to);

	while (posToCheck.getLength() > 0)
	{
		const bbe::Vector2i pos = posToCheck.popBack();
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x + 1, pos.y), from, to);
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x - 1, pos.y), from, to);
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x, pos.y + 1), from, to);
		floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x, pos.y - 1), from, to);
		if (fillDiagonal)
		{
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x + 1, pos.y + 1), from, to);
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x - 1, pos.y + 1), from, to);
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x + 1, pos.y - 1), from, to);
			floodFillStep(*this, posToCheck, bbe::Vector2i(pos.x - 1, pos.y - 1), from, to);
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

bbe::Colori bbe::Image::getPixel(const bbe::Vector2i& pos) const
{
	return getPixel(pos.x, pos.y);
}

bool bbe::Image::isLoadedGpu() const
{
	return m_prendererData != nullptr;
}
