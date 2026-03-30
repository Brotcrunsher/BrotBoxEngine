#include "BBE/Font.h"
#define STBTT_RASTERIZER_VERSION 1
#define STB_TRUETYPE_IMPLEMENTATION
#include "BBE/Logging.h"
#include "BBE/Math.h"
#include "BBE/SimpleFile.h"
#include "EmbeddedFonts.h"
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <stb_truetype.h>

namespace
{
	bool hasTtfOrOtfExtension(const std::filesystem::path &p)
	{
		std::string ext = p.extension().string();
		for (char &c : ext) c = (char)std::tolower((unsigned char)c);
		return ext == ".ttf" || ext == ".otf";
	}
}

const bbe::Font::CharData &bbe::Font::loadCharData(const int32_t codePoint, float scale_) const
{
	if (!isInit) bbe::Crash(bbe::Error::NotInitialized);

	if (charDatas.find({ codePoint, scale_ }) != charDatas.end()) bbe::Crash(bbe::Error::IllegalArgument); // A char was passed twice.

	CharData cd;

	const float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(fontSize * scale_));

	stbtt_GetCodepointHMetrics(&fontInfo, codePoint, &cd.advanceWidth, &cd.leftSideBearing);
	cd.advanceWidth = static_cast<int>(cd.advanceWidth * scale / scale_);
	cd.leftSideBearing = static_cast<int>(cd.leftSideBearing * scale / scale_);

	int32_t y1 = 0;
	stbtt_GetCodepointBox(&fontInfo, codePoint, nullptr, nullptr, nullptr, &y1);
	cd.verticalOffset = static_cast<int>((-y1) * scale / scale_);

	int width = 0;
	int height = 0;
	int xoff = 0;
	int yoff = 0;
	unsigned char *bitmap = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, codePoint, &width, &height, &xoff, &yoff);

	//TODO: Currently this is a very wasteful approach to rendering text as we create a separate image for every distinct
	//      char. It would be much more efficient to implement some form of texture atlas and use that here instead.
	if (bitmap != nullptr && width > 0 && height > 0)
	{
		cd.charImage = bbe::Image(width, height, bitmap, bbe::ImageFormat::R8);
		cd.charImage.setRepeatMode(bbe::ImageRepeatMode::CLAMP_TO_EDGE);
	}
	// If bitmap is nullptr the glyph has no visible area; leave charImage empty (handled like space).
	if (bitmap != nullptr) stbtt_FreeBitmap(bitmap, nullptr);
	charDatas[{ codePoint, scale_ }] = std::move(cd);
	return charDatas[{ codePoint, scale_ }];
}

const bbe::Font::CharData &bbe::Font::getCharData(int32_t c, float scale) const
{
	if (c == ' ' || c == '\n') scale = 1.0f;
	auto keyVal = charDatas.find({ c, scale });
	if (keyVal == charDatas.end())
	{
		return loadCharData(c, scale);
	}

	return keyVal->second;
}

bbe::Font::Font()
{
	// Do nothing.
}

bbe::Font::Font(const bbe::String &fontPath, unsigned fontSize)
{
	load(fontPath, fontSize);
}

void bbe::Font::load(const bbe::String &fontPath, unsigned fontSize)
{
	if (isInit)
	{
		bbe::Crash(bbe::Error::AlreadyCreated);
	}

	if (fontPath == "OpenSansRegular.ttf")
	{
		font = OpenSansRegular;
	}
	else
	{
#ifdef _WIN32
		static const bbe::List<bbe::String> platformDependentFontDirectories = { "C:/Windows/Fonts/" };
#elif defined(unix) || defined(linux)
		static const bbe::List<bbe::String> platformDependentFontDirectories = { "/usr/share/fonts/truetype/", "~/.fonts" };
#else
		static const bbe::List<bbe::String> platformDependentFontDirectories = {};
#endif
		BBELOGLN("Looking for Font " << fontPath);
		if (std::filesystem::exists(fontPath.getRaw()))
		{
			BBELOGLN("Found font via direct fontPath");
			this->fontPath = fontPath;
		}
		else
		{
			bool found = false;
			for (const bbe::String &platformDep : platformDependentFontDirectories)
			{
				const bbe::String currCheckPath = platformDep + fontPath;
				if (std::filesystem::exists(currCheckPath.getRaw()))
				{
					BBELOGLN("Found font via system path: " << currCheckPath);
					this->fontPath = currCheckPath;
					found = true;
					break;
				}
			}
			if (!found)
			{
				BBELOGLN("Could not find font!");
				bbe::Crash(bbe::Error::NullPointer);
			}
		}

		font = bbe::simpleFile::readBinaryFile(this->fontPath);
	}

	this->fontSize = fontSize;

	const int fontOffset = stbtt_GetFontOffsetForIndex(font.getRaw(), 0);
	if (fontOffset < 0 || !stbtt_InitFont(&fontInfo, font.getRaw(), fontOffset))
	{
		bbe::Crash(bbe::Error::NullPointer); // Font format not supported by stb_truetype
	}
	const float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(fontSize));

	int32_t ascent = 0;
	int32_t descent = 0;
	int32_t lineGap = 0;
	stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
	pixelsFromLineToLine = static_cast<int>((ascent - descent + lineGap) * scale);

	int32_t spaceAdvance = 0;
	int32_t spaceLeftSideBearing = 0;
	stbtt_GetCodepointHMetrics(&fontInfo, ' ', &spaceAdvance, &spaceLeftSideBearing);

	{
		CharData space;
		space.advanceWidth = static_cast<int>(spaceAdvance * scale);
		space.leftSideBearing = static_cast<int>(spaceLeftSideBearing * scale);
		charDatas[{ ' ', 1.0f }] = std::move(space);
	}

	{
		CharData empty;
		charDatas[{ '\n', 1.0f }] = std::move(empty);
	}

	isInit = true;
}

