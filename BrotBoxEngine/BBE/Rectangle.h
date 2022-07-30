#pragma once
#include "../BBE/Vector2.h"
#include "../BBE/Shape2.h"

namespace bbe
{
	class VulkanDevice;
	class Circle;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class Rectangle : public bbe::Shape2
	{
		friend class PrimitiveBrush2D;
		friend class ::bbe::INTERNAL::vulkan::VulkanManager;
	private:
		float m_x;
		float m_y;
		float m_width;
		float m_height;

	public:
		Rectangle();
		Rectangle(float x, float y, float width, float height);
		Rectangle(const Vector2 &vec, float width, float height);
		Rectangle(float x, float y, const Vector2 &dim);
		Rectangle(const Vector2 &vec, const Vector2 &dim);

		Vector2 getPos() const;
		Vector2 getDim() const;

		Rectangle combine(const Rectangle& other) const;
		Rectangle offset(const Vector2& off) const;

		float getLeft() const;
		float getRight() const;
		float getTop() const;
		float getBottom() const;

		float getX() const;
		float getY() const;
		float getWidth() const;
		float getHeight() const;
		virtual bbe::Vector2 getCenter() const override;
		virtual void getVertices(bbe::List<bbe::Vector2>& outVertices) const override;

		void setX(float x);
		void setY(float y);
		void setPos(float x, float y);
		void setPos(const Vector2 &vec);
		void setWidth(float width);
		void setHeight(float height);
		void setDim(float width, float height);
		void setDim(const Vector2 &vec);
		void set(float x, float y, float width, float height);
		void shrinkInPlace(float val);
		Rectangle shrinked(float val) const;
		Rectangle stretchedSpace(float x, float y) const;


		void translate(float x, float y);
		virtual void translate(const bbe::Vector2 &vec) override;

		float getDistanceTo(const Vector2 &vec);

		bool isPointInRectangle(const Vector2 point) const;
		using Shape2::intersects;
		bool intersects(const Rectangle& rectangle) const;
		bool intersects(const Circle& circle) const;
	};
}
