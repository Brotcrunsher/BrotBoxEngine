#include "BBE/BrotBoxEngine.h"
#include <iostream>
#include <vector>
#include <future>

class MyGame;
class MyGame : public bbe::Game
{
	inline static bbe::Random rand = bbe::Random();
	static float attractionForce(float distanceSq)
	{
		if (distanceSq <= 4.0f)
		{
			auto retVal = distanceSq * 0.5f - 1.0f;
			if (retVal < 0.0f) retVal *= 10.0f;
			return retVal;
		}
		const auto distanceHalfSq = distanceSq * 0.25;
		return 1.0f / distanceHalfSq;
	}

	static void solveAttraction(const bbe::Vector2& pos1, const bbe::Vector2& pos2, bbe::Vector2& speed, int32_t mass)
	{
		const auto distanceSq = pos1.getDistanceToSq(pos2) * 100.0f;
		const auto force = attractionForce(distanceSq);

		auto dir = pos1 - pos2;

		if (distanceSq <= 0)
		{
			dir = rand.randomVector2OnUnitSphere();
		}
		else
		{
			dir = dir.normalize();
		}

		dir *= force;
		dir *= 0.01f;

		speed -= dir * mass;
		//if (force < 0.0f)
		//{
		//	speed1 = speed1 * 0.9f + speed2 * 0.1f;
		//}
	}

	struct Particle
	{
		bbe::Vector2 pos;
		bbe::Vector2 speed;
	};

	struct GridCell
	{
		bbe::List<size_t> particleIds;
		bbe::Vector2 centerOfMass;
	};
	bool autoParticleManagement = true;
	int gridSize = 34;
	bbe::Grid<GridCell> particleGrid;
	bbe::List<Particle> particles;
	int numThreads = 23;
	float fps = 0.0;

	bbe::Vector2i posToGridIndex(const bbe::Vector2& pos) const
	{
		int32_t x = pos.x / (getWindowWidth() / particleGrid.getWidth());
		int32_t y = pos.y / (getWindowHeight() / particleGrid.getHeight());
		x = bbe::Math::clamp<int32_t>(x, 0, particleGrid.getWidth() - 1);
		y = bbe::Math::clamp<int32_t>(y, 0, particleGrid.getHeight() - 1);
		return { x, y };
	}

	void addParticle()
	{
		Particle newParticle;
		newParticle.pos = rand.randomVector2InUnitSphere() * bbe::Math::min(getWindowWidth(), getWindowHeight()) * 0.5f;
		newParticle.pos.x += getWindowWidth() * 0.5f;
		newParticle.pos.y += getWindowHeight() * 0.5f;
		const bbe::Vector2 toCenter = (bbe::Vector2(getWindowWidth() / 2, getWindowHeight() / 2) - newParticle.pos).normalize();
		newParticle.speed = toCenter.rotate90Clockwise() * 1.0f;
		particles.add(newParticle);
	}

	virtual void onStart() override
	{
		// High Score Debug:    8.500
		// High Score Release: 37.200
		for (int i = 0; i < 7000; i++)
		{
			addParticle();
		}
	}

	void updateGrid()
	{
		particleGrid = bbe::Grid<GridCell>(gridSize, gridSize);

		for (size_t i = 0; i < particles.getLength(); i++)
		{
			auto gridPos = posToGridIndex(particles[i].pos);
			particleGrid[gridPos].particleIds.add(i);
		}

		for (size_t i = 0; i < particleGrid.getWidth(); i++)
		{
			for (size_t k = 0; k < particleGrid.getHeight(); k++)
			{
				bbe::Vector2 centerOfMass;
				for (size_t m = 0; m < particleGrid[i][k].particleIds.getLength(); m++)
				{
					centerOfMass += particles[particleGrid[i][k].particleIds[m]].pos;
				}
				particleGrid[i][k].centerOfMass = centerOfMass / particleGrid[i][k].particleIds.getLength();
			}
		}
	}

	void updateParticle(size_t index)
	{
		Particle& p = particles[index];
		const auto gridPos = posToGridIndex(p.pos);
		for (int32_t offsetX = -2; offsetX <= 2; offsetX++)
		{
			for (int32_t offsetY = -2; offsetY <= 2; offsetY++)
			{
				const auto gridPos2 = gridPos + bbe::Vector2i(offsetX, offsetY);
				if (!particleGrid.isValidIndex(gridPos2)) continue;
				auto& cell = particleGrid[gridPos2];
				for (size_t i = 0; i < cell.particleIds.getLength(); i++)
				{
					auto otherIndex = cell.particleIds[i];
					if (index == otherIndex) continue;
					auto& otherParticle = particles[otherIndex];
					solveAttraction(p.pos, otherParticle.pos, p.speed, 1);
				}
			}
		}

		for (int32_t i = 0; i < (int32_t)particleGrid.getWidth(); i++)
		{
			for (int32_t k = 0; k < (int32_t)particleGrid.getHeight(); k++)
			{
				if (i >= gridPos.x - 2 && i <= gridPos.x - 2 && k >= gridPos.y - 2 && k <= gridPos.y - 2) continue;
				auto& cell = particleGrid[i][k];
				if (cell.particleIds.getLength() == 0) continue;
				solveAttraction(p.pos, cell.centerOfMass, p.speed, cell.particleIds.getLength());
			}
		}

#if 0
		for (size_t k = 0; k < particles.getLength(); k++)
		{
			if (index == k) continue;
			solveAttraction(p.pos, particles[k].pos, p.speed, particles[k].speed, 1);
		}
#endif
		p.speed *= 0.999f;
	}

