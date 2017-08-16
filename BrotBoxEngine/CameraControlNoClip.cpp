#include "stdafx.h"
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
	horizontalMouse += m_pgame->getMouseXDelta() / -100.f;
	verticalMouse += m_pgame->getMouseYDelta() / 100.f;
	verticalMouse = bbe::Math::clamp(verticalMouse, -bbe::Math::PI / 2 + 0.001f, bbe::Math::PI / 2 - 0.001f);
	if (horizontalMouse > bbe::Math::PI * 2)
	{
		horizontalMouse -= bbe::Math::PI * 2;
	}
	else if (horizontalMouse < 0)
	{
		horizontalMouse += bbe::Math::PI * 2;
	}

	bbe::Matrix4 rotHorizontal = bbe::Matrix4::createRotationMatrix(horizontalMouse, bbe::Vector3(0, 0, 1));
	bbe::Vector3 a = rotHorizontal * bbe::Vector3(1, 0, 0);
	bbe::Vector3 b = a.rotate(bbe::Math::PI / 2, bbe::Vector3(0, 0, 1));
	bbe::Matrix4 rotVertical = bbe::Matrix4::createRotationMatrix(verticalMouse, b);
	m_forward = rotVertical * (rotHorizontal * bbe::Vector3(1, 0, 0));

	if (m_pgame->isKeyPressed(bbe::KEY_1))
	{
		m_pgame->setCursorMode(bbe::CursorMode::DISABLED);
	}
	else if (m_pgame->isKeyPressed(bbe::KEY_2))
	{
		m_pgame->setCursorMode(bbe::CursorMode::HIDDEN);
	}
	else if (m_pgame->isKeyPressed(bbe::KEY_3))
	{
		m_pgame->setCursorMode(bbe::CursorMode::NORMAL);
	}

	float speedFactor = 1;
	if (m_pgame->isKeyDown(bbe::KEY_LEFT_SHIFT))
	{
		speedFactor = 10;
	}

	if (m_pgame->isKeyDown(bbe::KEY_W))
	{
		m_cameraPos = m_cameraPos + m_forward * timeSinceLastFrame * 10 * speedFactor;
	}
	if (m_pgame->isKeyDown(bbe::KEY_S))
	{
		m_cameraPos = m_cameraPos - m_forward * timeSinceLastFrame * 10 * speedFactor;
	}
	if (m_pgame->isKeyDown(bbe::KEY_A))
	{
		m_cameraPos = m_cameraPos + bbe::Vector3(m_forward.rotate(bbe::Math::PI / 2, bbe::Vector3(0, 0, 1)).xy(), 0).normalize() * timeSinceLastFrame * 10 * speedFactor;
	}
	if (m_pgame->isKeyDown(bbe::KEY_D))
	{
		m_cameraPos = m_cameraPos - bbe::Vector3(m_forward.rotate(bbe::Math::PI / 2, bbe::Vector3(0, 0, 1)).xy(), 0).normalize() * timeSinceLastFrame * 10 * speedFactor;
	}
	if (m_pgame->isKeyDown(bbe::KEY_SPACE))
	{
		m_cameraPos = m_cameraPos + bbe::Vector3(0, 0, 1) * timeSinceLastFrame * 10 * speedFactor;
	}
	if (m_pgame->isKeyDown(bbe::KEY_C))
	{
		m_cameraPos = m_cameraPos - bbe::Vector3(0, 0, 1) * timeSinceLastFrame * 10 * speedFactor;
	}
}

bbe::Vector3 bbe::CameraControlNoClip::getCameraPos()
{
	return m_cameraPos;
}

bbe::Vector3 bbe::CameraControlNoClip::getCameraTarget()
{
	return m_cameraPos + m_forward;
}
