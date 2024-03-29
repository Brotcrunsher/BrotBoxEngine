#include "BBE/BrotBoxEngine.h"
#include "AssetStore.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
private:
	static inline bbe::Random random = bbe::Random();
	struct Balloon {
		static constexpr float WIDTH = 40;
		static constexpr float HEIGHT = 60;
		float x = random.randomFloat(WINDOW_WIDTH - Balloon::WIDTH);
		float y = WINDOW_HEIGHT;
		float hue = random.randomFloat(360.f);
	};


	bool gameover = false;
	int score = 0;

	bbe::List<Balloon> balloons;

	void addBalloon() {
		balloons.add(Balloon());
	}

	bool checkGameOver(const Balloon& b) {
		return (b.y <= 140);
	}

	bool checkMousePosition(const Balloon& b) {
		return (getMouseX() < b.x + Balloon::WIDTH  && getMouseX() > b.x)
		    && (getMouseY() < b.y + Balloon::HEIGHT && getMouseY() > b.y);
	}

public:
	virtual void onStart() override
	{
		addBalloon();
	}

	virtual void update(float timeSinceLastFrame) override
	{
		if (gameover) return;

		size_t amountOfBalloonsToAdd = 0;
		bool balloonClickedThisFrame = false;
		for (Balloon& b : balloons) {
			if (checkGameOver(b)) {
				gameover = true;
			}

			if (checkMousePosition(b) && isMousePressed(bbe::MouseButton::LEFT) && !balloonClickedThisFrame) {
				b = Balloon();
				score++;
				balloonClickedThisFrame = true;
				if (random.randomFloat()<0.2) {
					amountOfBalloonsToAdd++;
				}
			}

			b.y-=timeSinceLastFrame*60;
		}
		for (size_t i = 0; i < amountOfBalloonsToAdd; i++) {
			addBalloon();
		}
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}

	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(bbe::Rectangle(0, 0, WINDOW_WIDTH, 140));

		brush.setColorRGB(0, 0, 0);
		bbe::String currentScore = "Your Score: ";
		currentScore += score;
		brush.fillText(10, 120, currentScore.getRaw(), 100);

		brush.setColorRGB(0, 0, 0);
		brush.fillLine(0, 140, WINDOW_WIDTH, 140, 5);

		brush.setColorRGB(1, 1, 1);
		brush.drawImage(bbe::Rectangle(0, 140, WINDOW_WIDTH, assetStore::sky()->getHeight()), *assetStore::sky());

		for (Balloon& b : balloons) {
			brush.setColorHSV(b.hue, 1, 1);
			brush.fillCircle(b.x, b.y, Balloon::WIDTH, Balloon::HEIGHT);
		}
		
		if (gameover){
			brush.setColorRGB(1, 0, 0);
			brush.fillText(10, WINDOW_HEIGHT/2, "Game Over!", 100);

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
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
