#include "BBE/BrotBoxEngine.h"
#include <iostream>

class MyGame : public bbe::Game
{
public:
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);

	bbe::Random rand;

	bbe::Terrain terrain;

	bbe::PointLight sunLight;

	bool wireframe = false;

	MyGame()
		:terrain(8 * 1024, 8 * 1024, "../../Third-Party/textures/dryDirt.png", 12)
	{
		ccnc.setCameraPos(bbe::Vector3(1000, 1000, 500));

		terrain.setBaseTextureMult(bbe::Vector2(2, 2));
		terrain.setMaxHeight(350);
		float *weightsGrass = new float[terrain.getWidth() * terrain.getHeight()];
		float *weightsSand  = new float[terrain.getWidth() * terrain.getHeight()];
		for (int i = 0; i < terrain.getHeight(); i++)
		{
			for (int k = 0; k < terrain.getWidth(); k++)
			{
				int index = i * terrain.getWidth() + k;
				float heightValue = (float)terrain.projectOnTerrain(bbe::Vector3(k, i, 0)).z / 350;

				float weightSand = bbe::Math::normalDist(heightValue, 0, 0.3);
				float weightGras = bbe::Math::normalDist(heightValue, 0.5, 0.3);
				float weightStone = bbe::Math::normalDist(heightValue, 1, 0.3);
				float weightSum = weightSand + weightGras + weightStone;

				weightSand /= weightSum;
				weightGras /= weightSum;
				weightStone /= weightSum;

				weightsGrass[index] = weightGras;
				weightsSand[index] = weightSand;
			}
		}
		terrain.addTexture("../../Third-Party/textures/sand.png", weightsSand);
		terrain.addTexture("../../Third-Party/textures/cf_ter_gcs_01.png", weightsGrass);
		delete[] weightsGrass;
		delete[] weightsSand;
	}

	virtual void onStart() override
	{
		sunLight.setPosition(bbe::Vector3(10000, 20000, 40000));
		sunLight.setLightColor(bbe::Color(1, 1, 0.9f));
		sunLight.setLightStrength(0.9f);
		sunLight.setFalloffMode(bbe::LightFalloffMode::LIGHT_FALLOFF_NONE);

		terrain.setBaseTextureMult(bbe::Vector2(128, 128));
	}

	virtual void update(float timeSinceLastFrame) override
	{
		std::cout << "FPS: " << 1 / timeSinceLastFrame << "\n";
		ccnc.update(timeSinceLastFrame);

		if (isKeyPressed(bbe::Key::I))
		{
			wireframe = !wireframe;
		}
	}
	int height = 2;
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setFillMode(wireframe ? bbe::FillMode::WIREFRAME : bbe::FillMode::SOLID);

		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());

		brush.drawTerrain(terrain);
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
	std::cout << "Loading. Please wait. This might take a while." << std::endl;
	bbe::Settings::setAmountOfLightSources(5);
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "3D Test");
	delete mg;

    return 0;
}

