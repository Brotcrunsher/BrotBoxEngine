#pragma once

#include "BBE/Math.h"
#include "BBE/Exceptions.h"

namespace bbe
{
	template<typename T>
	class Vector2_t
	{
	public:
		float x;
		float y;

		Vector2_t()
			: x(0), y(0)
		{
		}

		Vector2_t(float x, float y)
			: x(x), y(y)
		{
		}

		explicit Vector2_t(float xy)
			: x(xy), y(xy)
		{
		}

		static Vector2_t createVector2OnUnitCircle(float radians)
		{
			float x = Math::cos(radians);
			float y = Math::sin(radians);
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

		Vector2_t& operator*=(float scalar)
		{
			//UNTESTED
			this->x *= scalar;
			this->y *= scalar;
			return *this;
		}

		Vector2_t& operator/=(float scalar)
		{
			//UNTESTED
			this->x /= scalar;
			this->y /= scalar;
			return *this;
		}

		Vector2_t operator*(float scalar) const
		{
			return Vector2_t<T>(x * scalar, y * scalar);
		}

		Vector2_t operator/(float scalar) const
		{
			return Vector2_t<T>(x / scalar, y / scalar);
		}

		float operator*(const Vector2_t<T> &other) const
		{
			return x * other.x + y * other.y;
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

		float& operator[](int index)
		{
			switch (index)
			{
			case 0:
				return x;
			case 1:
				return y;
			default:
				throw IllegalIndexException();
			}
		}

		const float& operator[](int index) const
		{
			switch (index)
			{
			case 0:
				return x;
			case 1:
				return y;
			default:
				throw IllegalIndexException();
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
			return getLengthSq() > other.getLengthSq();
		}
		bool operator>=(const Vector2_t<T> &other) const
		{
			return getLengthSq() >= other.getLengthSq();
		}
		bool operator< (const Vector2_t<T> &other) const
		{
			return getLengthSq() < other.getLengthSq();
		}
		bool operator<=(const Vector2_t<T> &other) const
		{
			return getLengthSq() <= other.getLengthSq();
		}

		bool equals(const Vector2_t<T> &other, float epsilon = 0.001f) const
		{
			return Math::floatEquals(x, other.x, epsilon)
				&& Math::floatEquals(y, other.y, epsilon);
		}
		bool isSameLength(const Vector2_t<T> &other, float epsilon = 0.001f) const
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
		bool isUnit(float epsilon = 0.001f) const
		{
			float length = getLength();
			return length > 1 - epsilon && length < 1 + epsilon;
		}
		bool isCloseTo(const Vector2_t<T> &other, float maxDistance) const
		{
			return (operator-(other)).getLength() <= maxDistance;
		}
		bool isZero() const
		{
			return x == 0 && y == 0;
		}

		Vector2_t rotate(float radians) const
		{
			float sin = Math::sin(radians);
			float cos = Math::cos(radians);

			return Vector2_t<T>(
				x * cos - y * sin,
				x * sin + y * cos
			);
		}
		Vector2_t rotate(float radians, const Vector2_t<T> &center) const
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
		Vector2_t withLenght(float length) const
		{
			return normalize() * length;
		}
		Vector2_t normalize(const bbe::Vector2_t<T>& zeroBehavior = bbe::Vector2_t<T>(1, 0)) const
		{
			float length = getLength();
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
		Vector2_t clampComponents(float min, float max) const
		{
			return Vector2_t<T>(Math::clamp(x, min, max), Math::clamp(y, min, max));
		}
		Vector2_t project(const Vector2_t<T> &other) const
		{
			float scalar = operator*(other);
			scalar /= other.getLengthSq();
			return other * scalar;
		}
		Vector2_t reflect(const Vector2_t<T>&normal) const
		{
			Vector2_t<T> normalized = normal.normalize();
			return operator-(normalized * 2 * (operator*(normalized)));
		}

		float getLength() const
		{
			return Math::sqrt(getLengthSq());
		}
		float getLengthSq() const
		{
			return x * x + y * y;
		}
		float getDistanceTo(const Vector2_t<T>& other) const
		{
			return (operator-(other)).getLength();
		}
		float getDistanceTo(float x, float y) const
		{
			return getDistanceTo(Vector2_t<T>(x, y));
		}
		float getMax() const
		{
			return Math::max(x, y);
		}
		float getMin() const
		{
			return Math::min(x, y);
		}
		float getMaxAbs() const
		{
			return Math::maxAbs(x, y);
		}
		float getMinAbs() const
		{
			return Math::minAbs(x, y);
		}
		float getMaxAbsKeepSign() const
		{
			return Math::maxAbsKeepSign(x, y);
		}
		float getMinAbsKeepSign() const
		{
			return Math::minAbsKeepSign(x, y);
		}
		float getAngle() const
		{
			float angle = Math::acos(x / getLength());
			if (y >= 0)
			{
				return angle;
			}
			else
			{
				return Math::TAU - angle;
			}
		}
		float getAngle(const Vector2_t<T> &other) const
		{
			float angle = Math::acos(operator*(other) / getLength() / other.getLength());
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
	};

	using Vector2 = Vector2_t<float>;
}
