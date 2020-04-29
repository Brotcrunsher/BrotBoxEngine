#include "BBE/Font.h"
#include <filesystem>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include "BBE/SimpleFile.h"

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
	const float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

	int ascent = 0;
	int descent = 0;
	int lineGap = 0;
	stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
	pixelsFromLineToLine = (ascent - descent + lineGap) * scale;

	int spaceAdvance = 0;
	int spaceLeftSideBearing = 0;
	stbtt_GetCodepointHMetrics(&fontInfo, ' ', &spaceAdvance, &spaceLeftSideBearing);
	advanceWidths[' '] = spaceAdvance * scale;
	leftSideBearings[' '] = spaceLeftSideBearing * scale;

	for (size_t i = 0; i < chars.getLength(); i++)
	{
		if (chars[i] == ' ') throw IllegalArgumentException(); // It is not required to have a space as it will just advance the caret position and is always supported.
		if (charImages[chars[i]].isLoaded()) throw IllegalArgumentException(); // A char was passed twice.
		
		stbtt_GetCodepointHMetrics(&fontInfo, chars[i], advanceWidths + chars[i], leftSideBearings + chars[i]);
		advanceWidths[chars[i]] *= scale;
		leftSideBearings[chars[i]] *= scale;

		int y1 = 0;
		stbtt_GetCodepointBox(&fontInfo, chars[i], nullptr, nullptr, nullptr, &y1);
		verticalOffsets[chars[i]] = (-y1) * scale;

		int width = 0;
		int height = 0;
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

unsigned bbe::Font::getFontSize() const
{
	if (!isInit) throw NotInitializedException();
	return fontSize;
}

int bbe::Font::getPixelsFromLineToLine() const
{
	if (!isInit) throw NotInitializedException();
	return pixelsFromLineToLine;
}

const bbe::Image& bbe::Font::getImage(char c) const
{
	if (!isInit) throw NotInitializedException();
	return charImages[c];
}

int bbe::Font::getLeftSideBearing(char c) const
{
	if (!isInit) throw NotInitializedException();
	return leftSideBearings[c];
}

int bbe::Font::getAdvanceWidth(char c) const
{
	if (!isInit) throw NotInitializedException();
	return advanceWidths[c];
}

int bbe::Font::getVerticalOffset(char c) const
{
	if (!isInit) throw NotInitializedException();
	return verticalOffsets[c];
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
