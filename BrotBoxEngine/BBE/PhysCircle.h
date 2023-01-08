#pragma once

#include "../BBE/PhysShape.h"

class b2Body;
class b2Fixture;

namespace bbe
{
	class Game;
	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;
	template<typename Vec> class Rectangle_t;
	using Rectangle = Rectangle_t<bbe::Vector2>;

	class PhysCircle : public PhysShape
	{
	private:
		float m_radius = 0;

		void init(Game* context, float x, float y, float radius, float angle);

	public:

		PhysCircle(Game* context, float x, float y, float radius, float angle = 0);
		PhysCircle(Game* context, const Vector2& vec, float radius,float angle = 0);

		float getX() const override;
		float getY() const override;
		Vector2 getCenterOfMass() const override;
		float getRadius() const;
		
	};
}
