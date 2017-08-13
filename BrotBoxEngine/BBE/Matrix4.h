#pragma once

#include "../BBE/Vector4.h";
#include "../BBE/Vector3.h";

namespace bbe
{
	class Matrix4
	{
	private:
		Vector4 m_cols[4];

	public:
		Matrix4();
		
		static Matrix4 createTranslationMatrix(const Vector3 &translation);
		static Matrix4 createRotationMatrix(float radians, const Vector3 &rotationAxis);
		static Matrix4 createScaleMatrix(const Vector3 &scale);
		static Matrix4 createPerspectiveMatrix(float fieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane);
		static Matrix4 createViewMatrix(const Vector3 &cameraPos, const Vector3 &lookTarget, const Vector3 &upDirection);

		float get(int row, int col) const;
		void set(int row, int col, float val);

		float& operator[](int index);
		const float& operator[](int index) const;
		Matrix4 operator*(const Matrix4 &other) const;
		Vector3 operator*(const Vector3 &other) const;
		Vector4 operator*(const Vector4 &other) const;
	};
}