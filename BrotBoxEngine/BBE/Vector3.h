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
		Vector3(float x, float y, float z);
		Vector3(float x, const Vector2 &yz);
		Vector3(const Vector2 &xy, float z);

		Vector3 operator+(const Vector3 &other) const;
		Vector3 operator-(const Vector3 &other) const;

		Vector3 operator*(float scalar) const;
		Vector3 operator/(float scalar) const;

		float& operator[](int index);
		const float& operator[](int index) const;
	};
}