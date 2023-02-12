#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <iostream>
#include "portaudio.h"
#include "minimp3_ex.h"

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::SoundInstance latestSoundInstance;

	virtual void onStart() override
	{
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isMousePressed(bbe::MouseButton::LEFT))
		{
			latestSoundInstance = assetStore::TestSound()->play();
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.fillText(100, 100, "Click the mouse!", 20);
		if (latestSoundInstance.isPlaying())
		{
			brush.fillText(100, 150, "Test! This will do.", 20);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame* mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "ExampleSound!");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
