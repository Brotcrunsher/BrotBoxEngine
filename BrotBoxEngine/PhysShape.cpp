#include "BBE/PhysWorld.h"
#include "BBE/Math.h"
#include "BBE/Game.h"
#include "BBE/Vector2.h"
#include "BBE/Rectangle.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_rope_joint.h"
#include "box2d/b2_revolute_joint.h"
#include "box2d/b2_friction_joint.h"

#include "BBE/PhysRectangle.h"
#include "BBE/PhysCircle.h"
#include "BBE/PhysShape.h"

void bbe::PhysShape::freeze()
{
	m_pbody->SetType(b2BodyType::b2_staticBody);
}

void bbe::PhysShape::destroy()
{
	m_pcontext->getPhysWorld()->getRaw()->DestroyBody(m_pbody);
}

void bbe::PhysShape::addJointRope(PhysShape& other, float maxLength)
{
	b2RopeJointDef ropeJoint;
	ropeJoint.bodyA = this->getRawBody();
	ropeJoint.bodyB = other.getRawBody();
	ropeJoint.maxLength = maxLength / m_pcontext->getPhysWorld()->getPhysicsScale();
	m_pcontext->getPhysWorld()->getRaw()->CreateJoint(&ropeJoint);
}

void bbe::PhysShape::addJointRevolute(PhysShape& other, const bbe::Vector2& anchor)
{
	const float scale = m_pcontext->getPhysWorld()->getPhysicsScale();
	b2RevoluteJointDef revoltJoint;
	revoltJoint.Initialize(this->getRawBody(), other.getRawBody(), { anchor.x / scale, anchor.y / scale});
	m_pcontext->getPhysWorld()->getRaw()->CreateJoint(&revoltJoint);
}

bbe::PhysShape::PhysShape(Game* context)
	:m_pcontext(context)
{
}

bbe::Vector2 bbe::PhysShape::getPos() const
{
	return Vector2(getX(), getY());
}

b2Body* bbe::PhysShape::getRawBody()
{
	return m_pbody;
}

float bbe::PhysShape::getSpeedX() const
{
	return m_pbody->GetLinearVelocity().x * m_pcontext->getPhysWorld()->getPhysicsScale();
}

float bbe::PhysShape::getSpeedY() const
{
	return m_pbody->GetLinearVelocity().y * m_pcontext->getPhysWorld()->getPhysicsScale();
}

bbe::Vector2 bbe::PhysShape::getSpeed() const
{
	return Vector2(getSpeedX(), getSpeedY());
}

float bbe::PhysShape::getAngularSpeed() const
{
	return m_pbody->GetAngularVelocity();
}

void bbe::PhysShape::setSpeed(const bbe::Vector2& speed)
{
	m_pbody->SetLinearVelocity(
		{
			speed.x / m_pcontext->getPhysWorld()->getPhysicsScale(),
			speed.y / m_pcontext->getPhysWorld()->getPhysicsScale()
		}
	);
}

void bbe::PhysShape::addSpeed(const bbe::Vector2& speed)
{
	setSpeed(getSpeed() + speed);
}

float bbe::PhysShape::getDensity() const
{
	return m_pfixture->GetDensity();
}

void bbe::PhysShape::setDensity(float density)
{
	m_pfixture->SetDensity(density);
}

float bbe::PhysShape::getFriction() const
{
	return m_pfixture->GetFriction();
}

void bbe::PhysShape::setFriction(float friction)
{
	m_pfixture->SetFriction(friction);
}

float bbe::PhysShape::getRestitution() const
{
	return m_pfixture->GetRestitution();
}

void bbe::PhysShape::setRestitution(float restitution) const
{
	m_pfixture->SetRestitution(restitution);
}

float bbe::PhysShape::getAngle() const
{
	return m_pbody->GetAngle();
}