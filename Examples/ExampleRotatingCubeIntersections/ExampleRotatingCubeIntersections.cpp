#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::PointLight light;
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::Cube c1;
	bbe::Cube c2;
	bbe::Random rand;

	void placeCubes()
	{
		const bbe::Vector3 p1 = rand.randomVector3(100, 100, 100).rotate(rand.randomFloat(bbe::Math::TAU), rand.randomVector3InUnitSphere());
		const bbe::Vector3 p2 = rand.randomVector3(200, 200, 200).rotate(rand.randomFloat(bbe::Math::TAU), rand.randomVector3InUnitSphere());

		const bbe::Vector3 s1 = rand.randomVector3(1, 1, 1);
		const bbe::Vector3 s2 = rand.randomVector3(1, 1, 1);

		const bbe::Vector3 r1 = rand.randomVector3InUnitSphere();
		const bbe::Vector3 r2 = rand.randomVector3InUnitSphere();

		float rad1 = rand.randomFloat(bbe::Math::TAU);
		float rad2 = rand.randomFloat(bbe::Math::TAU);

		c1.set(p1, s1, r1, rad1);
		c2.set(p2, s2, r2, rad2);

		const bbe::Vector3 moveVec = c2.approach(c1, p1 - p2);
		c2.set(p2 + moveVec, s2, r2, rad2);
		ccnc.setCameraPos(p1);
	}

	virtual void onStart() override
	{
		placeCubes();
		light.setLightColor(1, 1, 1);
		light.setLightStrength(1);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyPressed(bbe::Key::SPACE))
		{
			placeCubes();
		}
		ccnc.update(timeSinceLastFrame * 0.01f);
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		light.setPosition(ccnc.getCameraPos());
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());

		brush.fillCube(c1);
		brush.fillCube(c2);
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
	MyGame* mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Rotating Cubes!");

    return 0;
}
