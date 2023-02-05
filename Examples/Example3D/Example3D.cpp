#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr size_t AMOUNT_OF_CUBES = 1024 * 2;
class MyGame : public bbe::Game
{
	struct CubeEntity
	{
		bbe::Cube cube;
		bbe::Vector3 rotationAxis;
		bbe::Color color;
		float rotationSpeed;
		float rotation;
	};
	CubeEntity cubeEntities[AMOUNT_OF_CUBES];
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::List<bbe::PointLight> lights;

	virtual void onStart() override
	{
		bbe::PointLight l(bbe::Vector3(-1, -1, 1));
		l.lightStrength = 10;
		lights.add(l);

		bbe::Random rand;
		for (int i = 0; i < AMOUNT_OF_CUBES; i++)
		{
			bbe::Vector3 position = rand.randomVector3InUnitSphere() * 50.0f;
			if (position.getLength() < 10)
			{
				i--;
				continue;
			}
			cubeEntities[i].rotationAxis = rand.randomVector3InUnitSphere();
			cubeEntities[i].rotation = rand.randomFloat() * bbe::Math::PI * 2;
			cubeEntities[i].rotationSpeed = rand.randomFloat() * bbe::Math::PI * 2 * 0.25f;
			cubeEntities[i].cube.set(position, bbe::Vector3(1), cubeEntities[i].rotationAxis, cubeEntities[i].rotation);
			cubeEntities[i].color = bbe::Color(rand.randomFloat(), rand.randomFloat(), rand.randomFloat());
		}
		
	}
	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "Lights: " << lights.getLength() << " FPS: " << (1.f / timeSinceLastFrame) << std::endl;
		ccnc.update(timeSinceLastFrame);
		for (int i = 0; i < AMOUNT_OF_CUBES; i++)
		{
			cubeEntities[i].rotation += cubeEntities[i].rotationSpeed * timeSinceLastFrame;
			if (cubeEntities[i].rotation > bbe::Math::PI * 2)
			{
				cubeEntities[i].rotation -= bbe::Math::PI * 2;
			}
			cubeEntities[i].cube.setRotation(cubeEntities[i].rotationAxis, cubeEntities[i].rotation);
		}

#ifndef BBE_RENDERER_VULKAN
		// The Vulkan Renderer currently only supports up to 5 lights, so we better not let the user add any.
		if (isKeyPressed(bbe::Key::E))
		{
			bbe::PointLight l(ccnc.getCameraPos());
			l.lightStrength = 10;
			lights.add(l);
		}
#endif
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		for (const bbe::PointLight& light : lights)
		{
			brush.addLight(light);
		}
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (int i = 0; i < AMOUNT_OF_CUBES; i++)
		{
			brush.setColor(cubeEntities[i].color);
			brush.fillCube(cubeEntities[i].cube);
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
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

