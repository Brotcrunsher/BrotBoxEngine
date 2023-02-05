#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
	bbe::Vector2 vectorField[128][72];
	
	float gravX = 0;
	float gravY = 5;
	float distanceToMiddle = 20;
	float multConst = 1;

	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
		bbe::Vector2 gravity = bbe::Vector2(gravX, gravY);
		bbe::Vector2 middle = bbe::Vector2(128 / 2, 72 / 2);
		for (int i = 0; i < 128; i++)
		{
			for (int k = 0; k < 72; k++)
			{
				bbe::Vector2 point = bbe::Vector2(i, k);
				bbe::Vector2 fromMid = point - middle;
				float dist = fromMid.getLength();
				float mult = bbe::Math::sqrt(bbe::Math::abs(dist - distanceToMiddle)) * multConst;
				bbe::Vector2 fromMidNorm = fromMid.normalize();

				vectorField[i][k] = gravity + fromMidNorm * mult;
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		ImGui::SliderFloat("GravX", &gravX, -10, +10);
		ImGui::SliderFloat("GravY", &gravY, -10, +10);
		ImGui::SliderFloat("distanceToMiddle", &distanceToMiddle, -100, 100);
		ImGui::SliderFloat("multConst", &multConst, -20, +20);

		brush.setColorRGB(1, 1, 1, 0.2f);
		for (int i = 0; i < 128; i++)
		{
			for (int k = 0; k < 72; k++)
			{
				bbe::Vector2 pos = { i * 10.f + 5.f, k * 10.f + 5.f};
				brush.fillLine(pos, pos + vectorField[i][k]);
			}
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "VectorField!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

