#pragma once

#include "PhysShape.h"

class b2Body;
class b2Fixture;

namespace bbe
{
	class Game;
	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;
	class Rectangle;

	class PhysRectangle : public PhysShape
	{
	private:
		float m_width = 0;
		float m_height = 0;

		void init(Game* context, float x, float y, float width, float height, float angle);

	public:

		PhysRectangle(Game* context, float x, float y, float width, float height, float angle = 0);
		PhysRectangle(Game* context, const Vector2& vec, float width, float height, float angle = 0);
		PhysRectangle(Game* context, float x, float y, const Vector2& dim, float angle = 0);
		PhysRectangle(Game* context, const Vector2& vec, const Vector2& dim, float angle = 0);
		PhysRectangle(Game* context, const Rectangle &rect, float angle = 0);

		float getX() const override;
		float getY() const override;
		Vector2 getCenterOfMass() const override;
		float getWidth() const;
		float getHeight() const;
		Vector2 getDim() const;
	};
}
