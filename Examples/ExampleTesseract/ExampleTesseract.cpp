#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class MyGame : public bbe::Game
{
	struct Barricade
	{
		bbe::Rectangle shape;
	};

	struct Bullet
	{
		constexpr static float TIME_BETWEEN_SHOTS = 0.1f;
		constexpr static float SIZE = 10;
		constexpr static float BASE_SPEED = 500;
		bbe::Rectangle shape;
		bbe::Vector2 speed;
	};

	void addBulletWithSpeed(const bbe::Vector2& speed)
	{
		bullets.add({ bbe::Rectangle(player.shape.getPos() + (bbe::Vector2(Player::SIZE, Player::SIZE) * 0.5) - (bbe::Vector2(Bullet::SIZE, Bullet::SIZE) * 0.5), bbe::Vector2(Bullet::SIZE, Bullet::SIZE)), speed + player.speed });
	}

	bool collidesWithAnyBarricade(const bbe::Rectangle& shape)
	{
		for (const Barricade& b : barricades)
		{
			if (b.shape.intersects(shape)) return true;
		}
		return false;
	}

	void moveShape(bbe::Rectangle &shape, const bbe::Vector2 &speed, float timeSinceLastFrame)
	{
		shape.x = bbe::Math::clamp(shape.x + speed.x * timeSinceLastFrame, 0.f, WINDOW_WIDTH - shape.width);
		for (const Barricade& b : barricades)
		{
			if (b.shape.intersects(shape))
			{
				if (speed.x > 0)
				{
					shape.x = b.shape.x - shape.width;
				}
				else
				{
					shape.x = b.shape.x + b.shape.width;
				}
			}
		}
		shape.y = bbe::Math::clamp(shape.y + speed.y * timeSinceLastFrame, 0.f, WINDOW_HEIGHT - shape.height);
		for (const Barricade& b : barricades)
		{
			if (b.shape.intersects(shape))
			{
				if (speed.y > 0)
				{
					shape.y = b.shape.y - shape.height;
				}
				else
				{
					shape.y = b.shape.y + b.shape.height;
				}
			}
		}
	}

	struct Player
	{
		constexpr static float ACCELERATION = 250;
		constexpr static float SIZE = 75;
		bbe::Rectangle shape = { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, SIZE, SIZE };
		bbe::Vector2 speed;
		float timeSinceLastShot = 0;
		bool dead = false;
		uint32_t score = 0;

		void calculateSpeed(MyGame* context)
		{
			if (dead) return;

			speed = {};
			if (context->isKeyDown(bbe::Key::W))
			{
				speed.y -= 1;
			}
			if (context->isKeyDown(bbe::Key::S))
			{
				speed.y += 1;
			}
			if (context->isKeyDown(bbe::Key::A))
			{
				speed.x -= 1;
			}
			if (context->isKeyDown(bbe::Key::D))
			{
				speed.x += 1;
			}
			if (speed.getLength() > 0)
			{
				speed = speed.normalize();
			}
			speed *= ACCELERATION;
		}

		void move(float timeSinceLastFrame, MyGame* context)
		{
			if (dead) return;
			context->moveShape(shape, speed, timeSinceLastFrame);
		}
		
		void checkBulletSpawn(float timeSinceLastFrame, MyGame* context)
		{
			if (dead) return;

			timeSinceLastShot += timeSinceLastFrame;
			if (context->isKeyDown(bbe::Key::UP) && timeSinceLastShot >= Bullet::TIME_BETWEEN_SHOTS)
			{
				timeSinceLastShot = 0;
				context->addBulletWithSpeed(bbe::Vector2(0, -Bullet::BASE_SPEED));
			}
			if (context->isKeyDown(bbe::Key::DOWN) && timeSinceLastShot >= Bullet::TIME_BETWEEN_SHOTS)
			{
				timeSinceLastShot = 0;
				context->addBulletWithSpeed(bbe::Vector2(0, +Bullet::BASE_SPEED));
			}
			if (context->isKeyDown(bbe::Key::LEFT) && timeSinceLastShot >= Bullet::TIME_BETWEEN_SHOTS)
			{
				timeSinceLastShot = 0;
				context->addBulletWithSpeed(bbe::Vector2(-Bullet::BASE_SPEED, 0));
			}
			if (context->isKeyDown(bbe::Key::RIGHT) && timeSinceLastShot >= Bullet::TIME_BETWEEN_SHOTS)
			{
				timeSinceLastShot = 0;
				context->addBulletWithSpeed(bbe::Vector2(+Bullet::BASE_SPEED, 0));
			}
		}

		void kill()
		{
			dead = true;
		}
	};

	struct Enemy
	{
		constexpr static float SPEED = 100;
		constexpr static float GET_AWAY_SPEED = 10;
		bbe::Rectangle shape;

		constexpr static float DEATH_TIME = 2;
		float timeSinceLastHit = 0;

		Enemy(const bbe::Rectangle& shape)
		{
			this->shape = shape;
		}

		void update(float timeSinceLastFrame, const bbe::Vector2& targetPos, MyGame* context)
		{
			if (timeSinceLastFrame > 1) return;


			timeSinceLastHit += timeSinceLastFrame;
			if (isDead()) return;

			bbe::Vector2 moveDir = (targetPos - shape.getPos()).normalize() * SPEED;
			for (size_t i = 0; i < context->enemies.getLength(); i++)
			{
				if (&context->enemies[i] == this || context->enemies[i].isDead()) continue;
				bbe::Vector2 getAwayDir = (shape.getPos() - context->enemies[i].shape.getPos());
				if (getAwayDir.getLength() < 20)
				{
					moveDir += getAwayDir.normalize() * SPEED;
				}
			}
			context->moveShape(shape, moveDir, timeSinceLastFrame);

			for (size_t i = 0; i < context->bullets.getLength(); i++)
			{
				if (shape.intersects(context->bullets[i].shape))
				{
					timeSinceLastHit = 0;
					context->bullets.removeIndex(i);
					i--;
					context->player.score++;
				}
			}

			if (context->player.shape.intersects(shape))
			{
				context->player.kill();
			}
		}

		bbe::Color getColor() const
		{
			if (timeSinceLastHit >= DEATH_TIME) return bbe::Color(0, 1, 0);

			float percentage = timeSinceLastHit / DEATH_TIME;
			return bbe::Color(0, 0.1f + percentage * 0.9f, 0);
		}

		bool isDead() const
		{
			return timeSinceLastHit < DEATH_TIME;
		}
	};

	struct Tesseract
	{
		constexpr static float TIME_BETWEEN_REPROSITIONINGS = 3;
		constexpr static float SIZE = 75;
		bbe::Random random;
		bbe::Rectangle shape{ 0, 0, SIZE, SIZE };
		float timeSinceLastNewPosition = 0;
		float timeAlive = 0;

		void update(float timeSinceLastFrame, MyGame* context)
		{
			timeAlive += timeSinceLastFrame;
			if (context->player.dead) return;
			timeSinceLastNewPosition += timeSinceLastFrame;

			if (timeSinceLastNewPosition >= TIME_BETWEEN_REPROSITIONINGS)
			{
				context->enemies.add(Enemy{ this->shape });
				timeSinceLastNewPosition = 0;
				newRandomPosition(context);
			}
		}

		void newRandomPosition(MyGame* context)
		{
			while (true)
			{
				shape.x = random.randomFloat(WINDOW_WIDTH - SIZE);
				shape.y = random.randomFloat(WINDOW_HEIGHT - SIZE);
				if (!context->collidesWithAnyBarricade(shape)) return;
			}
		}
	};

	Player player;
	Tesseract tesseract;
	bbe::List<Bullet> bullets;
	bbe::List<Barricade> barricades;
	bbe::List<Enemy> enemies;
	const bbe::Rectangle arena{ 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };

	void moveBullets(float timeSinceLastFrame)
	{
		for (Bullet& b : bullets)
		{
			b.shape.x = b.shape.x + b.speed.x * timeSinceLastFrame;
			b.shape.y = b.shape.y + b.speed.y * timeSinceLastFrame;
		}
	}

	void removeBulletsThatCollideWithBarricadesOrExitArena()
	{
		bullets.removeAll([this](const Bullet& s) {
			return !s.shape.intersects(arena);
		});
		bullets.removeAll([this](const Bullet& s) {
			return collidesWithAnyBarricade(s.shape);
		});
	}



	virtual void onStart() override
	{
		barricades.add({bbe::Rectangle(350, 300, 100, 200)});
		tesseract.newRandomPosition(this);
	}

	virtual void update(float timeSinceLastFrame) override
	{
		player.calculateSpeed(this);
		player.move(timeSinceLastFrame, this);

		moveBullets(timeSinceLastFrame);
		removeBulletsThatCollideWithBarricadesOrExitArena();

		player.checkBulletSpawn(timeSinceLastFrame, this);
		tesseract.update(timeSinceLastFrame, this);

		for (Enemy& e : enemies)
		{
			e.update(timeSinceLastFrame, player.shape.getPos(), this);
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(1, 0, 0);
		bbe::String scoreText = "Score: ";
		scoreText += player.score;
		brush.fillText(10, 40, scoreText.getRaw(), 50);

		brush.setColorRGB(0, 0, 1);
		for (const Barricade& b : barricades)
		{
			brush.fillRect(b.shape);
		}

		brush.setColorRGB(1, 1, 1);
		const bbe::Vector2 outerTopLeft     = tesseract.shape.getPos();
		const bbe::Vector2 outerTopRight    = tesseract.shape.getPos() + bbe::Vector2(tesseract.shape.width, 0);
		const bbe::Vector2 outerBottomLeft  = tesseract.shape.getPos() + bbe::Vector2(0, tesseract.shape.height);
		const bbe::Vector2 outerBottomRight = tesseract.shape.getPos() + tesseract.shape.getDim();
		brush.fillLine(outerTopLeft,     outerTopRight);
		brush.fillLine(outerTopLeft,     outerBottomLeft);
		brush.fillLine(outerBottomRight, outerTopRight);
		brush.fillLine(outerBottomRight, outerBottomLeft);
		const bbe::Vector2 middle = tesseract.shape.getPos() + tesseract.shape.getDim() * 0.5f;
		const float t = (bbe::Math::cos(tesseract.timeAlive * 10) + 1) * 0.6f;
		const bbe::Vector2 innerTopLeft     = bbe::Math::interpolateLinear(middle, outerTopLeft    , t);
		const bbe::Vector2 innerTopRight    = bbe::Math::interpolateLinear(middle, outerTopRight   , t);
		const bbe::Vector2 innerBottomLeft  = bbe::Math::interpolateLinear(middle, outerBottomLeft , t);
		const bbe::Vector2 innerBottomRight = bbe::Math::interpolateLinear(middle, outerBottomRight, t);
		brush.fillLine(innerTopLeft,     innerTopRight);
		brush.fillLine(innerTopLeft,     innerBottomLeft);
		brush.fillLine(innerBottomRight, innerTopRight);
		brush.fillLine(innerBottomRight, innerBottomLeft);
		brush.fillLine(outerTopLeft, innerTopLeft);
		brush.fillLine(outerTopRight, innerTopRight);
		brush.fillLine(outerBottomLeft, innerBottomLeft);
		brush.fillLine(outerBottomRight, innerBottomRight);

		for (const Enemy& e : enemies)
		{
			brush.setColorRGB(e.getColor());
			brush.fillRect(e.shape);
		}

		brush.setColorRGB(1, 1, 0);
		for (const Bullet& b : bullets)
		{
			brush.fillRect(b.shape);
		}

		brush.setColorRGB(1, 0, 0);
		brush.fillRect(player.shape);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Tesseract");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