const bbe::String &bbe::Font::getFontPath() const
{
	if (!isInit) bbe::Crash(bbe::Error::NotInitialized);
	return fontPath;
}

uint32_t bbe::Font::getFontSize() const
{
	if (!isInit) bbe::Crash(bbe::Error::NotInitialized);
	return fontSize;
}

int32_t bbe::Font::getPixelsFromLineToLine() const
{
	if (!isInit) bbe::Crash(bbe::Error::NotInitialized);
	return pixelsFromLineToLine;
}

const bbe::Image &bbe::Font::getImage(int32_t c, float scale) const
{
	return getCharData(c, scale).charImage;
}

int32_t bbe::Font::getLeftSideBearing(int32_t c) const
{
	if (getFixedWidth() > 0)
	{
		const bbe::Vector2i dim = getDimensions(c);
		return (getFixedWidth() - dim.x) / 2;
	}
	else
	{
		return getCharData(c, 1.0f).leftSideBearing;
	}
}

int32_t bbe::Font::getAdvanceWidth(int32_t c) const
{
	if (!isInit) bbe::Crash(bbe::Error::NotInitialized);
	if (getFixedWidth() > 0)
	{
		return getFixedWidth() - getLeftSideBearing(c);
	}
	else
	{
		return getCharData(c, 1.0f).advanceWidth;
	}
}

int32_t bbe::Font::getVerticalOffset(int32_t c) const
{
	if (!isInit) bbe::Crash(bbe::Error::NotInitialized);
	return getCharData(c, 1.0f).verticalOffset;
}

bbe::Vector2i bbe::Font::getDimensions(int32_t c) const
{
	return getImage(c, 1.f).getDimensions();
}

void bbe::Font::setFixedWidth(int32_t val)
{
	fixedWidth = val;
}

int32_t bbe::Font::getFixedWidth() const
{
	return fixedWidth;
}

