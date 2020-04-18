#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;


class MyGame : public bbe::Game
{

	class Balloon {

	public:
		int x;
		int y;
		int heigth;
		int width;
		float hue;

		Balloon(int x, int y, int width, int heigth, float hue) {
			this->x = x;
			this->y = y;

			this->width = width;
			this->heigth = heigth;
			this->hue = hue;
		}

	};


	int BALLOON_WIDTH = 40;
	int BALLOON_HEIGTH = 60;
	int SCORE = 0;
	bbe::Font myFont;
	bbe::Random random;
	bool gameover = false;
	bbe::Image background;

	bbe::List<Balloon> balloons;

	virtual void onStart() override
	{
		myFont.load("arial.ttf", 200);
		background.load("sky.jpg");
		addBalloon();

	}

	void addBalloon() {
		balloons.add(Balloon(random.randomInt(WINDOW_WIDTH - BALLOON_WIDTH), WINDOW_HEIGHT,
			 BALLOON_WIDTH, BALLOON_HEIGTH, randomBalloonHue()));
	}

	bbe::Color randomBalloonColor() {
		return bbe::Color(random.randomFloat(), random.randomFloat(), 0);
	}

	float randomBalloonHue() {
		return random.randomFloat()*360;
	}

	virtual void update(float timeSinceLastFrame) override
	{

		if (gameover) return;

		for (Balloon& b : balloons) {
			
			if (checkGameOver(b)) {
				gameover = true;
			}
			// check game over
			if (checkMousePosition(b) && isMousePressed(bbe::MouseButton::LEFT)) {
				b.y = WINDOW_HEIGHT;
				b.x = random.randomInt(WINDOW_WIDTH - b.width);
				b.hue = randomBalloonHue();
				SCORE++;

				if (random.randomFloat()<0.2) {
					addBalloon();
				}
			}
			b.y--;
		}
	}

	bool checkGameOver(const Balloon& b) {
		return  (b.y <= myFont.getFontSize() + 40);
	}

	bool checkMousePosition(const Balloon &b) {
		return (getMouseY() < b.y + b.heigth  && getMouseY() > b.y )
			&& (getMouseX() < b.x + b.width && getMouseX() > b.x);
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		
		brush.setColorRGB(bbe::Color(1, 1, 1));
		brush.fillRect(bbe::Rectangle(0, 0, WINDOW_WIDTH, myFont.getFontSize() + 40));

		brush.setColorRGB(bbe::Color(0, 0, 0));
		bbe::String currentScore = "Your Score: ";
		currentScore += SCORE;
		brush.fillText(10, myFont.getFontSize() + 20, currentScore.getRaw(), myFont);

		brush.setColorRGB(bbe::Color(0, 0, 0));
		brush.fillLine(0, myFont.getFontSize() + 40, WINDOW_WIDTH, myFont.getFontSize() + 40, 5);


		brush.drawImage(bbe::Rectangle(0, myFont.getFontSize() + 40, WINDOW_WIDTH, WINDOW_HEIGHT), background);

		// for each balloon draw
		for (Balloon& b : balloons) {
			brush.setColorHSV(b.hue, 1, 1);
			brush.fillCircle(bbe::Circle(b.x, b.y, b.width, b.heigth));
		}
		
		if (gameover){
			brush.setColorRGB(bbe::Color(1, 0, 0));
			brush.fillText(10, WINDOW_HEIGHT/2, "Game Over!", myFont);

		}
	}

	virtual void onEnd() override
	{
	}


	
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Balloon Game");
	delete mg;
}
