#include "stdafx.h"
#include "BBE/ColorByte.h"

bbe::ColorByte::ColorByte()
	:r((byte)255), g((byte)255), b((byte)255), a((byte)255)
{
}

bbe::ColorByte::ColorByte(byte r, byte g, byte b)
	:r(r), g(g), b(b), a((byte)255)
{
}

bbe::ColorByte::ColorByte(byte r, byte g, byte b, byte a)
	:r(r), g(g), b(b), a(a)
{
}
