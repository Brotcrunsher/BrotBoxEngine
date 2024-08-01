#pragma once
#include "../BBE/Matrix4.h"
#include "../BBE/Vector3.h"
#include "../BBE/List.h"
#include "../BBE/Shape2.h"
#include "../BBE/PosNormalPair.h"

namespace bbe
{
	enum class FaceFlag
	{
		NONE   = 0,
		BOTTOM = 1 << 0,
		TOP    = 1 << 1,
		LEFT   = 1 << 2,
		RIGHT  = 1 << 3,
		FRONT  = 1 << 4,
		BACK   = 1 << 5,

		CIRCUMFERENCE =                LEFT | RIGHT | FRONT | BACK,
		BOTTOMLESS    =          TOP | LEFT | RIGHT | FRONT | BACK,
		TOPLESS       = BOTTOM |       LEFT | RIGHT | FRONT | BACK,
		ALL           = BOTTOM | TOP | LEFT | RIGHT | FRONT | BACK,
	};

	class Cube : public bbe::Shape3
	{
		friend class PrimitiveBrush3D;
	private:
		Matrix4 m_transform;

	public:
		Cube();
		explicit Cube(const Vector3& pos, const Vector3& scale = Vector3(1, 1, 1), const Vector3& rotationVector = Vector3(0, 0, 0), float radians = 0);
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

		static bbe::List<bbe::PosNormalPair> getRenderVerticesDefault(FaceFlag ff = FaceFlag::ALL);
		bbe::List<bbe::PosNormalPair> getRenderVertices(FaceFlag ff = FaceFlag::ALL) const;
		static bbe::List<uint32_t> getRenderIndicesDefault(FaceFlag ff = FaceFlag::ALL);

		bbe::Vector3 approach(const bbe::Cube& other, const bbe::Vector3& approachVector) const;
	};
}
