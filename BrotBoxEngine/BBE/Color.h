#pragma once
#include "../BBE/Vector3.h"

namespace bbe
{
	class Color
	{
	public:
		float r;
		float g;
		float b;
		float a;

		Color();
		Color(float r, float g, float b);
		Color(float r, float g, float b, float a);

		bool operator== (const bbe::Color& other) const;
		bool operator!= (const bbe::Color& other) const;

		bbe::Color operator* (float scalar) const;
		bbe::Color operator/ (float scalar) const;

		static Vector3 HSVtoRGB(float h, float s, float v);
	};
}
