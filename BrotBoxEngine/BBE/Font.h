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
		unsigned fontSize        = 0;
		int pixelsFromLineToLine = 0;
		
		bbe::Image charImages     [256];
		int advanceWidths         [256] = {};
		int leftSideBearings      [256] = {};
		int verticalOffsets       [256] = {};

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
		unsigned getFontSize()              const;
		int      getPixelsFromLineToLine()  const;

		const bbe::Image& getImage(char c) const;
		int getLeftSideBearing(char c) const;
		int getAdvanceWidth(char c) const;
		int getVerticalOffset(char c) const;
	};
}