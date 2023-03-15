#include "BBE/Color.h"
#include "BBE/Math.h"

bbe::String bbe::Color::toHex() const
{
	const uint32_t r = uint32_t(bbe::Math::clamp01(this->r) * 255.0f);
	const uint32_t g = uint32_t(bbe::Math::clamp01(this->g) * 255.0f);
	const uint32_t b = uint32_t(bbe::Math::clamp01(this->b) * 255.0f);

	const uint32_t rgb = (r << 16) | (g << 8) | (b);

	return bbe::String::toHex(rgb);
}

bbe::Vector4 bbe::Color::toVector() const
{
	return bbe::Vector4(r, g, b, a);
}

bbe::Color bbe::Color::HSVtoRGB(float h, float s, float v)
{
	//UNTESTED
	h = bbe::Math::mod(h, 360.0f);
	int hi = (int)(h / 60);
	float f = (h / 60 - hi);

	float p = v * (1 - s);
	float q = v * (1 - s * f);
	float t = v * (1 - s * (1 - f));

	switch (hi)
	{
	case 1:
		return Color(q, v, p);
	case 2:
		return Color(p, v, t);
	case 3:
		return Color(p, q, v);
	case 4:
		return Color(t, p, v);
	case 5:
		return Color(v, p, q);
	default:
		return Color(v, t, p);
	}
}

bbe::Color bbe::Color::white()
{
	return Color(1, 1, 1, 1);
}

bool bbe::Color::operator==(const Color& other) const
{
	return r == other.r && g == other.g && b == other.b && a == other.a;
}
bool bbe::Color::operator!=(const Color& other) const
{
	return !(*this == other);
}

bbe::Color bbe::Color::operator*(float scalar) const
{
	return bbe::Color(r * scalar, g * scalar, b * scalar, a * scalar);
}

bbe::Color bbe::Color::operator/(float scalar) const
{
	return bbe::Color(r / scalar, g / scalar, b / scalar, a / scalar);
}

bbe::Color& bbe::Color::operator*=(float scalar)
{
	r *= scalar;
	g *= scalar;
	b *= scalar;
	a *= scalar;
	return *this;
}

bbe::Color& bbe::Color::operator/=(float scalar)
{
	r /= scalar;
	g /= scalar;
	b /= scalar;
	a /= scalar;
	return *this;
}
