#pragma once
#include "../BBE/Vector3.h"
#include "../BBE/Vector4.h"
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

		static Color white();

		bool operator== (const bbe::Color& other) const;
		bool operator!= (const bbe::Color& other) const;

		bbe::Color operator* (float scalar) const;
		bbe::Color operator/ (float scalar) const;

		bbe::Color& operator*= (float scalar);
		bbe::Color& operator/= (float scalar);

		bbe::String toHex() const;

		bbe::Vector4 toVector() const;

		static Color HSVtoRGB(float h, float s, float v);
	};
}
