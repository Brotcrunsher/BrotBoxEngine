#pragma once

#include "../BBE/Vector4.h"
#include "../BBE/Vector3.h"

namespace bbe
{
	class Matrix4
	{
	private:
		Vector4 m_cols[4];

	public:
		Matrix4();
		Matrix4(const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3);
		
		static Matrix4 createTranslationMatrix(const Vector3 &translation);
		static Matrix4 createRotationMatrix(float radians, const Vector3 &rotationAxis);
		static Matrix4 createRotationMatrix(const Vector3& from, const Vector3& to);
		static Matrix4 createScaleMatrix(const Vector3 &scale);
		static Matrix4 createPerspectiveMatrix(float fieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane);
		static Matrix4 createViewMatrix(const Vector3 &cameraPos, const Vector3 &lookTarget, const Vector3 &upDirection);
		static Matrix4 createTransform(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);

		float get(int row, int col) const;
		void set(int row, int col, float val);

		float& operator[](int index);
		const float& operator[](int index) const;
		Matrix4 operator*(const Matrix4 &other) const;
		Vector3 operator*(const Vector3 &other) const;
		Vector4 operator*(const Vector4 &other) const;

		Vector4 getColumn(int colIndex) const;
		Vector4 getRow(int rowIndex) const;

		Vector3 extractTranslation() const;
		Vector3 extractScale() const;
		Matrix4 extractRotation() const;
	};
}