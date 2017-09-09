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
	bbe::PointLight brightLight;
	bbe::PointLight sunLight;
	bbe::PointLight extraLight;
	
	bbe::Color colors[AMOUNTOFCUBES];

	bbe::Image image;
	bbe::Image image2;

	MyGame()
		:light(bbe::Vector3(100, 200, 0)), brightLight(bbe::Vector3(200, 200, 0)), terrain(1024 * 8, 1024 * 8, "../Third-Party/textures/dryDirt.png")
	{
	}

	virtual void onStart() override
	{
		sunLight.setPosition(bbe::Vector3(10000, 20000, 40000));
		sunLight.setLightColor(bbe::Color(1, 1, 0.9f));
		sunLight.setLightStrength(0.9f);
		sunLight.setFalloffMode(bbe::LightFalloffMode::LIGHT_FALLOFF_NONE);

		extraLight.setPosition(bbe::Vector3(-400, -400, 0));
		extraLight.setLightStrength(5000.f);
		extraLight.setFalloffMode(bbe::LightFalloffMode::LIGHT_FALLOFF_SQUARED);

		terrain.setTransform(bbe::Vector3(-750, -750, -120), bbe::Vector3(1), bbe::Vector3(1), 0);
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			originalPositions[i] = rand.randomVector3InUnitSphere();
			rotationAxis[i] = rand.randomVector3InUnitSphere();
			rotations[i] = rand.randomFloat() * bbe::Math::PI * 2;
			rotationSpeeds[i] = rand.randomFloat() * bbe::Math::PI * 2 * 0.25f;
			cubes[i].set(positions[i] , bbe::Vector3(1), rotationAxis[i], rotations[i]);
			colors[i] = bbe::Color(rand.randomFloat(), rand.randomFloat(), rand.randomFloat(), 1.0f);
		}

		image.load("images/TestImage.png");
		image2.load("images/TestImage2.png");
		
		terrain.setBaseTextureMult(bbe::Vector2(2, 2));
	}
	virtual void update(float timeSinceLastFrame) override
	{
		timePassed += timeSinceLastFrame;
		std::cout << "FPS: " << 1 / timeSinceLastFrame << std::endl;
		std::cout << std::endl;
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

		light.setPosition(bbe::Vector3(bbe::Math::sin(timePassed / 2) * 1000, 0, 0));

		int intTimePassed = (int)timePassed;
		blinkLight.turnOn(intTimePassed % 2 == 0);
		brightLight.setLightStrength(timePassed);
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			brush.setColor(colors[i]);
			brush.fillCube(cubes[i]);
		}

		brush.setColor(1, 1, 1);
		brush.drawTerrain(terrain);
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.drawImage(10, 10, 100, 100, image);
		brush.drawImage(120, 10, 100, 100, image2);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	bbe::Settings::setAmountOfLightSources(5);
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "3D Test");
	delete mg;

    return 0;
}

