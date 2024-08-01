#pragma once
#include "../BBE/Matrix4.h"
#include "../BBE/Vector3.h"

namespace bbe
{

	class VulkanDevice;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class IcoSphere
	{
		friend class PrimitiveBrush3D;
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		Matrix4 m_transform;



	public:
		IcoSphere();
		explicit IcoSphere(const Vector3& pos, const Vector3& scale = bbe::Vector3(1, 1, 1), const Vector3& rotationVector = bbe::Vector3(1, 0, 0), float radians = 0);
		explicit IcoSphere(const Matrix4 &transform);

		void set(const Vector3 &pos, const Vector3 &scale, const Vector3 &rotationVector, float radians);

		Vector3 getPos() const;
		float getX() const;
		float getY() const;
		float getZ() const;

		Vector3 getScale() const;
		float getWidth() const;
		float getHeight() const;
		float getDepth() const;

		const Matrix4& getTransform() const;
	};
}
