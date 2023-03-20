#pragma once

#include "../BBE/Vector3.h"

namespace bbe
{
	class Game;

	class CameraControlNoClip
	{
	private:
		Game   *m_pgame         = nullptr;
		Vector3 m_cameraPos     = bbe::Vector3(0.0f, 0.0f, 0.0f);
		Vector3 m_forward       = bbe::Vector3(1.0f, 0.0f, 0.0f);
		float   m_horizontalMouse = 0;
		float   m_verticalMouse   = 0;
		float   m_timeSinceShiftPress = 0;

		float   m_constraintZPos = 0.f;
		bool    m_isZPosConstrained = false;

	public:
		explicit CameraControlNoClip(Game* game);

		void update(float timeSinceLastFrame);
		Vector3 getCameraPos() const;
		Vector3 getCameraForward() const;
		Vector3 getCameraTarget() const;

		void setCameraPos(float x, float y, float z);
		void setCameraPos(const Vector3 &pos);
		void setCameraForward(float x, float y, float z);
		void setCameraForward(const Vector3& forward);

		void constraintZPos(float z);
	};
}