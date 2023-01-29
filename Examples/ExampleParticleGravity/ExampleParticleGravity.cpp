#include "BBE/BrotBoxEngine.h"

class Particle;

static bbe::Random s_random;
static bbe::List<Particle> particles;

static float size = 1;

class Particle
{
public:
	bbe::Vector3 pos;
	bbe::Vector3 speed;
	bbe::IcoSphere m_sphere;


	explicit Particle()
		: pos(s_random.randomVector3InUnitSphere() * 100.0f)
	{
	}

	Particle(const bbe::Vector3 &pos)
		: pos(pos)
	{
		//do nothing
	}

	void updateSpeed(float timeSinceLastFrame)
	{
		for (Particle &p : particles)
		{
			if (this == &p)
			{
				continue;
			}
			bbe::Vector3 dir = p.pos - pos;
			float length = dir.getLength();
			if (length != 0)
			{
				bbe::Vector3 gravity = dir.normalize() / length / length / 10;
				bbe::Vector3 antiTouch = dir.normalize() / length / length / length / 10;
				speed = speed + gravity - antiTouch;
			}
		}

		
		speed = speed * 0.99f;
	}

	void updatePos(float timeSinceLastFrame)
	{

		pos = pos + speed * timeSinceLastFrame * 0.2f;

		m_sphere.set(pos, bbe::Vector3(size), bbe::Vector3(1), 0);
	}

	void draw(bbe::PrimitiveBrush3D &brush)
	{
		brush.fillIcoSphere(m_sphere);
	}
};


class MyGame : public bbe::Game
{

	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::PointLight light;

	float maxSpeed = 0;
	float minSpeed = 0;
	bool wireframe = false;

	// Geerbt über Game
	virtual void onStart() override
	{
		std::cout << "hai onStart" << std::endl;
		light.falloffMode = bbe::LightFalloffMode::LIGHT_FALLOFF_NONE;
		light.lightStrength = 1;
		light.pos = bbe::Vector3(100, 100, 100);

		for (int i = 0; i < 200; i++)
		{
			particles.add(Particle());
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		ccnc.update(timeSinceLastFrame);
		std::cout << (1 / timeSinceLastFrame) << std::endl;

		if (isMousePressed(bbe::MouseButton::LEFT))
		{
			particles.add(Particle(ccnc.getCameraPos()));
		}

		timeSinceLastFrame = 0.016f;

		if (isKeyPressed(bbe::Key::T))
		{
			size += 0.1f;
		}

		if (isKeyPressed(bbe::Key::G))
		{
			size -= 0.1f;
		}

		if (isKeyPressed(bbe::Key::Z))
		{
			for (Particle &p : particles)
			{
				p.speed = bbe::Vector3();
			}
		}


		for (int iterations = 0; iterations < 20; iterations++)
		{
			for (Particle &p : particles)
			{
				p.updateSpeed(timeSinceLastFrame);
			}
			for (Particle &p : particles)
			{
				p.updatePos(timeSinceLastFrame);
			}
		}

		minSpeed = 100000.0f;
		maxSpeed = 0;
		for (std::size_t i = 0; i < particles.getLength(); i++)
		{
			float speed = particles[i].speed.getLength();
			if (speed > maxSpeed) maxSpeed = speed;
			if (speed < minSpeed) minSpeed = speed;
		}
		if (isKeyPressed(bbe::Key::I))
		{
			wireframe = !wireframe;
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.addLight(light);
		brush.setFillMode(wireframe ? bbe::FillMode::WIREFRAME : bbe::FillMode::SOLID);
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (Particle &p : particles)
		{
			float speed = p.speed.getLength();
			float percentage = (speed - minSpeed) / (maxSpeed + minSpeed);
			brush.setColor(1, 1 - percentage, 1 - percentage);
			p.draw(brush);
		}
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
	}
	virtual void onEnd() override
	{
	}
};

#include <iostream>
int main()
{
	std::cout << "hai main" << std::endl;
	MyGame game;
	game.start(1280, 720, "Particle Gravity");
    return 0;
}

