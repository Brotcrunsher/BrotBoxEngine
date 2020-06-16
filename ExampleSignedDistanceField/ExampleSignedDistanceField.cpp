#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
public:
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::FragmentShader sdfShader;

	bbe::Vector3 camPos;

	virtual void onStart() override
	{
		sdfShader.load(BBE_APPLICATION_ASSET_PATH "/fragSdf.spv");
		ccnc.setCameraPos(bbe::Vector3(0, 0, 0));
	}

	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1 / timeSinceLastFrame) << std::endl;
		ccnc.update(timeSinceLastFrame * 0.1);

	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		bbe::Vector3 camPos = ccnc.getCameraPos();
		camPos.z *= -1;
		sdfShader.setPushConstant(brush, 80, sizeof(float), &camPos.x);
		sdfShader.setPushConstant(brush, 84, sizeof(float), &camPos.z);
		sdfShader.setPushConstant(brush, 88, sizeof(float), &camPos.y);

		bbe::Vector3 forward = ccnc.getCameraForward();
		forward.z *= -1;
		sdfShader.setPushConstant(brush,  92, sizeof(float), &forward.x);
		sdfShader.setPushConstant(brush,  96, sizeof(float), &forward.z);
		sdfShader.setPushConstant(brush, 100, sizeof(float), &forward.y);

		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, &sdfShader);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Signed Distance Field Renderer");
	delete mg;
}
