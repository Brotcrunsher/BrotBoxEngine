#include "BBE/CameraControlNoClip.h"
#include "BBE/Game.h"
#include "BBE/Math.h"
#include "BBE/Matrix4.h"
#include "BBE/KeyboardKeys.h"
#include "BBE/Vector2.h"

bbe::CameraControlNoClip::CameraControlNoClip(Game * game)
	: m_pgame(game)
{
}

void bbe::CameraControlNoClip::update(float timeSinceLastFrame)
{
	m_horizontalMouse += m_pgame->getMouseXDelta() / -300.0f;
	m_verticalMouse += m_pgame->getMouseYDelta() / 300.f;
	m_verticalMouse = bbe::Math::clamp<float>(m_verticalMouse, -bbe::Math::PI / 2 + 0.01f, bbe::Math::PI / 2 - 0.01f);
	if (m_horizontalMouse > bbe::Math::PI * 2)
	{
		m_horizontalMouse -= bbe::Math::PI * 2;
	}
	else if (m_horizontalMouse < 0)
	{
		m_horizontalMouse += bbe::Math::PI * 2;
	}

	bbe::Matrix4 rotHorizontal = bbe::Matrix4::createRotationMatrix(m_horizontalMouse, bbe::Vector3(0, 0, 1));
	bbe::Vector3 a = rotHorizontal * bbe::Vector3(1, 0, 0);
	bbe::Vector3 b = a.rotate(bbe::Math::PI / 2, bbe::Vector3(0, 0, 1));
	bbe::Matrix4 rotVertical = bbe::Matrix4::createRotationMatrix(m_verticalMouse, b);
	m_forward = rotVertical * (rotHorizontal * bbe::Vector3(1, 0, 0));

	if (m_pgame->isKeyPressed(bbe::Key::_1))
	{
		m_pgame->setCursorMode(bbe::CursorMode::DISABLED);
	}
	else if (m_pgame->isKeyPressed(bbe::Key::_2))
	{
		m_pgame->setCursorMode(bbe::CursorMode::HIDDEN);
	}
	else if (m_pgame->isKeyPressed(bbe::Key::_3))
	{
		m_pgame->setCursorMode(bbe::CursorMode::NORMAL);
	}

	float speedFactor = 1;
	if (m_pgame->isKeyDown(bbe::Key::LEFT_SHIFT))
	{
		m_timeSinceShiftPress += timeSinceLastFrame;
		speedFactor = 10 * m_timeSinceShiftPress * m_timeSinceShiftPress;
	}
	else
	{
		m_timeSinceShiftPress = 1;
	}


	bbe::Vector3 movementForward = m_forward;
	if (m_isZPosConstrained)
	{
		movementForward.z = 0;
		movementForward = movementForward.normalize();
	}

	bool moving = false;
	if (m_pgame->isKeyDown(bbe::Key::W))
	{
		m_cameraPos = m_cameraPos + movementForward * timeSinceLastFrame * 10 * speedFactor;
		moving = true;
	}
	if (m_pgame->isKeyDown(bbe::Key::S))
	{
		m_cameraPos = m_cameraPos - movementForward * timeSinceLastFrame * 10 * speedFactor;
		moving = true;
	}
	if (m_pgame->isKeyDown(bbe::Key::A))
	{
		const bbe::Vector3 rot = movementForward.rotate(bbe::Math::PI / 2, bbe::Vector3(0, 0, 1));
		m_cameraPos = m_cameraPos + bbe::Vector3(rot.x, rot.y, 0).normalize() * timeSinceLastFrame * 10 * speedFactor;
		moving = true;
	}
	if (m_pgame->isKeyDown(bbe::Key::D))
	{
		const bbe::Vector3 rot = movementForward.rotate(bbe::Math::PI / 2, bbe::Vector3(0, 0, 1));
		m_cameraPos = m_cameraPos - bbe::Vector3(rot.x, rot.y, 0).normalize() * timeSinceLastFrame * 10 * speedFactor;
		moving = true;
	}
	if (m_pgame->isKeyDown(bbe::Key::SPACE))
	{
		m_cameraPos = m_cameraPos + bbe::Vector3(0, 0, 1) * timeSinceLastFrame * 10 * speedFactor;
		moving = true;
	}
	if (m_pgame->isKeyDown(bbe::Key::C))
	{
		m_cameraPos = m_cameraPos - bbe::Vector3(0, 0, 1) * timeSinceLastFrame * 10 * speedFactor;
		moving = true;
	}

	if (!moving)
	{
		m_timeSinceShiftPress = 1;
	}

	if (m_isZPosConstrained)
	{
		m_cameraPos.z = m_constraintZPos;
	}
}

bbe::Vector3 bbe::CameraControlNoClip::getCameraPos() const
{
	return m_cameraPos;
}

bbe::Vector3 bbe::CameraControlNoClip::getCameraForward() const
{
	return m_forward;
}

bbe::Vector3 bbe::CameraControlNoClip::getCameraTarget() const
{
	return m_cameraPos + m_forward;
}

void bbe::CameraControlNoClip::setCameraPos(float x, float y, float z)
{
	setCameraPos({ x, y, z });
}

void bbe::CameraControlNoClip::setCameraPos(const Vector3 &pos)
{
	m_cameraPos = pos;
}

void bbe::CameraControlNoClip::setCameraForward(float x, float y, float z)
{
	setCameraForward({ x, y, z });
}

void bbe::CameraControlNoClip::setCameraForward(const Vector3& forward)
{
	m_forward = forward;
}

void bbe::CameraControlNoClip::constraintZPos(float z)
{
	m_isZPosConstrained = true;
	m_constraintZPos = z;
}
