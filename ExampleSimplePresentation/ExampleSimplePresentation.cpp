#include "BBE/BrotBoxEngine.h"
#include "SimplePresentation.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
public:
	SimplePresentation sp{ SimplePresentation::SimplePresentationType::cpp };


	virtual void onStart() override
	{
//		sp.addText(
//"template<typename T>																	   \n"
//"struct Generator {					//blablabla bla bla bla								   \n"
//"    /* inline */ struct promise_type {													   \n"
//"        std::suspend_always initial_suspend() {										   \n"
//"            return {};																	   \n"
//"        }																				   \n"
//"        /* Multi-																		   \n"
//"           Line  */																	   \n"
//"        std::suspend_always final_suspend() noexcept {									   \n"
//"            return {};																	   \n"
//"        }																				   \n"
//"        Generator<T> get_return_object() {												   \n"
//"            return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) }; \n"
//"        }																				   \n"
//"																						   \n"
//"        std::suspend_always yield_value(T value) {										   \n"
//"            current_value = value;														   \n"
//"            current_value = \"random string\";											   \n"
//"            return {};																	   \n"
//"        }																				   \n"
//"        void return_void() {}															   \n"
//"        void unhandled_exception() {													   \n"
//"            std::exit(0123456789);														   \n"
//"        }																				   \n"
//"																						   \n"
//"        T current_value;																   \n"
//"    };																					   \n"
//		);

		sp.addText(bbe::simpleFile::readFile("D:/Videos/C++ Tutorial/Episode Bonus 038 - Coroutinen - Generatoren/usage.txt").getRaw());
		sp.addType("Generator");
	}

	virtual void update(float timeSinceLastFrame) override
	{
		SimplePresentation::PresentationControl pc = SimplePresentation::PresentationControl::none;
		     if (isKeyPressed(bbe::Key::LEFT))  pc = SimplePresentation::PresentationControl::previous;
		else if (isKeyPressed(bbe::Key::RIGHT)) pc = SimplePresentation::PresentationControl::next;
		sp.update(pc, getMouseScrollY() * 10);
	}

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		sp.draw(brush);
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

