#pragma once
#include "../BBE/Matrix4.h"
#include "../BBE/Vector3.h"
#include "../BBE/List.h"
#include "../BBE/Shape2.h"
#include "../BBE/PosNormalPair.h"

namespace bbe
{

	class Cube : public bbe::Shape3
	{
		friend class PrimitiveBrush3D;
	private:
		Matrix4 m_transform;

	public:
		Cube();
		Cube(const Vector3& pos, const Vector3& scale = Vector3(1, 1, 1), const Vector3& rotationVector = Vector3(0, 0, 0), float radians = 0);
		Cube(const Vector3& pos, const Vector3& scale, const Matrix4& matRotation);
		Cube(const Matrix4& matTranslation, const Matrix4& matScale, const Matrix4& matRotation);
		Cube(const Vector3& pos, const Matrix4& matScale, const Matrix4& matRotation);
		explicit Cube(const Matrix4 &transform);

		void set(const Vector3& pos, const Vector3& scale = Vector3(1, 1, 1), const Vector3& rotationVector = Vector3(0, 0, 0), float radians = 0);
		void set(const Vector3& pos, const Vector3& scale, const Matrix4 &matRotation);
		void set(const Matrix4& matTranslation, const Matrix4& matScale, const Matrix4 &matRotation);
		void set(const Vector3& pos, const Matrix4& matScale, const Matrix4& matRotation);
		void setRotation(const Vector3 &rotationVector, float radians);
		void setPosition(const Vector3& pos);

		virtual void translate(const bbe::Vector3& translation) override;

		Vector3 getPos() const;
		float getX() const;
		float getY() const;
		float getZ() const;
		virtual Vector3 getCenter() const override;

		Vector3 getScale() const;
		float getWidth() const;
		float getHeight() const;
		float getDepth() const;

		const Matrix4& getTransform() const;

		virtual bbe::List<bbe::Vector3> getNormals() const override;
		using Shape3::getVertices;
		virtual void getVertices(bbe::List<bbe::Vector3> &outVertices) const override;

		static bbe::List<bbe::PosNormalPair> getRenderVerticesDefault();
		bbe::List<bbe::PosNormalPair> getRenderVertices() const;
		static bbe::List<uint32_t> getRenderIndicesDefault();

		bbe::Vector3 approach(const bbe::Cube& other, const bbe::Vector3& approachVector) const;
	};
}
