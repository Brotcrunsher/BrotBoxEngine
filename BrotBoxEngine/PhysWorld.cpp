#include "BBE/PhysWorld.h"
#include "BBE/Math.h"

#include "box2d/b2_world.h"

void bbe::PhysWorld::destroy()
{
	if (m_pworld != nullptr)
	{
		delete m_pworld;
		m_pworld = nullptr;
	}
}

void bbe::PhysWorld::init(const bbe::Vector2& gravity)
{
	destroy();
	m_pworld = new b2World(b2Vec2(gravity.x, -gravity.y));
}

bbe::PhysWorld::PhysWorld()
{
	init({ 0, 0 });
}

bbe::PhysWorld::PhysWorld(const bbe::Vector2& gravity)
{
	init(gravity);
}

bbe::PhysWorld::PhysWorld(PhysWorld&& other)
{
	m_pworld = other.m_pworld;
	other.m_pworld = nullptr;
}

bbe::PhysWorld& bbe::PhysWorld::operator=(PhysWorld&& other)
{
	destroy();
	m_pworld = other.m_pworld;
	other.m_pworld = nullptr;
	return *this;
}

bbe::PhysWorld::~PhysWorld()
{
	destroy();
}

b2World* bbe::PhysWorld::getRaw()
{
	return m_pworld;
}

const b2World* bbe::PhysWorld::getRaw() const
{
	return m_pworld;
}

bbe::Vector2 bbe::PhysWorld::getGravity() const
{
	const b2Vec2 gravity = m_pworld->GetGravity();
	return bbe::Vector2(gravity.x, gravity.y);
}

void bbe::PhysWorld::setGravity(const bbe::Vector2& gravity)
{
	m_pworld->SetGravity(b2Vec2(gravity.x, gravity.y));
}

void bbe::PhysWorld::update(float timeSinceLastFrame)
{
	timeSinceLastStep += timeSinceLastFrame;
	timeSinceLastStep = bbe::Math::clamp(timeSinceLastStep, 0.0f, TIME_BETWEEN_STEPS * 2);

	if (timeSinceLastStep > TIME_BETWEEN_STEPS)
	{
		timeSinceLastStep -= TIME_BETWEEN_STEPS;
		m_pworld->Step(TIME_BETWEEN_STEPS, 8, 3);
	}
}

float bbe::PhysWorld::getPhysicsScale() const
{
	return physicsScale;
}

void bbe::PhysWorld::setPhysicsScale(float physicsScale)
{
	this->physicsScale = physicsScale;
}
