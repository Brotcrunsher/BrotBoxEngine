// Example3D.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "BBE/BrotBoxEngine.h"
#include <iostream>

//#define AMOUNTOFCUBES 1024 * 7
#define AMOUNTOFCUBES 1024 * 4

#define AMOUNTOFFRAMES (1000 * 1)
#define AMOUNTOFBATCHES (1000)

class MyGame : public bbe::Game
{
public:
	float renderTimes[AMOUNTOFFRAMES];
	float cpuTimes[AMOUNTOFFRAMES];
	float frameTimes[AMOUNTOFFRAMES];

	float avg_renderTimes[AMOUNTOFBATCHES];
	float avg_cpuTimes[AMOUNTOFBATCHES];
	float avg_frameTimes[AMOUNTOFBATCHES];

	//bbe::Cube cubes[AMOUNTOFCUBES];
	//bbe::Vector3 originalPositions[AMOUNTOFCUBES];
	//bbe::Vector3 positions[AMOUNTOFCUBES];
	//bbe::Vector3 rotationAxis[AMOUNTOFCUBES];
	//float rotationSpeeds[AMOUNTOFCUBES];
	//float rotations[AMOUNTOFCUBES];
	//bbe::CameraControlNoClip ccnc = bbe::CameraControlNoClip(this);
	bbe::Random rand;

	bbe::Terrain terrain;
	//bbe::PointLight light;
	//bbe::PointLight blinkLight;
	//bbe::PointLight brightLight;
	bbe::PointLight sunLight;
	//bbe::PointLight extraLight;

	//bbe::Color colors[AMOUNTOFCUBES];

	//bbe::Image image;
	//bbe::Image image2;

	bool wireframe = false;

	int frameNumber = 0;
	int batch = 0;

	MyGame()
		:/*light(bbe::Vector3(100, 200, 0)), brightLight(bbe::Vector3(200, 200, 0)), */terrain(1024 * 8, 1024 * 8, "../Third-Party/textures/dryDirt.png", 12)
	{
		terrain.setBaseTextureMult(bbe::Vector2(2, 2));
		terrain.setMaxHeight(150);
		float *weightsGrass = new float[terrain.getWidth() * terrain.getHeight()];
		float *weightsSand  = new float[terrain.getWidth() * terrain.getHeight()];
		for (int i = 0; i < terrain.getHeight(); i++)
		{
			for (int k = 0; k < terrain.getWidth(); k++)
			{
				int index = i * terrain.getWidth() + k;
				weightsGrass[index] = (float)i / (float)terrain.getHeight();
				weightsSand[index]  = (float)k / (float)terrain.getWidth();

				float weightSum = weightsGrass[index] + weightsSand[index];
				if (weightSum > 1)
				{
					weightsGrass[index] /= weightSum;
					weightsSand[index] /= weightSum;
				}

				//weightsGrass[index] = 1;
				//weightsSand[index] = 0;
			}
		}
		terrain.addTexture("../Third-Party/textures/cf_ter_gcs_01.png", weightsGrass);
		//terrain.addTexture("../Third-Party/textures/dryDirt.png", weightsGrass); //TODO DELETE THIS LINE AND UNCOMMENT ABOVE!
		terrain.addTexture("../Third-Party/textures/sand.png", weightsSand);
		delete weightsGrass;
		delete weightsSand;
	}

	virtual void onStart() override
	{
		sunLight.setPosition(bbe::Vector3(10000, 20000, 40000));
		sunLight.setLightColor(bbe::Color(1, 1, 0.9f));
		sunLight.setLightStrength(0.9f);
		sunLight.setFalloffMode(bbe::LightFalloffMode::LIGHT_FALLOFF_NONE);

		//extraLight.setPosition(bbe::Vector3(-400, -400, 0));
		//extraLight.setLightStrength(5000.f);
		//extraLight.setFalloffMode(bbe::LightFalloffMode::LIGHT_FALLOFF_SQUARED);

		//terrain.setTransform(bbe::Vector3(-750, -750, -120), bbe::Vector3(1), bbe::Vector3(1), 0);
		/*for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			originalPositions[i] = rand.randomVector3InUnitSphere();
			rotationAxis[i] = rand.randomVector3InUnitSphere();
			rotations[i] = rand.randomFloat() * bbe::Math::PI * 2;
			rotationSpeeds[i] = rand.randomFloat() * bbe::Math::PI * 2 * 0.25f;
			cubes[i].set(positions[i] , bbe::Vector3(1), rotationAxis[i], rotations[i]);
			colors[i] = bbe::Color(rand.randomFloat(), rand.randomFloat(), rand.randomFloat(), 1.0f);
		}*/

		//image.load("images/TestImage.png");
		//image2.load("images/TestImage2.png");

		terrain.setBaseTextureMult(bbe::Vector2(128, 128));
	}

