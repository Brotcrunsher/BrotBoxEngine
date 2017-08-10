#pragma once

#include "../BBE/Vector4.h";

namespace bbe
{
	class Matrix4
	{
	private:
		Vector4 m_cols[4];

	public:
		Matrix4();

		float get(int row, int col) const;
		void set(int row, int col, float val);

		float& operator[](int index);
		const float& operator[](int index) const;
		Matrix4 operator*(const Matrix4 other) const;
	};
}