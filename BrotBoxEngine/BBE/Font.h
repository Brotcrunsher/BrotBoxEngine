#pragma once

// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include <map>

#include "../BBE/String.h"
#include "../BBE/Image.h"
#include "../BBE/Rectangle.h"
#include "../BBE/Vector2.h"
#include "../BBE/Rectangle.h"

namespace bbe
{
	class Font;
	struct FittedFont
	{
		const Font* font = nullptr;
		bbe::String string = "";
	};

	class Font
	{
	private:
		static constexpr unsigned    DEFAULT_FONT_SIZE  = 20;
		// constexpr const is necessary for some compilers to avoid false warnings
		static constexpr const char* DEFAULT_CHARSET    = u8"1234567890!\"/()=\\abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+#-.,_:;<>|^{}[]*~&?%$äöüÄÖÜß";

		bool isInit              = false;
		bbe::String fontPath     = "";
		uint32_t fontSize        = 0;
		int32_t pixelsFromLineToLine = 0;

		int32_t fixedWidth = 0;
		
		struct CharData
		{
			bbe::Image charImage;
			int32_t advanceWidth = 0;
			int32_t leftSideBearing = 0;
			int32_t verticalOffset = 0;
		};

		std::map<int32_t, CharData> charDatas;

	public:
		Font();
		Font(const bbe::String& fontPath, 
		     unsigned fontSize        = DEFAULT_FONT_SIZE,
		     const bbe::String& chars = DEFAULT_CHARSET);

		void load(const bbe::String& fontPath,
		          unsigned fontSize        = DEFAULT_FONT_SIZE,
		          const bbe::String& chars = DEFAULT_CHARSET);

		const    bbe::String& getFontPath() const;
		uint32_t getFontSize()              const;
		int32_t  getPixelsFromLineToLine()  const;

		const bbe::Image& getImage(int32_t c) const;
		int32_t getLeftSideBearing(int32_t c) const;
		int32_t getAdvanceWidth(int32_t c) const;
		int32_t getVerticalOffset(int32_t c) const;

		bbe::Vector2 getDimensions(int32_t c) const;

		void setFixedWidth(int32_t val);
		int32_t getFixedWidth() const;

		void destroy();

		uint32_t getSharpnessFactor() const;

		bbe::List<Vector2> getRenderPositions(const Vector2& p, const char* text, float rotation = 0, bool verticalCorrection = true) const;
		bbe::List<Vector2> getRenderPositions(const Vector2& p, const bbe::String &text, float rotation = 0, bool verticalCorrection = true) const;

		bbe::Rectangle getBoundingBox(const bbe::String& text) const;
		bbe::Vector2 getSize(const bbe::String& text) const;

		static FittedFont getBestFittingFont(const bbe::List<Font>& fonts, const bbe::String& string, bbe::Vector2 maxSize);
	};
}
#endif
