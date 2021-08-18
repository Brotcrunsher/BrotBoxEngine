#include "BBE/Font.h"
#include <filesystem>

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
	this->chars      = chars;

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
	advanceWidths[' '] = static_cast<int>(spaceAdvance * scale / sharpnessFactor);
	leftSideBearings[' '] = static_cast<int>(spaceLeftSideBearing * scale / sharpnessFactor);

	for (size_t i = 0; i < chars.getLength(); i++)
	{
		if (chars[i] == ' ') throw IllegalArgumentException(); // It is not required to have a space as it will just advance the caret position and is always supported.
		if (charImages[chars[i]].isLoaded()) throw IllegalArgumentException(); // A char was passed twice.
		
		stbtt_GetCodepointHMetrics(&fontInfo, chars[i], advanceWidths + chars[i], leftSideBearings + chars[i]);
		advanceWidths[chars[i]] = static_cast<int>(advanceWidths[chars[i]] * scale / sharpnessFactor);
		leftSideBearings[chars[i]] = static_cast<int>(leftSideBearings[chars[i]] * scale / sharpnessFactor);

		int32_t y1 = 0;
		stbtt_GetCodepointBox(&fontInfo, chars[i], nullptr, nullptr, nullptr, &y1);
		verticalOffsets[chars[i]] = static_cast<int>((-y1) * scale / sharpnessFactor);

		int32_t width = 0;
		int32_t height = 0;
		unsigned char* bitmap = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, chars[i], &width, &height, 0, 0);
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
		bbe::Image image = bbe::Image(width, height, (float*)convertedList.getRaw(), bbe::ImageFormat::R8G8B8A8);
		charImages[chars[i]] = std::move(image);
	}

	isInit = true;
}

const bbe::String& bbe::Font::getFontPath() const
{
	if (!isInit) throw NotInitializedException();
	return fontPath;
}

const bbe::String& bbe::Font::getChars() const
{
	if (!isInit) throw NotInitializedException();
	return chars;
}

uint32_t bbe::Font::getFontSize() const
{
	if (!isInit) throw NotInitializedException();
	return fontSize / sharpnessFactor;
}

int32_t bbe::Font::getPixelsFromLineToLine() const
{
	if (!isInit) throw NotInitializedException();
	return pixelsFromLineToLine;
}

const bbe::Image& bbe::Font::getImage(char c) const
{
	if (!isInit) throw NotInitializedException();
	return charImages[c];
}

int32_t bbe::Font::getLeftSideBearing(char c) const
{
	if (!isInit) throw NotInitializedException();
	if (getFixedWidth() > 0)
	{
		return 0;
	}
	else
	{
		return leftSideBearings[c];
	}
}

int32_t bbe::Font::getAdvanceWidth(char c) const
{
	if (!isInit) throw NotInitializedException();
	if (getFixedWidth() > 0)
	{
		return getFixedWidth();
	}
	else
	{
		return advanceWidths[c];
	}
}

int32_t bbe::Font::getVerticalOffset(char c) const
{
	if (!isInit) throw NotInitializedException();
	return verticalOffsets[c];
}

bbe::Vector2 bbe::Font::getDimensions(char c) const
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
	for (size_t i = 0; i < chars.getLength(); i++)
	{
		if (charImages[chars[i]].isLoaded())
		{
			charImages[chars[i]].destroy();
		}
	}
}

uint32_t bbe::Font::getSharpnessFactor() const
{
	return sharpnessFactor;
}

bbe::List<bbe::Vector2> bbe::Font::getRenderPositions(const Vector2& p, const char* text, float rotation) const
{
	bbe::List<bbe::Vector2> retVal;

	const float lineStart = p.x;

	Vector2 currentPosition = p;

	while (*text)
	{
		if (*text == '\n')
		{
			retVal.add(currentPosition);
			currentPosition.x = lineStart;
			currentPosition.y += getPixelsFromLineToLine();
		}
		else if (*text == ' ')
		{
			retVal.add(currentPosition);
			currentPosition.x += getLeftSideBearing(*text) + getAdvanceWidth(*text);
		}
		else
		{
			currentPosition.x += getLeftSideBearing(*text);
			const bbe::Image& charImage = getImage(*text);
			retVal.add((bbe::Vector2(currentPosition.x, currentPosition.y + getVerticalOffset(*text)) + charImage.getDimensions() / 2).rotate(rotation, p) - charImage.getDimensions() / 2);
			currentPosition.x += getAdvanceWidth(*text);
		}

		text++;
	}

	return retVal;
}

bbe::List<bbe::Vector2> bbe::Font::getRenderPositions(const Vector2& p, const bbe::String& text, float rotation) const
{
	return getRenderPositions(p, text.getRaw(), rotation);
}
