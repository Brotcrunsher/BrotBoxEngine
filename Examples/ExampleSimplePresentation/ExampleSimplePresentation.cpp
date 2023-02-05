#include "BBE/BrotBoxEngine.h"
#include "SimplePresentation.h"
#include <iostream>
#include <Windows.h>

constexpr int WINDOW_WIDTH = 1280 + 50;
constexpr int WINDOW_HEIGHT = 720 + 50;

class MyGame : public bbe::Game
{
public:
	SlideShow slideShow;
	PresentationControl previousPc = PresentationControl::none;
	float timeInThisPc = 0;
	const char* path;

	MyGame(const char* path)
		: path(path)
	{
	}

	virtual void onStart() override
	{
		// TODO Remove absolute paths
		
		if (path)
		{
			std::cout << "Loading Manifest: " << path << std::endl;
		}
		slideShow.addManifest(path ? path : "D:/Videos/C++ Tutorial/Episode Bonus 048 - OpenGL bis zum Dreieck/Manifest.txt");
	}

	virtual void update(float timeSinceLastFrame) override
	{
		PresentationControl pc = PresentationControl::none;
		const bool bigJump = isKeyDown(bbe::Key::LEFT_CONTROL) || isKeyDown(bbe::Key::RIGHT_CONTROL);

		     if (isKeyDown(bbe::Key::LEFT))  pc = bigJump ? PresentationControl::previous_slide : PresentationControl::previous;
		else if (isKeyDown(bbe::Key::RIGHT)) pc = bigJump ? PresentationControl::next_slide : PresentationControl::next;
		
		if(pc == previousPc && timeInThisPc < 0.5f)
		{
			pc = PresentationControl::none;
			timeInThisPc += timeSinceLastFrame;
		}
		else
		{
			if (pc == previousPc)
			{
				timeInThisPc = 0.5f - 0.05f;
			}
			else
			{
				timeInThisPc = 0;
			}
			previousPc = pc;
		}
		
		slideShow.update(pc, getMouseScrollY() * 10, timeSinceLastFrame);
	}

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		slideShow.draw(brush);
	}

	virtual void onEnd() override
	{
	}

	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
};


int main(int argc, const char** argv)
{
	MyGame* mg = new MyGame(argc >= 2 ? argv[1] : nullptr);
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Signed Distance Field Renderer");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}

