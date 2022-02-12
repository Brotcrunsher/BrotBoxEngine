// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include "BBE/PhysWorld.h"
#include "BBE/Math.h"
#include "BBE/Game.h"
#include "BBE/Vector2.h"
#include "BBE/Rectangle.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_fixture.h"
#include "BBE/PhysRectangle.h"

void bbe::PhysRectangle::init(Game* context, float x, float y, float width, float height, float angle)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	const float physicsScale = context->getPhysWorld()->getPhysicsScale();
	bodyDef.position.Set((x + width / 2) / physicsScale, (y + height / 2) / physicsScale);
	bodyDef.angle = angle;
	m_pbody = context->getPhysWorld()->getRaw()->CreateBody(&bodyDef);
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox((width / 2) / physicsScale, (height / 2) / physicsScale);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.5f;
	fixtureDef.restitution = 0.0f;
	m_pfixture = m_pbody->CreateFixture(&fixtureDef);

	m_pcontext = context;
	m_width = width;
	m_height = height;
}

bbe::PhysRectangle::PhysRectangle(Game* context, float x, float y, float width, float height, float angle)
	: PhysShape(context)
{
	init(context, x, y, width, height, angle);
}

bbe::PhysRectangle::PhysRectangle(Game* context, const Vector2& vec, float width, float height, float angle)
	: PhysShape(context)
{
	init(context, vec.x, vec.y, width, height, angle);
}

bbe::PhysRectangle::PhysRectangle(Game* context, float x, float y, const Vector2& dim, float angle)
	: PhysShape(context)
{
	init(context, x, y, dim.x, dim.y, angle);
}

bbe::PhysRectangle::PhysRectangle(Game* context, const Vector2& vec, const Vector2& dim, float angle)
	: PhysShape(context)
{
	init(context, vec.x, vec.y, dim.x, dim.y, angle);
}

bbe::PhysRectangle::PhysRectangle(Game* context, const Rectangle& rect, float angle)
	: PhysShape(context)
{
	init(context, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), angle);
}

float bbe::PhysRectangle::getX() const
{
	return m_pbody->GetPosition().x * m_pcontext->getPhysWorld()->getPhysicsScale() - m_width / 2;
}

float bbe::PhysRectangle::getY() const
{
	return m_pbody->GetPosition().y * m_pcontext->getPhysWorld()->getPhysicsScale() - m_height / 2;
}

bbe::Vector2 bbe::PhysRectangle::getCenterOfMass() const
{
	return Vector2(getX() + getWidth() / 2, getY() + getHeight() / 2);
}

float bbe::PhysRectangle::getWidth() const
{
	return m_width;
}

float bbe::PhysRectangle::getHeight() const
{
	return m_height;
}

bbe::Vector2 bbe::PhysRectangle::getDim() const
{
	return Vector2(getWidth(), getHeight());
}
#endif
