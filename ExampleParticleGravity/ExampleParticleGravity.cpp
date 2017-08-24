// ExampleParticleGravity.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "BBE/BrotBoxEngine.h"

class Particle;

static bbe::Random random;
static bbe::List<Particle> particles;

static float size = 1;

class Particle
{
public:
	bbe::Vector3 pos;
	bbe::Vector3 speed;
	bbe::IcoSphere m_sphere;

	Particle()
	{
		pos = random.randomVector3InUnitSphere() * 100.0f;
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
				bbe::Vector3 gravity = dir.normalize() / length / length / 1000;
				bbe::Vector3 antiTouch = dir.normalize() / length / length / length / 1000;
				speed = speed + gravity - antiTouch;
			}
		}

		speed = speed * 0.999999f;
	}

	void updatePos(float timeSinceLastFrame)
	{

		pos = pos + speed * timeSinceLastFrame;

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


	// Geerbt über Game
	virtual void onStart() override
	{
		light.setFalloffMode(bbe::LightFalloffMode::LIGHT_FALLOFF_NONE);
		light.setLightStrength(1);
		light.setPosition(bbe::Vector3(100, 100, 100));
		for (int i = 0; i < 500; i++)
		{
			particles.add(Particle());
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		ccnc.update(timeSinceLastFrame);
		std::cout << (1 / timeSinceLastFrame) << std::endl;

		timeSinceLastFrame = 0.016f;

		if (isKeyPressed(bbe::Keys::T))
		{
			size += 0.1f;
		}

		if (isKeyPressed(bbe::Keys::G))
		{
			size -= 0.1f;
		}

		if (isKeyPressed(bbe::Keys::Z))
		{
			for (Particle &p : particles)
			{
				p.speed = bbe::Vector3();
			}
		}


		for (int iterations = 0; iterations < 2; iterations++)
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
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
		for (Particle &p : particles)
		{
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

int main()
{
	bbe::Settings::setAmountOfTransformContainers(8);
	MyGame game;
	game.start(1280, 720, "Particle Gravity");
    return 0;
}

