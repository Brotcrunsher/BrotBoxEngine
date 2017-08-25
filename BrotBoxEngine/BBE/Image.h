#pragma once

#include "../BBE/ColorByte.h"
#include "../BBE/Color.h"
#include "../BBE/String.h"

namespace bbe
{
	class Image
	{
	private:
		ColorByte *m_pdata  = nullptr;
		int    m_width  = 0;
		int    m_height = 0;

	public:
		Image();
		Image(const char* path);
		Image(int width, int height);
		Image(int width, int height, const Color &c);

		~Image();

		void load(const char* path);
		void load(int width, int height);
		void load(int width, int height, const Color &c);

		void destroy();

		int getWidth() const;
		int getHeight() const;
		Color getPixel(int x, int y) const;
	};
}