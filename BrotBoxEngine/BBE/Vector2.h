#pragma once

#include "BBE/Math.h"
#include "BBE/Error.h"

namespace bbe
{
	template<typename T>
	class Vector2_t
	{
	public:
		using SubType = T;

		T x;
		T y;

		constexpr Vector2_t()
			: x(0), y(0)
		{
		}

		constexpr Vector2_t(T x, T y)
			: x(x), y(y)
		{
		}

		explicit constexpr Vector2_t(T xy)
			: x(xy), y(xy)
		{
		}

		static Vector2_t createVector2OnUnitCircle(T radians)
		{
			T x = static_cast<T>(Math::cos(radians));
			T y = static_cast<T>(Math::sin(radians));
			return Vector2(x, y);
		}

		Vector2_t& operator+=(const Vector2_t<T>& other)
		{
			this->x += other.x;
			this->y += other.y;
			return *this;
		}

		Vector2_t& operator-=(const Vector2_t<T>& other)
		{
			this->x -= other.x;
			this->y -= other.y;
			return *this;
		}

		Vector2_t& operator*=(const Vector2_t<T>& other)
		{
			this->x *= other.x;
			this->y *= other.y;
			return *this;
		}

		Vector2_t& operator/=(const Vector2_t<T>& other)
		{
			this->x /= other.x;
			this->y /= other.y;
			return *this;
		}

		Vector2_t& operator*=(T scalar)
		{
			//UNTESTED
			this->x *= scalar;
			this->y *= scalar;
			return *this;
		}

		Vector2_t& operator/=(T scalar)
		{
			//UNTESTED
			this->x /= scalar;
			this->y /= scalar;
			return *this;
		}

		Vector2_t operator*(T scalar) const
		{
			return Vector2_t<T>(x * scalar, y * scalar);
		}

		Vector2_t operator/(T scalar) const
		{
			return Vector2_t<T>(x / scalar, y / scalar);
		}

		T operator*(const Vector2_t<T> &other) const
		{
			return x * other.x + y * other.y;
		}

		Vector2_t operator/(const Vector2_t<T>& other) const
		{
			return Vector2_t<T>(x / other.x, y / other.y);
		}

		Vector2_t operator+(const Vector2_t<T> &other) const
		{
			return Vector2_t<T>(x + other.x, y + other.y);
		}

		Vector2_t operator-(const Vector2_t<T> &other) const
		{
			return Vector2_t<T>(x - other.x, y - other.y);
		}

		Vector2_t operator-() const
		{
			return Vector2_t<T>(-x, -y);
		}

		T& operator[](int index)
		{
			switch (index)
			{
			case 0:
				return x;
			case 1:
				return y;
			default:
				bbe::Crash(bbe::Error::IllegalIndex);
			}
		}

		const T& operator[](int index) const
		{
			switch (index)
			{
			case 0:
				return x;
			case 1:
				return y;
			default:
				bbe::Crash(bbe::Error::IllegalIndex);
			}
		}

		bool operator==(const Vector2_t<T> &other) const
		{
			return x == other.x && y == other.y;
		}
		bool operator!=(const Vector2_t<T> &other) const
		{
			return !(operator==(other));
		}
		bool operator> (const Vector2_t<T> &other) const
		{
			if (x > other.x) return true;
			if (x < other.x) return false;
			return y > other.y;
		}
		bool operator>=(const Vector2_t<T> &other) const
		{
			if (x > other.x) return true;
			if (x < other.x) return false;
			return y >= other.y;
		}
		bool operator< (const Vector2_t<T> &other) const
		{
			if (x < other.x) return true;
			if (x > other.x) return false;
			return y < other.y;
		}
		bool operator<=(const Vector2_t<T> &other) const
		{
			if (x < other.x) return true;
			if (x > other.x) return false;
			return y <= other.y;
		}

		bool equals(const Vector2_t<T> &other, T epsilon = 0.001f) const
		{
			return Math::floatEquals(x, other.x, epsilon)
				&& Math::floatEquals(y, other.y, epsilon);
		}
		bool isSameLength(const Vector2_t<T> &other, T epsilon = 0.001f) const
		{
			return Math::floatEquals(getLengthSq(), other.getLengthSq(), epsilon * epsilon);
		}
		bool isSameDirection(const Vector2_t<T> &other) const
		{
			return operator*(other) > 0;
		}
		bool isOppositeDirection(const Vector2_t<T> &other) const
		{
			return operator*(other) < 0;
		}
		bool isLeft(const Vector2_t<T> &other) const
		{
			return (rotate90CounterClockwise() * other) > 0;
		}
		bool isRight(const Vector2_t<T> &other) const
		{
			return (rotate90Clockwise() * other) > 0;
		}
		bool isContainingNaN() const
		{
			return Math::isNaN(x) || Math::isNaN(y);
		}
		bool isContainingInfinity() const
		{
			return Math::isInfinity(x) || Math::isInfinity(y);
		}
		bool isUnit(T epsilon = 0.001) const
		{
			T length = getLength();
			return length > 1 - epsilon && length < 1 + epsilon;
		}
		bool isCloseTo(const Vector2_t<T> &other, T maxDistance) const
		{
			return (operator-(other)).getLength() <= maxDistance;
		}
		bool isZero() const
		{
			return x == 0 && y == 0;
		}

