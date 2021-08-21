#include "BBE/BrotBoxEngine.h"
#include "SimplePresentation.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280 + 50;
constexpr int WINDOW_HEIGHT = 720 + 50;

class MyGame : public bbe::Game
{
public:
	SlideShow slideShow;


	virtual void onStart() override
	{
		slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 038 - Coroutinen - Generatoren/intro.txt");
		slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 038 - Coroutinen - Generatoren/usage.txt");
		slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 038 - Coroutinen - Generatoren/generator.txt");
		slideShow.addType("Generator");
	}

	virtual void update(float timeSinceLastFrame) override
	{
		PresentationControl pc = PresentationControl::none;
		     if (isKeyPressed(bbe::Key::LEFT))  pc = PresentationControl::previous;
		else if (isKeyPressed(bbe::Key::RIGHT)) pc = PresentationControl::next;
			 slideShow.update(pc, getMouseScrollY() * 10);
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


int main()
{
	MyGame* mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Signed Distance Field Renderer");
	delete mg;
}

