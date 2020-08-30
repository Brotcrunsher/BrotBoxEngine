#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include "portaudio.h"
#include "minimp3_ex.h"

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	bbe::Sound mySound;
	bbe::Font myFont;
	bbe::SoundInstance latestSoundInstance;

	virtual void onStart() override
	{
		mySound.load(BBE_APPLICATION_ASSET_PATH "/TestSound.mp3");
		myFont.load("arial.ttf", 20);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isMousePressed(bbe::MouseButton::LEFT))
		{
			latestSoundInstance = mySound.play();
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.fillText(100, 100, "Click the mouse!", myFont);
		if (latestSoundInstance.isPlaying())
		{
			brush.fillText(100, 150, "Test! This will do.", myFont);
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
	delete mg;
}
