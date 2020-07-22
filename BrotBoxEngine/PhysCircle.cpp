#include "BBE/PhysWorld.h"
#include "BBE/Math.h"
#include "BBE/Game.h"
#include "BBE/Vector2.h"
#include "BBE/Rectangle.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"
#include "BBE/PhysRectangle.h"
#include "BBE/PhysCircle.h"


bbe::PhysCircle::PhysCircle(Game* context, float x, float y, float radius, float angle)
	: PhysShape(context)
{
	init(context, x, y, radius, angle);
}

bbe::PhysCircle::PhysCircle(Game* context, const Vector2& vec, float radius, float angle)
	: PhysShape(context)
{
	init(context, vec.x, vec.y, radius, angle);
}

void bbe::PhysCircle::init(Game* context, float x, float y, float radius, float angle)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	const float physicsScale = context->getPhysWorld()->getPhysicsScale();
	bodyDef.position.Set((x + radius) / physicsScale, (y + radius) / physicsScale);
	bodyDef.angle = angle;
	m_pbody = context->getPhysWorld()->getRaw()->CreateBody(&bodyDef);
	b2CircleShape dynamicBox;
	dynamicBox.m_radius = radius / physicsScale;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.5f;
	fixtureDef.restitution = 0.0f;
	m_pfixture = m_pbody->CreateFixture(&fixtureDef);
	m_pcontext = context;
	m_radius = radius;
}

float bbe::PhysCircle::getX() const
{
	return m_pbody->GetPosition().x * m_pcontext->getPhysWorld()->getPhysicsScale() - m_radius;
}

float bbe::PhysCircle::getY() const
{
	return m_pbody->GetPosition().y * m_pcontext->getPhysWorld()->getPhysicsScale() - m_radius;
}

bbe::Vector2 bbe::PhysCircle::getCenterOfMass() const
{
	return Vector2(getX() + m_radius, getY() + m_radius);
}

float bbe::PhysCircle::getRadius() const
{
	return m_radius;
}
