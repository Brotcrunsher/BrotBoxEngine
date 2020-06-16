#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
private:
	struct Balloon {
		float x;
		float y;
		float width;
		float height;
		float hue;
	};

	static constexpr int BALLOON_WIDTH = 40;
	static constexpr int BALLOON_HEIGHT = 60;

	bool gameover = false;
	int score = 0;

	bbe::Font font;
	bbe::Random random;
	bbe::Image background;
	bbe::List<Balloon> balloons;

	void addBalloon() {
		balloons.add(
			Balloon { 
				random.randomFloat() * (WINDOW_WIDTH - BALLOON_WIDTH),
				WINDOW_HEIGHT,
				BALLOON_WIDTH, 
				BALLOON_HEIGHT, 
				randomBalloonHue() 
			}
		);
	}

	float randomBalloonHue() {
		return random.randomFloat() * 360;
	}

	bool checkGameOver(const Balloon& b) {
		return (b.y <= font.getFontSize() + 40);
	}

	bool checkMousePosition(const Balloon& b) {
		return (getMouseY() < b.y + b.height && getMouseY() > b.y)
			&& (getMouseX() < b.x + b.width  && getMouseX() > b.x);
	}

public:
	virtual void onStart() override
	{
		font.load("arial.ttf", 100);
		background.load(BBE_APPLICATION_ASSET_PATH "/sky.jpg");
		addBalloon();
	}

	virtual void update(float timeSinceLastFrame) override
	{
		if (gameover) return;

		const size_t amountOfBalloons = balloons.getLength();
		for (size_t i = 0; i < amountOfBalloons; i++) {
			Balloon& b = balloons[i];
			if (checkGameOver(b)) {
				gameover = true;
			}

			if (checkMousePosition(b) && isMousePressed(bbe::MouseButton::LEFT)) {
				b.y = WINDOW_HEIGHT;
				b.x = random.randomFloat(WINDOW_WIDTH - b.width);
				b.hue = randomBalloonHue();
				score++;

				if (random.randomFloat()<0.2) {
					addBalloon();
				}
			}
			b.y-=timeSinceLastFrame*60;
		}
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(bbe::Rectangle(0, 0, WINDOW_WIDTH, font.getFontSize() + 40));

		brush.setColorRGB(0, 0, 0);
		bbe::String currentScore = "Your Score: ";
		currentScore += score;
		brush.fillText(10, font.getFontSize() + 20, currentScore.getRaw(), font);

		brush.setColorRGB(0, 0, 0);
		brush.fillLine(0, font.getFontSize() + 40, WINDOW_WIDTH, font.getFontSize() + 40, 5);

		brush.setColorRGB(1, 1, 1);
		brush.drawImage(bbe::Rectangle(0, font.getFontSize() + 40, WINDOW_WIDTH, background.getHeight()), background);

		for (Balloon& b : balloons) {
			brush.setColorHSV(b.hue, 1, 1);
			brush.fillCircle(bbe::Circle(b.x, b.y, b.width, b.height));
		}
		
		if (gameover){
			brush.setColorRGB(1, 0, 0);
			brush.fillText(10, WINDOW_HEIGHT/2, "Game Over!", font);

		}
	}

	virtual void onEnd() override
	{
		background.destroy();
		font.destroy();
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Balloon Game");
	delete mg;
}
