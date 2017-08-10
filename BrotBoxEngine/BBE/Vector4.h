#pragma once


namespace bbe
{
	class Vector2;
	class Vector3;

	class Vector4
	{
	public:
		float x;
		float y;
		float z;
		float w;

		Vector4();
		Vector4(float x, float y, float z, float w);
		Vector4(float x, float y, const Vector2 &zw);
		Vector4(const Vector2 &xy, float z, float w);
		Vector4(const Vector2 &xy, const Vector2 &zw);
		Vector4(float x, const Vector2 &yz, float w);
		Vector4(const Vector3 &xyz, float w);
		Vector4(float x, const Vector3 &yzw);

		Vector4 operator+(const Vector4 &other) const;
		Vector4 operator-(const Vector4 &other) const;

		Vector4 operator*(float scalar) const;
		Vector4 operator/(float scalar) const;

		float& operator[](int index);
		const float& operator[](int index) const;
	};
}