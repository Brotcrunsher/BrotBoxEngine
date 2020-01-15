#include "BBE/BrotBoxEngine.h"
#include <iostream>

#define PERFORMANCETEST 0

#if PERFORMANCETEST
#define AMOUNTOFFRAMES (1000 * 10)
#define AMOUNTOFBATCHES (1000)
#endif

class MyGame : public bbe::Game
{
public:
#if PERFORMANCETEST
	float renderTimes[AMOUNTOFFRAMES * AMOUNTOFBATCHES];
	float cpuTimes[AMOUNTOFFRAMES * AMOUNTOFBATCHES];
	float frameTimes[AMOUNTOFFRAMES * AMOUNTOFBATCHES];

	float avg_renderTimes[AMOUNTOFBATCHES];
	float avg_cpuTimes[AMOUNTOFBATCHES];
	float avg_frameTimes[AMOUNTOFBATCHES];
#endif

#if !PERFORMANCETEST
	bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
#endif

	bbe::Random rand;

	bbe::TerrainSingle terrain;		//Terrain Single Draw Call (Tessellation)
	//bbe::Terrain terrain;			//Terrain Multi  Draw Call (Tessellation)
	//bbe::TerrainMesh terrain;		//Meshimplementation

	bbe::PointLight sunLight;

	bool wireframe = false;


#if PERFORMANCETEST
	int frameNumber = 0;
	int batch = 0;
#endif

	MyGame()
		:terrain(8 * 1024, 8 * 1024, "../Third-Party/textures/dryDirt.png", 12)
	{
#if !PERFORMANCETEST
		ccnc.setCameraPos(bbe::Vector3(1000, 1000, 500));
#endif

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
		terrain.addTexture("../Third-Party/textures/sand.png", weightsSand);
		terrain.addTexture("../Third-Party/textures/cf_ter_gcs_01.png", weightsGrass);
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
#if !PERFORMANCETEST
		std::cout << "FPS: " << 1 / timeSinceLastFrame << "\n";
		ccnc.update(timeSinceLastFrame);
#endif

		if (isKeyPressed(bbe::Key::I))
		{
			wireframe = !wireframe;
		}

#if PERFORMANCETEST
		if (frameNumber < AMOUNTOFFRAMES)
		{
			frameTimes[frameNumber + batch * AMOUNTOFFRAMES] = timeSinceLastFrame;
		}
#endif
	}
	int height = 2;
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setFillMode(wireframe ? bbe::FillMode::WIREFRAME : bbe::FillMode::SOLID);

#if !PERFORMANCETEST
		brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());
#endif

#if PERFORMANCETEST
		float angle = (float)frameNumber / (float)AMOUNTOFFRAMES * bbe::Math::PI * 2;
		bbe::Vector3 center(4 * 1024, 4 * 1024, 0);
		bbe::Vector3 distTo(2 * 1024, 4 * 1024, 0);

		distTo = distTo.rotate(angle, bbe::Vector3(0, 0, 1), center);

		bbe::Vector3 pos = terrain.projectOnTerrain(distTo) + bbe::Vector3(0, 0, height);

		brush.setCamera(pos, center);
#endif

		brush.drawTerrain(terrain);

#if PERFORMANCETEST
		if (frameNumber < AMOUNTOFFRAMES)
		{
			renderTimes[frameNumber + batch * AMOUNTOFFRAMES] = bbe::Profiler::getRenderTime();
			cpuTimes[frameNumber + batch * AMOUNTOFFRAMES] = bbe::Profiler::getCPUTime();
		}
		else if(frameNumber == AMOUNTOFFRAMES)
		{
			float avgRenderTime = 0;
			float avgCpuTime = 0;
			float avgFrameTime = 0;
			for (int i = 3; i < AMOUNTOFFRAMES; i++)
			{
				avgRenderTime += renderTimes[i + batch * AMOUNTOFFRAMES];
				avgCpuTime    += cpuTimes[i + batch * AMOUNTOFFRAMES];
				avgFrameTime  += frameTimes[i + batch * AMOUNTOFFRAMES];
			}
			avgRenderTime /= (AMOUNTOFFRAMES - 3);
			avgCpuTime    /= (AMOUNTOFFRAMES - 3);
			avgFrameTime  /= (AMOUNTOFFRAMES - 3);

			avg_renderTimes[batch] = avgRenderTime;
			avg_cpuTimes[batch]    = avgCpuTime;
			avg_frameTimes[batch]  = avgFrameTime;

			frameNumber = 0;
			std::cout << batch << "\n";
			batch++;
			height++;
			if (batch == AMOUNTOFBATCHES)
			{
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__REALrenderTimes") + height + ".txt", renderTimes, AMOUNTOFBATCHES * AMOUNTOFFRAMES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__REALcpuTimes") + height + ".txt", cpuTimes, AMOUNTOFBATCHES * AMOUNTOFFRAMES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__REALframeTimes") + height + ".txt", frameTimes, AMOUNTOFBATCHES * AMOUNTOFFRAMES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__AVGrenderTimes") + height + ".txt", avg_renderTimes, AMOUNTOFBATCHES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__AVGcpuTimes") + height + ".txt", avg_cpuTimes, AMOUNTOFBATCHES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__AVGframeTimes") + height + ".txt", avg_frameTimes, AMOUNTOFBATCHES);
				std::exit(0);
			}
			return;
		}

		frameNumber++;
#endif
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