	virtual void update(float timeSinceLastFrame) override
	{
		//std::cout << "FPS: " << 1 / timeSinceLastFrame << "\n";
		//ccnc.update(timeSinceLastFrame);
		//std::cout << "Highest FPS: " << 1 / lowestDelta << "\n";
		//std::cout << "Lowest FPS:  " << 1 / highestDelta << "\n";
		//std::cout << std::endl;

		/*for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			positions[i] = originalPositions[i] * ((bbe::Math::sin(timePassed / 10.0f) + 1) * 40.0f + 10.0f);
			rotations[i] += rotationSpeeds[i] * timeSinceLastFrame;
			if (rotations[i] > bbe::Math::PI * 2)
			{
				rotations[i] -= bbe::Math::PI * 2;
			}
			cubes[i].set(positions[i], bbe::Vector3(1), rotationAxis[i], rotations[i]);
		}*/

		//light.setPosition(bbe::Vector3(bbe::Math::sin(timePassed / 2) * 1000, 0, 0));

		if (isKeyPressed(bbe::Key::I))
		{
			wireframe = !wireframe;
		}

		if (frameNumber < AMOUNTOFFRAMES)
		{
			frameTimes[frameNumber] = timeSinceLastFrame;
		}

	}
	int height = 2;
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
		brush.setFillMode(wireframe ? bbe::FillMode::WIREFRAME : bbe::FillMode::SOLID);

		//ccnc.setCameraPos(terrain.projectOnTerrain(ccnc.getCameraPos()));
		//brush.setCamera(ccnc.getCameraPos(), ccnc.getCameraTarget());

		//std::cout << ccnc.getCameraPos().x << "\t" << ccnc.getCameraPos().y << "\t" << ccnc.getCameraPos().z;

		float angle = (float)frameNumber / (float)AMOUNTOFFRAMES * bbe::Math::PI * 2;
		bbe::Vector3 center(2 * 1024, 2 * 1024, 0);
		bbe::Vector3 distTo(1 * 1024, 2 * 1024, 0);

		distTo = distTo.rotate(angle, bbe::Vector3(0, 0, 1), center);

		bbe::Vector3 pos = terrain.projectOnTerrain(distTo) + bbe::Vector3(0, 0, height);

		brush.setCamera(pos, center);

		/*for (int i = 0; i < AMOUNTOFCUBES; i++)
		{
			brush.setColor(colors[i]);
			brush.fillCube(cubes[i]);
		}*/

		/*for (int i = 0; i < 200; i++)
		{
			brush.setColor(bbe::Color(rand.randomFloat(), rand.randomFloat(), rand.randomFloat(), 1.0f));
			brush.fillCube(bbe::Cube(bbe::Vector3(0, 0, 100 - i), bbe::Vector3(1, 1, 1), bbe::Vector3(), 0));
		}*/

		//brush.setColor(1, 1, 1);

		brush.drawTerrain(terrain);
		
		//std::cout << bbe::Profiler::getRenderTime() << "\t\t\t" << bbe::Profiler::getCPUTime() << std::endl;

		if (frameNumber < AMOUNTOFFRAMES)
		{
			renderTimes[frameNumber] = bbe::Profiler::getRenderTime();
			cpuTimes[frameNumber] = bbe::Profiler::getCPUTime();
		}
		else if(frameNumber == AMOUNTOFFRAMES)
		{
			/*bbe::simpleFile::writeFloatArrToFile(bbe::String("_renderTimes") + height + ".txt", renderTimes, AMOUNTOFFRAMES);
			bbe::simpleFile::writeFloatArrToFile(bbe::String("_cpuTimes") + height + ".txt", cpuTimes, AMOUNTOFFRAMES);
			bbe::simpleFile::writeFloatArrToFile(bbe::String("_frameTimes") + height + ".txt", frameTimes, AMOUNTOFFRAMES);
			//std::exit(0);
			frameNumber = 0;
			height += 100;*/


			float avgRenderTime = 0;
			float avgCpuTime = 0;
			float avgFrameTime = 0;
			for (int i = 3; i < AMOUNTOFFRAMES; i++)
			{
				avgRenderTime += renderTimes[i];
				avgCpuTime    += cpuTimes[i];
				avgFrameTime  += frameTimes[i];
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
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__AVGrenderTimes") + height + ".txt", avg_renderTimes, AMOUNTOFBATCHES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__AVGcpuTimes") + height + ".txt", avg_cpuTimes, AMOUNTOFBATCHES);
				bbe::simpleFile::writeFloatArrToFile(bbe::String("__AVGframeTimes") + height + ".txt", avg_frameTimes, AMOUNTOFBATCHES);
				std::exit(0);
			}
			return;
		}

		frameNumber++;
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		//brush.drawImage(10, 10, 100, 100, image);
		//brush.drawImage(120, 10, 100, 100, image2);
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	bbe::Settings::setAmountOfLightSources(5);
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "3D Test");
	delete mg;

    return 0;
}

