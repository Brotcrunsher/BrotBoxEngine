// Example3D.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "BBE/BrotBoxEngine.h"
#include <iostream>

#define AMOUNTOFCUBES 1024
class MyGame : public bbe::Game
{
	bbe::Cube cubes[AMOUNTOFCUBES];
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::Random rand;

	virtual void onStart() override
	{
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			cubes[i].set(rand.randomVector3(1000.f), bbe::Vector3(1), bbe::Vector3(1), 0);
		}
		
	}
	virtual void update(float timeSinceLastFrame) override
	{
		ccnc.update(timeSinceLastFrame);
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			brush.fillCube(cubes[i]);
		}
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame mg;
	mg.start(1280, 720, "3D Test");

    return 0;
}

