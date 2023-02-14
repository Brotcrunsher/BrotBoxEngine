#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::Vector3 p1 = bbe::Vector3(0,  0, 0);
	bbe::Vector3 p2 = bbe::Vector3(0, -20, 0);

	virtual void onStart() override
	{
		ccnc.setCameraPos(-20, 0, 0);
		ccnc.setCameraForward(1, 0, 0);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		static float radians = 0;
		radians += timeSinceLastFrame;
		p2 = bbe::Vector3(0, -20, 0).rotate(radians, bbe::Vector3(0, 0, 1));

		ccnc.update(timeSinceLastFrame);

		if (isKeyDown(bbe::Key::Q)) p1 = ccnc.getCameraPos();
		if (isKeyDown(bbe::Key::E)) p2 = ccnc.getCameraPos();
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.addLight(bbe::PointLight(ccnc.getCameraPos()));
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());

		brush.fillIcoSphere(bbe::IcoSphere(p1));
		brush.fillIcoSphere(bbe::IcoSphere(p2));
		brush.fillLine(p1, p2);

		brush.setColor(1, 0, 0);
		brush.fillLine({ 0, 0, 0 }, { 4, 0, 0 });
		brush.setColor(0, 1, 0);
		brush.fillLine({ 0, 0, 0 }, { 0, 4, 0 });
		brush.setColor(0, 0, 1);
		brush.fillLine({ 0, 0, 0 }, { 0, 0, 4 });
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
	mg->start(1280, 720, "Example3DLineDraw!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