bbe::List<bbe::Vector2> bbe::Font::getRenderPositions(const Vector2 &p, const char *text, float rotation, bool verticalCorrection) const
{
	bbe::List<bbe::Vector2> retVal;

	const float lineStart = p.x;

	Vector2 currentPosition = p;

	const bbe::String string = text;
	for (auto it = string.getIterator(); it.valid(); ++it)
	{
		const int32_t codePoint = it.getCodepoint();
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
			const bbe::Image &charImage = getImage(codePoint, 1.0f);
			if (verticalCorrection)
			{
				retVal.add((bbe::Vector2(currentPosition.x, currentPosition.y + getVerticalOffset(codePoint)) + charImage.getDimensions().as<float>() / 2).rotate(rotation, p) - charImage.getDimensions().as<float>() / 2);
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

bbe::List<bbe::Vector2> bbe::Font::getRenderPositions(const Vector2 &p, const bbe::String &text, float rotation, bool verticalCorrection) const
{
	return getRenderPositions(p, text.getRaw(), rotation, verticalCorrection);
}

bbe::Rectangle bbe::Font::getBoundingBox(const bbe::String &text) const
{
	if (text.isEmpty()) return bbe::Rectangle();

	bbe::List<bbe::Vector2> renderPositions = getRenderPositions(bbe::Vector2(0, 0), text);

	bbe::Rectangle retVal = bbe::Rectangle(renderPositions[0], getDimensions(text.getCodepoint(0)).as<float>());
	retVal.width = (float)getAdvanceWidth(text.getCodepoint(0));

	auto it = text.getIterator();
	++it;
	for (size_t i = 1; i < renderPositions.getLength(); ++i, ++it)
	{
		bbe::Rectangle curr = bbe::Rectangle(renderPositions[i], getDimensions(it.getCodepoint()).as<float>());
		curr.width = (float)getAdvanceWidth(it.getCodepoint());
		retVal = retVal.combine(curr);
	}

	return retVal;
}

bbe::Vector2 bbe::Font::getSize(const bbe::String &text) const
{
	const bbe::Rectangle size = getBoundingBox(text);
	return size.getDim() - size.getPos();
}

bool bbe::Font::getRasterOriginAndBounds(const bbe::String &text, const bbe::Vector2i &topLeft, bbe::Vector2 &outOrigin, bbe::Rectangle &outBounds) const
{
	if (text.isEmpty()) return false;
	outBounds = getBoundingBox(text);
	outOrigin = topLeft.as<float>() - outBounds.getPos();
	return true;
}

bbe::FittedFont bbe::Font::getBestFittingFont(const bbe::List<Font> &fonts, const bbe::String &string, bbe::Vector2 maxSize)
{
	if (fonts.isEmpty())
	{
		return {};
	}

	auto splits = string.split(" ");
	size_t spaces = 0;
	for (size_t i = 0; i < splits.getLength(); i++)
	{
		if (splits.getLength() == 0) spaces++;
		else
		{
			for (size_t k = 0; k < spaces; k++)
			{
				splits[i] = " " + splits[i];
			}
			spaces = 1;
		}
	}

	// TODO add the spaces to the start of the splits

	for (size_t i = fonts.getLength() - 1; i != (size_t)-1; i--)
	{
		bbe::String combination = "";
		bool foundMatch = true;
		for (size_t k = 0; k < splits.getLength(); k++)
		{
			bbe::String candidate = combination + (k == 0 ? splits[k].trim() : splits[k]);
			bbe::Vector2 candidateSize = fonts[i].getSize(candidate);
			if (candidateSize.x > maxSize.x)
			{
				if (k == 0)
				{
					// If not even a single word fits, then this font wont fit for sure
					foundMatch = false;
					break;
				}
				candidate = combination + "\n" + splits[k].trim();
				candidateSize = fonts[i].getSize(candidate);
				if (candidateSize.y > maxSize.y || candidateSize.x > maxSize.x)
				{
					foundMatch = false;
					break;
				}
			}

			combination = candidate;
		}

		if (foundMatch)
		{
			return { &fonts[i], combination };
		}
	}

	return {};
}

bool bbe::Font::isFontFileUsable(const bbe::String &path, const bbe::String &requiredGlyphs)
{
	if (path == "OpenSansRegular.ttf") return true;

	std::ifstream file(path.getRaw(), std::ios::binary | std::ios::ate);
	if (!file) return false;
	const auto size = file.tellg();
	if (size <= 0) return false;
	file.seekg(0);

	std::vector<unsigned char> data(static_cast<size_t>(size));
	if (!file.read(reinterpret_cast<char *>(data.data()), size)) return false;

	stbtt_fontinfo info = {};
	const int offset = stbtt_GetFontOffsetForIndex(data.data(), 0);
	if (offset < 0) return false;
	if (!stbtt_InitFont(&info, data.data(), offset)) return false;

	const char *rg = requiredGlyphs.getRaw();
	for (size_t i = 0; rg[i] != '\0'; i++)
	{
		const unsigned char c = (unsigned char)rg[i];
		if (stbtt_FindGlyphIndex(&info, (int)c) == 0) return false;
	}
	return true;
}

bbe::List<bbe::FontFileEntry> bbe::Font::findUsableSystemFonts(const bbe::String &requiredGlyphs)
{
	bbe::List<bbe::FontFileEntry> result;
	result.add({ "OpenSansRegular", "OpenSansRegular.ttf" });

	bbe::List<bbe::String> fontDirs;
#ifdef _WIN32
	fontDirs.add("C:/Windows/Fonts/");
#else
	fontDirs.add("/usr/share/fonts/");
	fontDirs.add("/usr/local/share/fonts/");
	{
		const char *home = std::getenv("HOME");
		if (home)
		{
			fontDirs.add(bbe::String(home) + "/.fonts/");
			fontDirs.add(bbe::String(home) + "/.local/share/fonts/");
		}
	}
#endif

	for (size_t d = 0; d < fontDirs.getLength(); d++)
	{
		const std::string dirStr(fontDirs[d].getRaw());
		if (!std::filesystem::exists(dirStr)) continue;
		try
		{
			for (const auto &entry : std::filesystem::recursive_directory_iterator(dirStr))
			{
				if (!entry.is_regular_file()) continue;
				if (!hasTtfOrOtfExtension(entry.path())) continue;

				const bbe::String fullPath = entry.path().string().c_str();
				if (!bbe::Font::isFontFileUsable(fullPath, requiredGlyphs)) continue;

				bbe::FontFileEntry fe;
				fe.displayName = entry.path().stem().string().c_str();
				fe.path = fullPath;
				result.add(fe);
			}
		}
		catch (...)
		{
		}
	}

	if (result.getLength() > 2)
	{
		bbe::List<bbe::FontFileEntry> tail;
		for (size_t i = 1; i < result.getLength(); i++) tail.add(result[i]);
		tail.sort([](const bbe::FontFileEntry &a, const bbe::FontFileEntry &b) { return a.displayName < b.displayName; });
		while (result.getLength() > 1) result.popBack();
		for (size_t i = 0; i < tail.getLength(); i++) result.add(tail[i]);
	}

	return result;
}
