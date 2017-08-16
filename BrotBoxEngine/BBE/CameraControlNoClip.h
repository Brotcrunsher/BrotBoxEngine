#pragma once

#include "../BBE/Vector3.h"

namespace bbe
{
	class Game;

	class CameraControlNoClip
	{
	private:
		Game *m_pgame = nullptr;
		Vector3 m_cameraPos = bbe::Vector3(0.0f, 0.0f, 0.0f);
		Vector3 m_forward = bbe::Vector3(1.0f, 0.0f, 0.0f);
		float horizontalMouse = 0;
		float verticalMouse = 0;

	public:
		CameraControlNoClip(Game* game);

		void update(float timeSinceLastFrame);
		Vector3 getCameraPos();
		Vector3 getCameraTarget();
	};
}