#pragma once

#include "../BBE/Game.h"
#include "../BBE/Rectangle.h"
#include "../BBE/PrimitiveBrush2D.h"

namespace bbe
{
	namespace test
	{
		class MyGame : public Game
		{
			Rectangle rect;

			virtual void onStart() override;
			virtual void update(float timeSinceLastFrame) override;
			virtual void draw2D(PrimitiveBrush2D &brush) override;
			virtual void draw3D(PrimitiveBrush3D &brush) override;
			virtual void onEnd() override;
		};

		void testWindow();
	}
}