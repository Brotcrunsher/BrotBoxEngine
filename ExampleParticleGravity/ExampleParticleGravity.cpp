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
				bbe::Vector3 gravity = dir.normalize() / length / length / 10;
				bbe::Vector3 antiTouch = dir.normalize() / length / length / length / 10;
				speed = speed + gravity - antiTouch;
			}
		}

		speed = speed * 0.9f;
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

		if (isKeyPressed(bbe::KEY_T))
		{
			size += 0.1;
		}

		if (isKeyPressed(bbe::KEY_G))
		{
			size -= 0.1;
		}

		if (isKeyPressed(bbe::KEY_Z))
		{
			for (Particle &p : particles)
			{
				p.speed = bbe::Vector3();
			}
		}

		for (Particle &p : particles)
		{
			p.updateSpeed(timeSinceLastFrame);
		}
		for (Particle &p : particles)
		{
			p.updatePos(timeSinceLastFrame);
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
	game.start(1280, 720, "3D Test");
    return 0;
}