	void updateSpeedsThread(size_t startIndex, size_t endIndex)
	{
		for (size_t i = startIndex; i < endIndex; i++)
		{
			updateParticle(i);
		}
	}

	void updateSpeeds()
	{
		std::vector<std::future<void>> futures;
		for (int i = 0; i < numThreads; i++)
		{
			futures.push_back(bbe::async(&MyGame::updateSpeedsThread, this, particles.getLength() / numThreads * i, particles.getLength() / numThreads * (i + 1)));
		}

		for (int i = 0; i < numThreads; i++)
		{
			futures[i].get();
		}
	}
	
	void updatePositions()
	{
		for (size_t i = 0; i < particles.getLength(); i++)
		{
			particles[i].pos += particles[i].speed * 0.1f;

			if (particles[i].pos.x < 0)
			{
				particles[i].pos.x *= -1;
				particles[i].speed.x *= -1;
			}
			if (particles[i].pos.y < 0)
			{
				particles[i].pos.y *= -1;
				particles[i].speed.y *= -1;
			}

			if (particles[i].pos.x >= getWindowWidth())
			{
				particles[i].pos.x = 2 * getWindowWidth() - particles[i].pos.x;
				particles[i].speed.x *= -1;
			}
			if (particles[i].pos.y >= getWindowHeight())
			{
				particles[i].pos.y = 2 * getWindowHeight() - particles[i].pos.y;
				particles[i].speed.y *= -1;
			}
		}
	}

	virtual void update(float timeSinceLastFrame) override
	{
		const float currentFps = 1.0f / timeSinceLastFrame;
		fps = 0.9f * fps + 0.1f * currentFps;
		beginMeasure("updateGrid");
		updateGrid();

		beginMeasure("updateSpeeds");
		updateSpeeds();

		beginMeasure("updatePositions");
		updatePositions();
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		beginMeasure("Render");
#if 0 // Attraction Plot
		bbe::List<float> a;
		bbe::List<float> b;

		for (int i = 0; i < 1000; i++)
		{
			a.add(i);
			b.add(attractionForce(i / 100.f));
		}

		if (ImPlot::BeginPlot("Plot", { -1, 250 })) {
			ImPlot::SetupAxes("Time", "Value");
			ImPlot::PlotLine("bla", a.getRaw(), b.getRaw(), a.getLength());
			ImPlot::EndPlot();
		}
#endif


		brush.setColorRGB(1, 1, 1, 0.3f);
		for (size_t i = 0; i < particles.getLength(); i++)
		{
			brush.fillRect(particles[i].pos, 1, 1);
		}
		static bool renderCenterOfMass = false;
		ImGui::Checkbox("renderCenterOfMass", &renderCenterOfMass);
		if (renderCenterOfMass)
		{
			brush.setColorRGB(0, 1, 0, 1);
			for (size_t i = 0; i < particleGrid.getWidth(); i++)
			{
				for (size_t k = 0; k < particleGrid.getHeight(); k++)
				{
					brush.fillRect(particleGrid[i][k].centerOfMass, 1, 1);
				}
			}
		}


		ImGui::Checkbox("autoParticleManagement", &autoParticleManagement);
		if (ImGui::Button("+100") || (autoParticleManagement && fps > 31))
		{
			for (size_t i = 0; i < 100; i++)
			{
				addParticle();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("-100") || (autoParticleManagement && fps < 29))
		{
			for (size_t i = 0; i < 100; i++)
			{
				particles.popBack();
			}
		}

		if (ImGui::Button("+1000"))
		{
			for (size_t i = 0; i < 1000; i++)
			{
				addParticle();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("-1000"))
		{
			for (size_t i = 0; i < 1000; i++)
			{
				particles.popBack();
			}
		}

		ImGui::InputInt("Num Threads", &numThreads);
		ImGui::InputInt("Grid Size", &gridSize);
		ImGui::Text("FPS: %f", fps);
		ImGui::Text("Particles: %d", (int)particles.getLength());
		drawMeasurement();
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "ExampleParticleGravity2D");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}