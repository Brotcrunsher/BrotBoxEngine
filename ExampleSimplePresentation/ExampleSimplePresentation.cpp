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


	virtual void onStart() override
	{
		// TODO Remove absolute paths
		//slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 039 - unique_ptr ist teurer als C Pointer/unique_ptr.txt");
		//slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 039 - unique_ptr ist teurer als C Pointer/c_ptr.txt");
		//slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 039 - unique_ptr ist teurer als C Pointer/unique_ptr.txt");
		//slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 039 - unique_ptr ist teurer als C Pointer/noexcept.txt");

		//slideShow.addSlide("D:/Videos/C++ Tutorial/Episode Bonus 039 - unique_ptr ist teurer als C Pointer/rvalue_asm.txt");


		//slideShow.forceFontSize(15);
		//for (size_t i = 0; i < slideShow.slides.getLength(); i++)
		//{
		//	slideShow.slides[i].compile();
		//}
		//slideShow.writeAsPowerPoint("D:/__Projekte/C++/Visual Studio Projekte/BrotboxEngine/ExampleSimplePresentation/out.xml");

		slideShow.addManifest("D:/Videos/C++ Tutorial/Episode Bonus 043 - Copy Elision/Manifest.txt");
	}

	virtual void update(float timeSinceLastFrame) override
	{
		PresentationControl pc = PresentationControl::none;
		const bool bigJump = isKeyDown(bbe::Key::LEFT_CONTROL) || isKeyDown(bbe::Key::RIGHT_CONTROL);

		     if (isKeyPressed(bbe::Key::LEFT))  pc = bigJump ? PresentationControl::previous_slide : PresentationControl::previous;
		else if (isKeyPressed(bbe::Key::RIGHT)) pc = bigJump ? PresentationControl::next_slide : PresentationControl::next;
		
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

