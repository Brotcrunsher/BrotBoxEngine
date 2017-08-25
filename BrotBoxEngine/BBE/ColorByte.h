#pragma once

#include "../BBE/DataType.h"


namespace bbe
{
	class ColorByte
	{
	public:
		byte r;
		byte g;
		byte b;
		byte a;

		ColorByte();
		ColorByte(byte r, byte g, byte b);
		ColorByte(byte r, byte g, byte b, byte a);
	};
}
