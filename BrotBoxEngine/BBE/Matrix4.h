#pragma once

#include "../BBE/Vector4.h";

namespace bbe
{
	class Matrix4
	{
	private:
		float col0[4];
		float col1[4];
		float col2[4];
		float col3[4];

	public:
		Matrix4();

		float& operator[](int index);
		const float& operator[](int index) const;
		Matrix4 operator*(const Matrix4 other) const;
	};
}