#pragma once


class b2Body;
class b2Fixture;

namespace bbe
{
	class Game;
	class Vector2;
	class Rectangle;

	class PhysRectangle
	{
	private:
		b2Body*    m_pbody    = nullptr;
		b2Fixture* m_pfixture = nullptr;
		Game*      m_pcontext = nullptr;
		float m_width = 0;
		float m_height = 0;

		void init(Game* context, float x, float y, float width, float height, float angle);

	public:

		PhysRectangle(Game* context, float x, float y, float width, float height, float angle = 0);
		PhysRectangle(Game* context, const Vector2& vec, float width, float height, float angle = 0);
		PhysRectangle(Game* context, float x, float y, const Vector2& dim, float angle = 0);
		PhysRectangle(Game* context, const Vector2& vec, const Vector2& dim, float angle = 0);
		PhysRectangle(Game* context, const Rectangle &rect, float angle = 0);

		float getX() const;
		float getY() const;
		float getWidth() const;
		float getHeight() const;
		float getAngle() const;

		void freeze();
	};
}
