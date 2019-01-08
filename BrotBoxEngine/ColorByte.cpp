#include "stdafx.h"
#include "BBE/ColorByte.h"

bbe::ColorByte::ColorByte()
	:r(static_cast<byte>(255)), g(static_cast<byte>(255)), b(static_cast<byte>(255)), a(static_cast<byte>(255))
{
	//UNTESTED
}

bbe::ColorByte::ColorByte(byte r, byte g, byte b)
	:r(r), g(g), b(b), a(static_cast<byte>(255))
{
	//UNTESTED
}

bbe::ColorByte::ColorByte(byte r, byte g, byte b, byte a)
	:r(r), g(g), b(b), a(a)
{
	//UNTESTED
}
