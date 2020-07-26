#pragma once


namespace bbe
{
	class Vector2
	{
	public:
		float x;
		float y;

		Vector2();
		Vector2(float x, float y);
		explicit Vector2(float xy);
		static Vector2 createVector2OnUnitCircle(float radians);

		Vector2& operator+=(const Vector2& other);
		Vector2& operator-=(const Vector2& other);
		Vector2& operator*=(const Vector2& other);
		Vector2& operator/=(const Vector2& other);

		Vector2& operator*=(float scaler);
		Vector2& operator/=(float scaler);

		Vector2 operator*(float scalar) const;
		Vector2 operator/(float scalar) const;
		float operator*(const Vector2 &other) const;
		Vector2 operator+(const Vector2 &other) const;
		Vector2 operator-(const Vector2 &other) const;
		Vector2 operator-() const;
		float& operator[](int index);
		const float& operator[](int index) const;

		bool operator==(const Vector2 &other) const;
		bool operator!=(const Vector2 &other) const;
		bool operator> (const Vector2 &other) const;
		bool operator>=(const Vector2 &other) const;
		bool operator< (const Vector2 &other) const;
		bool operator<=(const Vector2 &other) const;

		bool equals(const Vector2 &other, float epsilon = 0.001f) const;
		bool isSameLength(const Vector2 &other, float epsilon = 0.001f) const;
		bool isSameDirection(const Vector2 &other) const;
		bool isOppositeDirection(const Vector2 &other) const;
		bool isLeft(const Vector2 &other) const;
		bool isRight(const Vector2 &other) const;
		bool isContainingNaN() const;
		bool isContainingInfinity() const;
		bool isUnit(float epsilon = 0.001f) const;
		bool isCloseTo(const Vector2 &other, float maxDistance) const;
		bool isZero() const;

		Vector2 rotate(float radians) const;
		Vector2 rotate(float radians, const Vector2 &center) const;
		Vector2 rotate90Clockwise() const;
		Vector2 rotate90CounterClockwise() const;
		Vector2 withLenght(float length) const;
		Vector2 normalize(const bbe::Vector2& zeroBehavior = bbe::Vector2(1, 0)) const;
		Vector2 abs() const;
		Vector2 clampComponents(float min, float max) const;
		Vector2 project(const Vector2 &other) const;
		Vector2 reflect(const Vector2 &normal) const;

		float getLength() const;
		float getLengthSq() const;
		float getDistanceTo(const Vector2& other) const;
		float getDistanceTo(float x, float y) const;
		float getMax() const;
		float getMin() const;
		float getMaxAbs() const;
		float getMinAbs() const;
		float getMaxAbsKeepSign() const;
		float getMinAbsKeepSign() const;
		float getAngle() const;
		float getAngle(const Vector2 &other) const;

		Vector2 xx() const;
		Vector2 xy() const;
		Vector2 yx() const;
		Vector2 yy() const;
	};
}