		T cross(const Vector2_t& other) const
		{
			return this->x * other.y - this->y * other.x;
		}

		Vector2_t rotate(T radians) const
		{
			T sin = static_cast<T>(Math::sin(radians));
			T cos = static_cast<T>(Math::cos(radians));

			return Vector2_t<T>(
				x * cos - y * sin,
				x * sin + y * cos
			);
		}
		Vector2_t rotate(T radians, const Vector2_t<T> &center) const
		{
			Vector2_t<T> rotVec = operator-(center);
			rotVec = rotVec.rotate(radians);
			return rotVec + center;
		}
		Vector2_t rotate90Clockwise() const
		{
			return Vector2_t<T>(-y, x);
		}
		Vector2_t rotate90CounterClockwise() const
		{
			return Vector2_t<T>(y, -x);
		}
		Vector2_t withLenght(T length) const
		{
			return normalize() * length;
		}
		Vector2_t normalize(const bbe::Vector2_t<T>& zeroBehavior = bbe::Vector2_t<T>(1, 0)) const
		{
			T length = getLength();
			if (length == 0)
			{
				return zeroBehavior;
			}
			return Vector2_t<T>(x / length, y / length);
		}
		Vector2_t abs() const
		{
			return Vector2_t<T>(Math::abs(x), Math::abs(y));
		}
		Vector2_t clampComponents(T min, T max) const
		{
			return Vector2_t<T>(Math::clamp(x, min, max), Math::clamp(y, min, max));
		}
		Vector2_t project(const Vector2_t<T> &other) const
		{
			T scalar = operator*(other);
			scalar /= other.getLengthSq();
			return other * scalar;
		}
		Vector2_t reflect(const Vector2_t<T>&normal) const
		{
			Vector2_t<T> normalized = normal.normalize();
			return operator-(normalized * 2 * (operator*(normalized)));
		}

		Vector2_t maxVector(const Vector2_t<T>& other) const
		{
			return Vector2_t<T>(
				bbe::Math::max(x, other.x),
				bbe::Math::max(y, other.y)
				);
		}

		T getLength() const
		{
			return Math::sqrt(getLengthSq());
		}
		T getLengthSq() const
		{
			return x * x + y * y;
		}
		T getLengthSqSigned() const
		{
			return bbe::Math::abs(x) * x + bbe::Math::abs(y) * y;
		}
		T getDistanceTo(const Vector2_t<T>& other) const
		{
			return (operator-(other)).getLength();
		}
		T getDistanceTo(T x, T y) const
		{
			return getDistanceTo(Vector2_t<T>(x, y));
		}
		T getMax() const
		{
			return Math::max(x, y);
		}
		T getMin() const
		{
			return Math::min(x, y);
		}
		T getMaxAbs() const
		{
			return Math::maxAbs(x, y);
		}
		T getMinAbs() const
		{
			return Math::minAbs(x, y);
		}
		T getMaxAbsKeepSign() const
		{
			return Math::maxAbsKeepSign(x, y);
		}
		T getMinAbsKeepSign() const
		{
			return Math::minAbsKeepSign(x, y);
		}
		T getAngle() const
		{
			T angle = Math::acos(x / getLength());
			if (y >= 0)
			{
				return angle;
			}
			else
			{
				return Math::TAU - angle;
			}
		}
		T getAngle(const Vector2_t<T> &other) const
		{
			T angle = Math::acos(operator*(other) / getLength() / other.getLength());
			if (isLeft(other))
			{
				return angle;
			}
			else
			{
				return Math::TAU - angle;
			}
		}

		Vector2_t xx() const
		{
			return Vector2_t<T>(x, x);
		}
		Vector2_t xy() const
		{
			return Vector2_t<T>(x, y);
		}
		Vector2_t yx() const
		{
			return Vector2_t<T>(y, x);
		}
		Vector2_t yy() const
		{
			return Vector2_t<T>(y, y);
		}

		template<typename U>
		Vector2_t<U> as() const
		{
			return Vector2_t<U>((U)x, (U)y);
		}
	};

	using Vector2  = Vector2_t<float>;
	using Vector2d = Vector2_t<double>;
	using Vector2i = Vector2_t<int32_t>;

	template<>
	uint32_t hash(const Vector2i& t);
	template<>
	uint32_t hash(const Vector2& t);

	class LineIterator
	{
	private:
		bbe::Vector2i a;
		bbe::Vector2i b;
		bbe::Vector2i diff;
		bbe::Vector2i step;
		int32_t error = 0;
		bool moreAvailable = true;

		void init(const bbe::Vector2i& a, const bbe::Vector2i& b);

	public:
		LineIterator();
		LineIterator(const bbe::Vector2i& a, const bbe::Vector2i& b);

		bool hasNext() const;
		bbe::Vector2i next();
	};
}
