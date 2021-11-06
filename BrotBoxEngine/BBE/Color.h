#pragma once
#include "../BBE/Vector3.h"
#include "../BBE/String.h"

namespace bbe
{
	class Color
	{
	public:
		float r;
		float g;
		float b;
		float a;

		constexpr Color()
			: r(0), g(0), b(0), a(1)
		{}
		constexpr Color(float r, float g, float b)
			: r(r), g(g), b(b), a(1)
		{}
		constexpr Color(float r, float g, float b, float a)
			: r(r), g(g), b(b), a(a)
		{}

		bool operator== (const bbe::Color& other) const;
		bool operator!= (const bbe::Color& other) const;

		bbe::Color operator* (float scalar) const;
		bbe::Color operator/ (float scalar) const;

		bbe::Color& operator*= (float scalar);
		bbe::Color& operator/= (float scalar);

		bbe::String toHex() const;

		static Vector3 HSVtoRGB(float h, float s, float v);
	};
}
