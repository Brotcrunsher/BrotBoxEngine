#include "BBE/Font.h"
#include <filesystem>
#define STBTT_RASTERIZER_VERSION 1
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include "BBE/SimpleFile.h"

constexpr uint32_t sharpnessFactor = 4;

bbe::Font::Font()
{
	// Do nothing.
}

bbe::Font::Font(const bbe::String& fontPath, unsigned fontSize, const bbe::String& chars)
{
	load(fontPath, fontSize, chars);
}

void bbe::Font::load(const bbe::String& fontPath, unsigned fontSize, const bbe::String& chars)
{
	if (isInit)
	{
		throw AlreadyCreatedException();
	}
	
#ifdef _WIN32
	static const bbe::List<bbe::String> platformDependentFontDirectories = { "C:/Windows/Fonts/" };
#elif defined(unix)
	static const bbe::List<bbe::String> platformDependentFontDirectories = { "/usr/share/fonts/truetype/", "~/.fonts" };
#else
	static const bbe::List<bbe::String> platformDependentFontDirectories = { };
#endif

	if (std::filesystem::exists(fontPath.getRaw()))
	{
		this->fontPath = fontPath;
	}
	else
	{
		bool found = false;
		for (const bbe::String & platformDep : platformDependentFontDirectories)
		{
			const bbe::String currCheckPath = platformDep + fontPath;
			if (std::filesystem::exists(currCheckPath.getRaw()))
			{
				this->fontPath = currCheckPath;
				found = true;
				break;
			}
		}
		if (!found) throw NullPointerException();
	}
	this->fontSize   = fontSize;

	const bbe::List<unsigned char> font = bbe::simpleFile::readBinaryFile(this->fontPath);
	stbtt_fontinfo fontInfo;
	stbtt_InitFont(&fontInfo, font.getRaw(), stbtt_GetFontOffsetForIndex(font.getRaw(), 0));
	const float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(fontSize * sharpnessFactor));

	int32_t ascent = 0;
	int32_t descent = 0;
	int32_t lineGap = 0;
	stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
	pixelsFromLineToLine = static_cast<int>((ascent - descent + lineGap) * scale / sharpnessFactor);

	int32_t spaceAdvance = 0;
	int32_t spaceLeftSideBearing = 0;
	stbtt_GetCodepointHMetrics(&fontInfo, ' ', &spaceAdvance, &spaceLeftSideBearing);
	
	{
		CharData space;
		space.advanceWidth = static_cast<int>(spaceAdvance * scale / sharpnessFactor);
		space.leftSideBearing = static_cast<int>(spaceLeftSideBearing * scale / sharpnessFactor);
		charDatas[' '] = std::move(space);
	}

	{
		CharData empty;
		charDatas['\n'] = std::move(empty);
	}

	const bbe::String string = chars;
	//TODO Wasteful! This could really use an iterator.
	for (size_t i = 0; i < string.getLength(); i++)
	{
		const int32_t codePoint = string.getCodepoint(i);
		if (codePoint == ' ') throw IllegalArgumentException(); // It is not required to have a space as it will just advance the caret position and is always supported.
		
		if (charDatas.find(codePoint) != charDatas.end()) throw IllegalArgumentException(); // A char was passed twice.
		
		CharData cd;

		stbtt_GetCodepointHMetrics(&fontInfo, codePoint, &cd.advanceWidth, &cd.leftSideBearing);
		cd.advanceWidth = static_cast<int>(cd.advanceWidth * scale / sharpnessFactor);
		cd.leftSideBearing = static_cast<int>(cd.leftSideBearing * scale / sharpnessFactor);

		int32_t y1 = 0;
		stbtt_GetCodepointBox(&fontInfo, codePoint, nullptr, nullptr, nullptr, &y1);
		cd.verticalOffset = static_cast<int>((-y1) * scale / sharpnessFactor);

		int width = 0;
		int height = 0;
		int xoff = 0;
		int yoff = 0;
		unsigned char* bitmap = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, codePoint, &width, &height, &xoff, &yoff);
		if (bitmap == nullptr) throw NullPointerException();
		
		bbe::List<byte> convertedList = bbe::List<byte>((size_t)width * (size_t)height * sizeof(float));
		for (size_t i = 0; i < (size_t)width * (size_t)height; i++)
		{
			convertedList.add(bitmap[i]);
			convertedList.add(bitmap[i]);
			convertedList.add(bitmap[i]);
			convertedList.add(bitmap[i]);
		}
		stbtt_FreeBitmap(bitmap, nullptr);

		//TODO: Currently this is a very wasteful approach to rendering text as we create a separate image for every distinct
		//      char. It would be much more efficient to implement some form of texture atlas and use that here instead.
		cd.charImage = bbe::Image(width, height, (float*)convertedList.getRaw(), bbe::ImageFormat::R8G8B8A8);
		cd.charImage.setRepeatMode(bbe::ImageRepeatMode::CLAMP_TO_EDGE);
		charDatas[codePoint] = std::move(cd);
	}

	isInit = true;
}

