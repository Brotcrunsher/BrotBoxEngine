#include "BBE/BrotBoxEngine.h"

/*
* Quick and dirty rendering for one of my videos.
* You maybe don't want to have a too close look at this. ;-)
*/





































class MyGame : public bbe::Game
{
	bbe::List<bbe::List<float>> frequencies;
	bbe::List<bbe::List<float>> partialSums;
	bbe::Random rand;

	float t = 0;


	enum class mode
	{
		wait = 0,
		frequency_change = 1,
		volume_change = 2,
		offset_change = 3,
		adding_frequencies = 4,
	};
	mode currentMode = mode::wait;

	void addFrequency(float wavelength, float offset)
	{
		bbe::List<float> newWave;
		constexpr size_t samples = 44000;
		for (size_t i = 0; i < samples; i++)
		{
			newWave.add(bbe::Math::sin((float)i / wavelength + offset));
		}
		frequencies.add(newWave);

		if (partialSums.getLength() == 0)
		{
			partialSums.add(newWave);
		}
		else
		{
			bbe::List<float> newSum;
			for (size_t i = 0; i < samples; i++)
			{
				newSum.add(newWave[i] + partialSums[partialSums.getLength() - 1][i]);
			}
			partialSums.add(newSum);
		}
	}


	virtual void onStart() override
	{
		rand.setSeed(10);
		addFrequency(1000, 0);
		addFrequency(300, 0);
		addFrequency(10000, 0);
		for (int i = 0; i < 1000; i++)
		{
			if(i < 10)
				addFrequency(rand.randomFloat() * 7000 + 300, rand.randomFloat() * 1000);
			else
				addFrequency(rand.randomFloat() * 7000 + 1, rand.randomFloat() * 1000);
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (isKeyPressed(bbe::Key::SPACE))
		{
			currentMode = (mode)((int)currentMode + 1);
			t = 0;
		}
		if (currentMode == mode::adding_frequencies)
		{
			t += timeSinceLastFrame * 0.1f * (1 + t);
		}
		else
		{
			t += timeSinceLastFrame;
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		const bbe::Vector2 mouse = getMouseGlobal();

		ImGui::SetNextWindowSize({ (float)getScaledWindowWidth(), (float)getScaledWindowHeight() });
		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::Begin("Settings");

		const float target = (float)getScaledWindowHeight() * 40.f / 100.f;
		if (currentMode == mode::wait)
		{
			ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, target });
			ImGui::PlotLines("##p2", partialSums[ 0].getRaw(), partialSums[0].getLength(), 0, 0, FLT_MAX, FLT_MAX, ImVec2((float)getScaledWindowWidth() * 0.8f, (float)getScaledWindowHeight() / 3));
		}
		else if (currentMode == mode::frequency_change)
		{
			bbe::List<float> newWave;
			constexpr size_t samples = 44000;
			if (t > bbe::Math::TAU) t = bbe::Math::TAU;
			for (size_t i = 0; i < samples; i++)
			{
				newWave.add(bbe::Math::sin((float)i / 1000 * (bbe::Math::sin(t) / 2.f + 1.f)));
			}
			ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, target });
			ImGui::PlotLines("##p2", newWave.getRaw(), newWave.getLength(), 0, 0, FLT_MAX, FLT_MAX, ImVec2((float)getScaledWindowWidth() * 0.8f, (float)getScaledWindowHeight() / 3));
		}
		else if (currentMode == mode::volume_change)
		{
			bbe::List<float> newWave;
			constexpr size_t samples = 44000;
			if (t > bbe::Math::TAU) t = bbe::Math::TAU;
			for (size_t i = 0; i < samples; i++)
			{
				newWave.add(bbe::Math::sin((float)i / 1000) * (bbe::Math::cos(t) + 1) / 2);
			}
			ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, target });
			ImGui::PlotLines("##p2", newWave.getRaw(), newWave.getLength(), 0, 0, -1, +1, ImVec2((float)getScaledWindowWidth() * 0.8f, (float)getScaledWindowHeight() / 3));
		}
		else if (currentMode == mode::offset_change)
		{
			bbe::List<float> newWave;
			constexpr size_t samples = 44000;
			if (t > bbe::Math::TAU) t = bbe::Math::TAU;
			for (size_t i = 0; i < samples; i++)
			{
				newWave.add(bbe::Math::sin((float)i / 1000 + t * 3));
			}
			ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, target });
			ImGui::PlotLines("##p2", newWave.getRaw(), newWave.getLength(), 0, 0, -1, +1, ImVec2((float)getScaledWindowWidth() * 0.8f, (float)getScaledWindowHeight() / 3));
		}
		if (currentMode == mode::adding_frequencies)
		{
			size_t index = (size_t)t;
			if (index > partialSums.getLength() - 1) index = partialSums.getLength() - 1;

			ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, bbe::Math::interpolateLinear(-(float)getScaledWindowHeight() / 3, target, t - (int)t) });
			if (index < frequencies.getLength() - 2) ImGui::PlotLines("##p1", frequencies[index + 1].getRaw(), frequencies[index + 1].getLength(), 0, 0, -2.1f, +2.1f, ImVec2((float)getScaledWindowWidth() * 0.8f, (float)getScaledWindowHeight() / 3));
			ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, target });
			ImGui::PlotLines("##p2", partialSums[index + 0].getRaw(), partialSums[index + 0].getLength(), 0, 0, FLT_MAX, FLT_MAX, ImVec2((float)getScaledWindowWidth() * 0.8f, (float)getScaledWindowHeight() / 3));
		}

		ImGui::End();
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame game;
	game.start(1280, 720, "Example Wave Drawer");
	return 0;
}

