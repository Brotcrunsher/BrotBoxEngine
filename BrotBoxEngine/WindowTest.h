#pragma once

#include "Game.h"
#include "Rectangle.h"
#include "PrimitiveBrush2D.h"

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
			virtual void onEnd() override;
		};

		void testWindow();
	}
}