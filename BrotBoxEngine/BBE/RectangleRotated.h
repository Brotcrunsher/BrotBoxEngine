#pragma once
#include "../BBE/Vector2.h"
#include "../BBE/Shape2.h"

namespace bbe
{
	template<typename Vec> class Rectangle_t;
	using Rectangle = Rectangle_t<bbe::Vector2>;

	class RectangleRotated : public bbe::Shape2<bbe::Vector2>
	{
		friend class PrimitiveBrush2D;

	private:
		float m_x;
		float m_y;
		float m_width;
		float m_height;
		float m_rotation;

	public:
		RectangleRotated();
		RectangleRotated(float x, float y, float width, float height, float rotation);
		RectangleRotated(const Vector2 &vec, float width, float height, float rotation);
		RectangleRotated(float x, float y, const Vector2 &dim, float rotation);
		RectangleRotated(const Vector2 &vec, const Vector2 &dim, float rotation);
		RectangleRotated(const Rectangle& rect, float rotation);

		bool operator==(const RectangleRotated& other) const;

		float getX() const;
		float getY() const;
		bbe::Vector2 getPos() const;
		float getWidth() const;
		float getHeight() const;
		bbe::Vector2 getDim() const;
		float getRotation() const;

		void setX(float x);
		void setY(float y);
		void setWidth(float width);
		void setHeight(float height);
		void setRotation(float rotation);
		virtual void translate(const Vector2& vec) override;

		virtual bbe::Vector2 getCenter() const override;

		using Shape2::getVertices;
		virtual void getVertices(bbe::List<bbe::Vector2>& outVertices) const override;
	};
}
