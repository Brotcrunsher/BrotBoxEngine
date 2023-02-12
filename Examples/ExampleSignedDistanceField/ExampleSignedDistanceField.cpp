#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
public:
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);

	bbe::Vector3 camPos;

	virtual void onStart() override
	{
		ccnc.setCameraPos(bbe::Vector3(0, 0, 0));
	}

	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << (1 / timeSinceLastFrame) << std::endl;
		ccnc.update(timeSinceLastFrame * 0.1);
		//ccnc.setCameraForward({ 1, 0, 0 });
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		bbe::Vector3 camPos = ccnc.getCameraPos();
		camPos.z *= -1;
#ifdef BBE_RENDERER_VULKAN
		assetStore::sdf()->setPushConstant(80, sizeof(float), &camPos.x);
		assetStore::sdf()->setPushConstant(84, sizeof(float), &camPos.z);
		assetStore::sdf()->setPushConstant(88, sizeof(float), &camPos.y);
#elif defined(BBE_RENDERER_OPENGL)
		// TODO: There is some foul play here. OpenGL has to switch y and z. Why? Left-/Right-Handedness?
		assetStore::sdf()->setUniform1f("camX", camPos.x);
		assetStore::sdf()->setUniform1f("camY", camPos.z);
		assetStore::sdf()->setUniform1f("camZ", camPos.y);
#endif

		bbe::Vector3 forward = ccnc.getCameraForward();
		forward.z *= -1;
#ifdef BBE_RENDERER_VULKAN
		assetStore::sdf()->setPushConstant( 92, sizeof(float), &forward.x);
		assetStore::sdf()->setPushConstant( 96, sizeof(float), &forward.z);
		assetStore::sdf()->setPushConstant(100, sizeof(float), &forward.y);
#elif defined(BBE_RENDERER_OPENGL)
		// TODO: There is some foul play here. OpenGL has to switch y and z. Why? Left-/Right-Handedness?
		assetStore::sdf()->setUniform1f("camForwardX", forward.x);
		assetStore::sdf()->setUniform1f("camForwardY", forward.z);
		assetStore::sdf()->setUniform1f("camForwardZ", forward.y);
#endif

		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, assetStore::sdf());
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
