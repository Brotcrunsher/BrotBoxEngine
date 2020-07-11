#pragma once

#include "../BBE/String.h"
#include "../BBE/Image.h"

namespace bbe
{
	class Font
	{
	private:
		static constexpr unsigned    DEFAULT_FONT_SIZE  = 20;
		// constexpr const is necessary for some compilers to avoid false warnings
		static constexpr const char* DEFAULT_CHARSET    = u8"1234567890!\"/()=\\abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+#-.,_:;<>|^";

		bool isInit              = false;
		bbe::String fontPath     = "";
		bbe::String chars        = "";
		uint32_t fontSize        = 0;
		int32_t pixelsFromLineToLine = 0;

		int32_t fixedWidth = 0;
		
		bbe::Image charImages     [256];
		int32_t advanceWidths         [256] = {};
		int32_t leftSideBearings      [256] = {};
		int32_t verticalOffsets       [256] = {};

	public:
		Font();
		Font(const bbe::String& fontPath, 
		     unsigned fontSize        = DEFAULT_FONT_SIZE,
		     const bbe::String& chars = DEFAULT_CHARSET);

		void load(const bbe::String& fontPath,
		          unsigned fontSize        = DEFAULT_FONT_SIZE,
		          const bbe::String& chars = DEFAULT_CHARSET);

		const    bbe::String& getFontPath() const;
		const    bbe::String& getChars()    const;
		uint32_t getFontSize()              const;
		int32_t  getPixelsFromLineToLine()  const;

		const bbe::Image& getImage(char c) const;
		int32_t getLeftSideBearing(char c) const;
		int32_t getAdvanceWidth(char c) const;
		int32_t getVerticalOffset(char c) const;

		void setFixedWidth(int32_t val);
		int32_t getFixedWidth() const;

		void destroy();
	};
}