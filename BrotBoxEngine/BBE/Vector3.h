#pragma once


namespace bbe
{
	class Vector2;

	class Vector3
	{
	public:
		float x;
		float y;
		float z;

		Vector3();
		explicit Vector3(float xyz);
		Vector3(float x, float y, float z);
		Vector3(float x, const Vector2 &yz);
		Vector3(const Vector2 &xy, float z);

		Vector3& operator+=(const Vector3& other);
		Vector3& operator-=(const Vector3& other);
		Vector3& operator*=(const Vector3& other);
		Vector3& operator/=(const Vector3& other);

		Vector3 operator*(float scalar) const;
		Vector3 operator/(float scalar) const;
		float operator*(const Vector3 &other) const;
		Vector3 operator+(const Vector3 &other) const;
		Vector3 operator-(const Vector3 &other) const;
		Vector3 operator-() const;
		float& operator[](int index);
		const float& operator[](int index) const;

		bool operator==(const Vector3 &other) const;
		bool operator!=(const Vector3 &other) const;
		bool operator> (const Vector3 &other) const;
		bool operator>=(const Vector3 &other) const;
		bool operator< (const Vector3 &other) const;
		bool operator<=(const Vector3 &other) const;

		bool equals(const Vector3 &other, float epsilon = 0.001f) const;
		bool isSameLength(const Vector3 &other, float epsilon = 0.001f) const;
		bool isSameDirection(const Vector3 &other) const;
		bool isOppositeDirection(const Vector3 &other) const;
		bool isLeft(const Vector3 &other) const;
		bool isRight(const Vector3 &other) const;
		bool isContainingNaN() const;
		bool isContainingInfinity() const;
		bool isUnit(float epsilon = 0.001f) const;
		bool isCloseTo(const Vector3 &other, float maxDistance) const;
		bool isZero() const;

		Vector3 rotate(float radians, const Vector3 &axisOfRotation) const;
		Vector3 rotate(float radians, const Vector3 &axisOfRotation, const Vector3 &center) const;
		Vector3 withLenght(float length) const;
		Vector3 normalize() const;
		Vector3 abs() const;
		Vector3 clampComponents(float min, float max) const;
		Vector3 project(const Vector3 &other) const;
		Vector3 reflect(const Vector3 &normal) const;
		Vector3 cross(const Vector3 &other) const;

		float getLength() const;
		float getLengthSq() const;
		float getDistanceTo(const Vector3 &other) const;
		float getMax() const;
		float getMin() const;
		float getMaxAbs() const;
		float getMinAbs() const;
		float getMaxAbsKeepSign() const;
		float getMinAbsKeepSign() const;
		float getAngle() const;
		float getAngle(const Vector3 &other) const;


		//Begin Swizzles
		Vector2 xx() const;
		Vector2 xy() const;
		Vector2 xz() const;
		Vector2 yx() const;
		Vector2 yy() const;
		Vector2 yz() const;
		Vector2 zx() const;
		Vector2 zy() const;
		Vector2 zz() const;

		Vector3 xxx() const;
		Vector3 xxy() const;
		Vector3 xxz() const;
		Vector3 xyx() const;
		Vector3 xyy() const;
		Vector3 xyz() const;
		Vector3 xzx() const;
		Vector3 xzy() const;
		Vector3 xzz() const;
		Vector3 yxx() const;
		Vector3 yxy() const;
		Vector3 yxz() const;
		Vector3 yyx() const;
		Vector3 yyy() const;
		Vector3 yyz() const;
		Vector3 yzx() const;
		Vector3 yzy() const;
		Vector3 yzz() const;
		Vector3 zxx() const;
		Vector3 zxy() const;
		Vector3 zxz() const;
		Vector3 zyx() const;
		Vector3 zyy() const;
		Vector3 zyz() const;
		Vector3 zzx() const;
		Vector3 zzy() const;
		Vector3 zzz() const;
	};
}