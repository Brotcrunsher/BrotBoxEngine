#include "BBE/BrotBoxEngine.h"
#include <iostream>

#define AMOUNTOFCUBES 1024 * 8
class MyGame : public bbe::Game
{
	bbe::Cube cubes[AMOUNTOFCUBES];
	bbe::Vector3 positions[AMOUNTOFCUBES];
	bbe::Vector3 rotationAxis[AMOUNTOFCUBES];
	float rotationSpeeds[AMOUNTOFCUBES];
	float rotations[AMOUNTOFCUBES];
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::Random rand;
	bbe::PointLight light;

	virtual void onStart() override
	{
		light.setPosition(bbe::Vector3(-1, -1, 1));
		light.setLightStrength(5);
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			positions[i] = rand.randomVector3InUnitSphere() * 100.0f;
			rotationAxis[i] = rand.randomVector3InUnitSphere();
			rotations[i] = rand.randomFloat() * bbe::Math::PI * 2;
			rotationSpeeds[i] = rand.randomFloat() * bbe::Math::PI * 2 * 0.25f;
			cubes[i].set(positions[i] , bbe::Vector3(1), rotationAxis[i], rotations[i]);
		}
		
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << 1 / timeSinceLastFrame << std::endl;
		ccnc.update(timeSinceLastFrame);
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			rotations[i] += rotationSpeeds[i] * timeSinceLastFrame;
			if (rotations[i] > bbe::Math::PI * 2)
			{
				rotations[i] -= bbe::Math::PI * 2;
			}
			cubes[i].set(positions[i], bbe::Vector3(1), rotationAxis[i], rotations[i]);
		}
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
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "3D Test");

    return 0;
}

