// Example3D.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "BBE/BrotBoxEngine.h"
#include <iostream>

#define AMOUNTOFCUBES 1024 * 7
class MyGame : public bbe::Game
{
public:
	bbe::Cube cubes[AMOUNTOFCUBES];
	bbe::Vector3 originalPositions[AMOUNTOFCUBES];
	bbe::Vector3 positions[AMOUNTOFCUBES];
	bbe::Vector3 rotationAxis[AMOUNTOFCUBES];
	float rotationSpeeds[AMOUNTOFCUBES];
	float rotations[AMOUNTOFCUBES];
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::Random rand;

	float timePassed = 0.0f;

	bbe::Terrain terrain;
	bbe::PointLight light;

	bbe::PointLight blinkLight;

	MyGame()
		:light(bbe::Vector3(100, 200, 0))
	{

	}

	virtual void onStart() override
	{
		terrain.setTransform(bbe::Vector3(-750, -750, -120), bbe::Vector3(1), bbe::Vector3(1), 0);
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			originalPositions[i] = rand.randomVector3InUnitSphere();
			rotationAxis[i] = rand.randomVector3InUnitSphere();
			rotations[i] = rand.randomFloat() * bbe::Math::PI * 2;
			rotationSpeeds[i] = rand.randomFloat() * bbe::Math::PI * 2 * 0.25f;
			cubes[i].set(positions[i] , bbe::Vector3(1), rotationAxis[i], rotations[i]);
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		timePassed += timeSinceLastFrame;
		std::cout << 1 / timeSinceLastFrame << std::endl;
		ccnc.update(timeSinceLastFrame);


		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			positions[i] = originalPositions[i] * ((bbe::Math::sin(timePassed / 10.0f) + 1) * 40.0f + 10.0f);
			rotations[i] += rotationSpeeds[i] * timeSinceLastFrame;
			if (rotations[i] > bbe::Math::PI * 2)
			{
				rotations[i] -= bbe::Math::PI * 2;
			}
			cubes[i].set(positions[i], bbe::Vector3(1), rotationAxis[i], rotations[i]);
		}

		light.setPosition(bbe::Vector3(bbe::Math::sin(timePassed) * 1000, 0, 0));

		int intTimePassed = (int)timePassed;
		blinkLight.turnOn(intTimePassed % 2 == 0);
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			brush.fillCube(cubes[i]);
		}

		brush.drawTerrain(terrain);
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
	bbe::Settings::setAmountOfTransformContainers(8);
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "3D Test");

    return 0;
}

