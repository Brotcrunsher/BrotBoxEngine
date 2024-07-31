#pragma once
#include "../BBE/Vector3.h"
#include "../BBE/Vector4.h"
#include "../BBE/String.h"
#include "../BBE/Math.h"

namespace bbe
{
	template<typename T, T maxValue>
	class Color_t
	{
	public:
		static constexpr T MAXIMUM_VALUE = maxValue;

		T r;
		T g;
		T b;
		T a;

		constexpr Color_t()
			: r(0), g(0), b(0), a(maxValue)
		{}
		constexpr Color_t(T rgb)
			: r(rgb), g(rgb), b(rgb), a(maxValue)
		{}
		constexpr Color_t(T r, T g, T b)
			: r(r), g(g), b(b), a(maxValue)
		{}
		constexpr Color_t(T r, T g, T b, T a)
			: r(r), g(g), b(b), a(a)
		{}
		constexpr Color_t(const T* arr)
			: r(arr[0]), g(arr[1]), b(arr[2]), a(arr[3])
		{}

		static Color_t white()
		{
			return Color_t(maxValue, maxValue, maxValue, maxValue);
		}

		bool operator== (const bbe::Color_t<T, maxValue>& other) const
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}
		bool operator!= (const bbe::Color_t<T, maxValue>& other) const
		{
			return !(*this == other);
		}

		bbe::Color_t<T, maxValue> blendTo(const bbe::Color_t<T, maxValue>& other, float t) const
		{
			if (t < 0.f) return *this;
			if (t > 1.f) return other;

			return bbe::Color_t<T, maxValue>(
				bbe::Math::interpolateLinear(r, other.r, t),
				bbe::Math::interpolateLinear(g, other.g, t),
				bbe::Math::interpolateLinear(b, other.b, t),
				bbe::Math::interpolateLinear(a, other.a, t)
			);
		}

		bbe::Color_t<T, maxValue> blendTo(const bbe::Color_t<T, maxValue>& other) const
		{
			const float t = other.a / float(maxValue);
			if (t < 0.f) return *this;
			if (t > 1.f) return other;

			return bbe::Color_t<T, maxValue>(
				bbe::Math::interpolateLinear(r, other.r, t),
				bbe::Math::interpolateLinear(g, other.g, t),
				bbe::Math::interpolateLinear(b, other.b, t),
				maxValue // Is this correct?
			);
		}

		bbe::Color_t<T, maxValue> operator* (float scalar) const
		{
			return bbe::Color_t<T, maxValue>(r * scalar, g * scalar, b * scalar, a * scalar);
		}
		bbe::Color_t<T, maxValue> operator/ (float scalar) const
		{
			return bbe::Color_t<T, maxValue>(r / scalar, g / scalar, b / scalar, a / scalar);
		}

		bbe::Color_t<T, maxValue>& operator*= (float scalar)
		{
			r *= scalar;
			g *= scalar;
			b *= scalar;
			a *= scalar;
			return *this;
		}
		bbe::Color_t<T, maxValue>& operator/= (float scalar)
		{
			r /= scalar;
			g /= scalar;
			b /= scalar;
			a /= scalar;
			return *this;
		}

		bbe::String toHex() const
		{
			if constexpr (std::is_same_v<T, float>)
			{
				const uint32_t r = uint32_t(bbe::Math::clamp01(this->r) * 255.0f);
				const uint32_t g = uint32_t(bbe::Math::clamp01(this->g) * 255.0f);
				const uint32_t b = uint32_t(bbe::Math::clamp01(this->b) * 255.0f);

				const uint32_t rgb = (r << 16) | (g << 8) | (b);

				return bbe::String::toHex(rgb);
			}
			else if constexpr (std::is_same_v<T, bbe::byte>)
			{
				const uint32_t r = uint32_t(this->r);
				const uint32_t g = uint32_t(this->g);
				const uint32_t b = uint32_t(this->b);

				const uint32_t rgb = (r << 16) | (g << 8) | (b);

				return bbe::String::toHex(rgb);
			}
			bbe::Crash(bbe::Error::NotImplemented);
		}

		bbe::Vector4 toVector() const
		{
			return bbe::Vector4(r, g, b, a);
		}

		static Color_t HSVtoRGB(float h, float s, float v)
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
				return Color_t(q, v, p);
			case 2:
				return Color_t(p, v, t);
			case 3:
				return Color_t(p, q, v);
			case 4:
				return Color_t(t, p, v);
			case 5:
				return Color_t(v, p, q);
			default:
				return Color_t(v, t, p);
			}
		}

		Color_t<byte, 255> asByteColor() const
		{
			if constexpr (std::is_same_v<T, float>)
			{
				return Color_t<byte, 255>(
					bbe::Math::clamp01(this->r) * 255,
					bbe::Math::clamp01(this->g) * 255,
					bbe::Math::clamp01(this->b) * 255,
					bbe::Math::clamp01(this->a) * 255
				);
			}
			else if constexpr (std::is_same_v<T, bbe::byte>)
			{
				return *this;
			}
			bbe::Crash(bbe::Error::NotImplemented);
		}
	};

	using Color = Color_t<float, 1.0f>;
	using Colori = Color_t<byte, 255>;
}