const bbe::String& bbe::Font::getFontPath() const
{
	if (!isInit) throw NotInitializedException();
	return fontPath;
}

uint32_t bbe::Font::getFontSize() const
{
	if (!isInit) throw NotInitializedException();
	return fontSize;
}

int32_t bbe::Font::getPixelsFromLineToLine() const
{
	if (!isInit) throw NotInitializedException();
	return pixelsFromLineToLine;
}

const bbe::Image& bbe::Font::getImage(int32_t c) const
{
	if (!isInit) throw NotInitializedException();
	return charDatas.at(c).charImage;
}

int32_t bbe::Font::getLeftSideBearing(int32_t c) const
{
	if (!isInit) throw NotInitializedException();
	if (getFixedWidth() > 0)
	{
		return 0;
	}
	else
	{
		return charDatas.at(c).leftSideBearing;
	}
}

int32_t bbe::Font::getAdvanceWidth(int32_t c) const
{
	if (!isInit) throw NotInitializedException();
	if (getFixedWidth() > 0)
	{
		return getFixedWidth();
	}
	else
	{
		return charDatas.at(c).advanceWidth;
	}
}

int32_t bbe::Font::getVerticalOffset(int32_t c) const
{
	if (!isInit) throw NotInitializedException();
	return charDatas.at(c).verticalOffset;
}

bbe::Vector2 bbe::Font::getDimensions(int32_t c) const
{
	return getImage(c).getDimensions() / getSharpnessFactor();
}

void bbe::Font::setFixedWidth(int32_t val)
{
	fixedWidth = val;
}

int32_t bbe::Font::getFixedWidth() const
{
	return fixedWidth;
}

void bbe::Font::destroy()
{
	for (auto& cd : charDatas)
	{
		if (cd.second.charImage.isLoaded())
		{
			cd.second.charImage.destroy();
		}
	}
}

uint32_t bbe::Font::getSharpnessFactor() const
{
	return sharpnessFactor;
}

bbe::List<bbe::Vector2> bbe::Font::getRenderPositions(const Vector2& p, const char* text, float rotation, bool verticalCorrection) const
{
	bbe::List<bbe::Vector2> retVal;

	const float lineStart = p.x;

	Vector2 currentPosition = p;

	const bbe::String string = text;
	//TODO Wasteful! This could really use an iterator.
	for (size_t i = 0; i < string.getLength(); i++)
	{
		const int32_t codePoint = string.getCodepoint(i);
		if (codePoint == '\n')
		{
			retVal.add(currentPosition);
			currentPosition.x = lineStart;
			currentPosition.y += getPixelsFromLineToLine();
		}
		else if (codePoint == ' ')
		{
			retVal.add(currentPosition);
			currentPosition.x += getLeftSideBearing(codePoint) + getAdvanceWidth(codePoint);
		}
		else
		{
			currentPosition.x += getLeftSideBearing(codePoint);
			const bbe::Image& charImage = getImage(codePoint);
			if (verticalCorrection)
			{
				retVal.add((bbe::Vector2(currentPosition.x, currentPosition.y + getVerticalOffset(codePoint)) + charImage.getDimensions() / 2).rotate(rotation, p) - charImage.getDimensions() / 2);
			}
			else
			{
				retVal.add(currentPosition.rotate(rotation, p));
			}
			currentPosition.x += getAdvanceWidth(codePoint);
		}
	}

	return retVal;
}

bbe::List<bbe::Vector2> bbe::Font::getRenderPositions(const Vector2& p, const bbe::String& text, float rotation, bool verticalCorrection) const
{
	return getRenderPositions(p, text.getRaw(), rotation, verticalCorrection);
}
