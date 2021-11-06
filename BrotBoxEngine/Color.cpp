#include "BBE/Color.h"
#include "BBE/Math.h"

bbe::String bbe::Color::toHex() const
{
	const uint32_t r = bbe::Math::clamp01(this->r) * 255;
	const uint32_t g = bbe::Math::clamp01(this->g) * 255;
	const uint32_t b = bbe::Math::clamp01(this->b) * 255;

	const uint32_t rgb = (r << 16) | (g << 8) | (b);

	return bbe::String::toHex(rgb);
}

bbe::Vector3 bbe::Color::HSVtoRGB(float h, float s, float v)
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
		return Vector3(q, v, p);
	case 2:
		return Vector3(p, v, t);
	case 3:
		return Vector3(p, q, v);
	case 4:
		return Vector3(t, p, v);
	case 5:
		return Vector3(v, p, q);
	default:
		return Vector3(v, t, p);
	}
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
