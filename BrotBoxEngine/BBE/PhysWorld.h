#pragma once

class b2World;

#include "../BBE/Vector2.h"

namespace bbe
{
	class PhysWorld
	{
	private:
		b2World* m_pworld = nullptr;
		constexpr static float TIME_BETWEEN_STEPS = 1.f / 120.f;
		float timeSinceLastStep = 0;
		float physicsScale = 10;

		void destroy();
		void init(const bbe::Vector2& gravity);

	public:
		PhysWorld();
		explicit PhysWorld(const bbe::Vector2& gravity);

		PhysWorld(const PhysWorld&) = delete;
		PhysWorld(PhysWorld&& other);
		PhysWorld& operator=(const PhysWorld&) = delete;
		PhysWorld& operator=(PhysWorld&& other);

		virtual ~PhysWorld();

		b2World* getRaw();
		const b2World* getRaw() const;

		bbe::Vector2 getGravity() const;
		void setGravity(const bbe::Vector2& gravity);

		void update(float timeSinceLastFrame);

		float getPhysicsScale() const;
		void setPhysicsScale(float physicsScale);
	};
}
