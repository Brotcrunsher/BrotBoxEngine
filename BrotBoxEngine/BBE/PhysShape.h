#pragma once


class b2Body;
class b2Fixture;

namespace bbe
{
	class Game;
	template<typename T> class Vector2_t;
	using Vector2 = Vector2_t<float>;
	class Rectangle;

	class PhysShape
	{
	protected:
		b2Body*    m_pbody    = nullptr;
		b2Fixture* m_pfixture = nullptr;
		Game*      m_pcontext = nullptr;

	public:

		PhysShape(Game* context);
		
		virtual float getX() const = 0;
		virtual float getY() const = 0;
		virtual Vector2 getCenterOfMass() const = 0;

		virtual float getAngle() const;
		virtual Vector2 getPos() const;
		virtual b2Body* getRawBody();
		virtual float getSpeedX() const;
		virtual float getSpeedY() const;
		virtual Vector2 getSpeed() const;
		virtual float getAngularSpeed() const;
		virtual void setSpeed(const bbe::Vector2& speed);
		virtual void addSpeed(const bbe::Vector2& speed);

		virtual float getDensity() const;
		virtual void setDensity(float density);
		virtual float getFriction() const;
		virtual void setFriction(float friction);
		virtual float getRestitution() const;
		virtual void setRestitution(float restitution) const;

		virtual void freeze();
		virtual void addJointRope(PhysShape& other, float maxLength);
		virtual void addJointRevolute(PhysShape& other, const bbe::Vector2& anchor);
	};
}
