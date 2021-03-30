#include "BBE/BrotBoxEngine.h"
#include <future>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// Inspired by CodeParade
// See: https://www.youtube.com/watch?v=Z_zmZ23grXE

class MyGame : public bbe::Game
{
	constexpr static size_t amountOfParticles = 500;
	constexpr static float particleSize = 3;
	int particleTypes = 3;
	float maxMax = 600;
	float minMin = 20;
	float maxAttraction;

	struct Particle
	{
		bbe::Vector2 pos;
		bbe::Vector2 speed;
		int particleType;
	};

	struct AttractionFunction
	{
		float min;
		float max;
		float half;
		float halfStrength;

		AttractionFunction(float min, float max, float halfStrength)
			: min(min), max(max), halfStrength(halfStrength), half((min + max) / 2.f)
		{

		}

		AttractionFunction(const AttractionFunction& other) = default;
		AttractionFunction(AttractionFunction&& other) = default;
		AttractionFunction& operator=(const AttractionFunction& other) = default;
		AttractionFunction& operator=(AttractionFunction&& other) = default;

		bbe::Vector2 operator() (bbe::Vector2 a, bbe::Vector2 b) const
		{
			const bbe::Vector2 to = b - a;
			const float dist = to.getLength();

			if (dist >= max) return bbe::Vector2(0, 0);

			const bbe::Vector2 norm = to.normalize();


			if (dist >= half)
			{
				const float t = (dist - half) / (max - half);
				return norm * (bbe::Math::interpolateLinear(halfStrength, 0, t));
			}
			else if (dist >= min)
			{
				const float t = (dist - min) / (half - min);
				return norm * bbe::Math::interpolateLinear(0, halfStrength, t);
			}
			else if (dist >= 0.1f)
			{
				const float t = 2.f * min * (1.f / (min + 2.f) - 1.f / (dist + 2.f));
				return norm * t * 10;
			}
			else
			{
				return bbe::Vector2(0, 0);
			}
		}
	};

	bbe::List<Particle> particles;
	bbe::Random rand;

	bbe::List<bbe::List<AttractionFunction>> attractionMatrix;

	bbe::List<bbe::Vector2> offsets;

	void placeRandomParticle()
	{
		particles.add(Particle{
			rand.randomVector2(WINDOW_WIDTH, WINDOW_HEIGHT),
			{0, 0},
			rand.randomInt(particleTypes)
			});
	}

	void generateRandomParticles()
	{
		particles.clear();
		for (int i = 0; i < amountOfParticles; i++)
		{
			placeRandomParticle();
		}
	}

	void generateRandomAttractionMatrix()
	{
		attractionMatrix.clear();
		for (size_t i = 0; i < particleTypes; i++)
		{
			attractionMatrix.add({});
			for (size_t k = 0; k < particleTypes; k++)
			{
				float attraction = rand.randomFloat() * maxAttraction * 2 - maxAttraction;
				if (i == k)
				{
					attraction = bbe::Math::abs(attraction);
				}
				attractionMatrix[i].add(AttractionFunction(
					rand.randomFloat(minMin),
					rand.randomFloat(maxMax),
					attraction
				));
			}
		}
	}

	void randomizeWorldSettings()
	{
		particleTypes = rand.randomFloat(10) + 2;
		minMin = rand.randomFloat(10) + 10;
		maxMax = rand.randomFloat(600) + minMin + 1;
		maxAttraction = rand.randomFloat() * 10;
	}

	void gameStart()
	{
		randomizeWorldSettings();
		generateRandomAttractionMatrix();
		generateRandomParticles();
	}

	virtual void onStart() override
	{
		offsets.add({ -WINDOW_WIDTH, -WINDOW_HEIGHT });
		offsets.add({ 0, -WINDOW_HEIGHT });
		offsets.add({ +WINDOW_WIDTH, -WINDOW_HEIGHT });
		offsets.add({ -WINDOW_WIDTH, 0 });
		offsets.add({ 0, 0 });
		offsets.add({ +WINDOW_WIDTH, 0 });
		offsets.add({ -WINDOW_WIDTH, +WINDOW_HEIGHT });
		offsets.add({ 0, +WINDOW_HEIGHT });
		offsets.add({ +WINDOW_WIDTH, +WINDOW_HEIGHT });
		gameStart();
	}

	void updateParticleSpeed(size_t index, size_t max)
	{
		for (size_t i = index; i < max && i < particles.getLength(); i++)
		{
			for (size_t k = 0; k < particles.getLength(); k++)
			{
				if (i == k) continue;

				for (size_t m = 0; m < offsets.getLength(); m++)
				{
					const bbe::Vector2 otherParticlePos = particles[k].pos + offsets[m];
					const bbe::Vector2& thisParticle = particles[i].pos;
					const bbe::Vector2 moveDir = otherParticlePos - thisParticle;
					const AttractionFunction& attractionFunction = attractionMatrix[particles[i].particleType][particles[k].particleType];

					particles[i].speed += attractionFunction(particles[i].pos, otherParticlePos) * 0.1f;
				}
			}
			particles[i].speed *= 0.9f;
		}
	}

	virtual void update(float timeSinceLastFrame) override
	{
		bbe::List<std::future<void>> futures;
		futures.resizeCapacity(particles.getLength());
		const size_t increment = amountOfParticles / 12;
		for (size_t i = 0; i < particles.getLength(); i += increment)
		{
			futures.add(std::async(std::launch::async, &MyGame::updateParticleSpeed, this, i, i + increment));
		}
		for (size_t i = 0; i < futures.getLength(); i++)
		{
			futures[i].wait();
		}

		for (size_t i = 0; i < particles.getLength(); i++)
		{
			particles[i].pos += particles[i].speed * 0.03;
		}

		for (size_t i = 0; i < particles.getLength(); i++)
		{
			if (particles[i].pos.x < 0)
			{
				particles[i].pos.x += WINDOW_WIDTH;
			}
			if (particles[i].pos.x > WINDOW_WIDTH)
			{
				particles[i].pos.x -= WINDOW_WIDTH;
			}
			if (particles[i].pos.y < 0)
			{
				particles[i].pos.y += WINDOW_HEIGHT;
			}
			if (particles[i].pos.y > WINDOW_HEIGHT)
			{
				particles[i].pos.y -= WINDOW_HEIGHT;
			}
		}

		if (isKeyPressed(bbe::Key::SPACE))
		{
			gameStart();
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		for (size_t i = 0; i < particles.getLength(); i++)
		{
			brush.setColorHSV(particles[i].particleType * 360 * bbe::Math::GOLDEN_RATIO, 1 - particles[i].particleType / 10, 1);
			brush.fillCircle(particles[i].pos.x - particleSize, particles[i].pos.y - particleSize, particleSize * 2, particleSize * 2);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame game;
	game.start(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle Life");
	return 0;
}

