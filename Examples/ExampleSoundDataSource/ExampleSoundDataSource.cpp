#include "BBE/BrotBoxEngine.h"

class MySoundSource : public bbe::SoundDataSource
{
public:
	int amountOfFrequencies = 30;
	bbe::List<float> frequencies;
	bbe::List<float> volumeMult;
	bbe::List<float> offsets;
	int exponent = 1;
	bool fadeIn = false;
	bool autoAdjustMasterVolume = true;
	bool muted = false;
	mutable float masterVolume = 1;
	static constexpr uint32_t hz = 44000;


	virtual float getSample(size_t i, uint32_t channel) const override
	{
		if (muted)
		{
			return 0.f;
		}
		float fadeInMult = 1;
		if (fadeIn)
		{
			fadeInMult = i / (float)hz;
			if (fadeInMult > 1) fadeInMult = 1;
		}
		float val = 0;
		const size_t range = bbe::Math::min<size_t>(amountOfFrequencies, frequencies.getLength());
		for (size_t k = 0; k < range; k++)
		{
			float freq = bbe::Math::sin((float)i / (frequencies[k]) + offsets[k]) / (float)range * volumeMult[k];
			val += freq;
		}

		val *= fadeInMult;

		const bool wasNegative = val < 0;
		val = bbe::Math::pow(val, exponent);
		if (val > 0 && wasNegative) val *= -1;

		val *= masterVolume;

		const float absVal = bbe::Math::abs(val);
		if (absVal > 1)
		{
			masterVolume /= absVal;
		}

		return val;
	}
	virtual size_t getAmountOfSamples() const override
	{
		return (size_t)-1;
	}

	virtual uint32_t getHz() const override
	{
		return hz;
	}

	virtual uint32_t getAmountOfChannels() const override
	{
		return 1;
	}
};

class MyGame : public bbe::Game
{
	bbe::SoundInstance soundInstance;
	bbe::Random rand;
	MySoundSource mySound;
	bbe::List<float> shownVals;

	virtual void onStart() override
	{
		soundInstance = mySound.play();
		shownVals.resizeCapacityAndLength(44000 * 10);
	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		const bbe::Vector2 mouse = getMouseGlobal();

		ImGui::SetNextWindowSize({ (float)getScaledWindowWidth(), (float)getScaledWindowHeight() });
		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::Begin("Settings");
		
		if (ImGui::Button("Restart Sound"))
		{
			soundInstance.stop();
			soundInstance = mySound.play();
		}
		if (ImGui::Button("Randomize"))
		{
			MySoundSource newSound = mySound;
			for (int frequency = 0; frequency < mySound.amountOfFrequencies; frequency++)
			{
				newSound.frequencies[frequency] = rand.randomFloat() * 999 + 1;
				newSound.volumeMult[frequency] = rand.randomFloat() * 2;
				newSound.offsets[frequency] = rand.randomFloat() * 2 * bbe::Math::PI;
				newSound.exponent = rand.randomInt(3) + 1;
			}
			if (newSound.autoAdjustMasterVolume)
			{
				newSound.masterVolume = 100000;
			}
			for (size_t i = 0; i < shownVals.getLength(); i++)
			{
				// Adjust master volume.
				shownVals[i] = newSound.getSample(i, 0);
			}
			soundInstance.stop();
			mySound = newSound;
			soundInstance = mySound.play();
		}
		ImGui::Checkbox("Fade In", &mySound.fadeIn);
		ImGui::Checkbox("Auto Adjust Master Volume", &mySound.autoAdjustMasterVolume);
		ImGui::Checkbox("Muted", &mySound.muted);
		ImGui::InputInt("amountOfFrequencies", &mySound.amountOfFrequencies);
		ImGui::SliderInt("Exponent", &mySound.exponent, 0, 10);
		ImGui::SliderFloat("Master Volume", &mySound.masterVolume, 0, 100);
		while (mySound.frequencies.getLength() < mySound.amountOfFrequencies)
		{
			mySound.frequencies.add(10 * (1 + mySound.frequencies.getLength()));
			mySound.volumeMult.add(1);
			mySound.offsets.add(0);
		}
		for (int frequency = 0; frequency < mySound.amountOfFrequencies; frequency++)
		{
			bbe::String label = "##freq";
			label += frequency;
			ImGui::VSliderFloat(label.getRaw(), ImVec2((float)getScaledWindowWidth() / 150, (float)getScaledWindowWidth() / 25), &mySound.frequencies[frequency], 1, 1000, "", 0);
			ImGui::SameLine();
		}
		ImGui::Text("1 / Wavelength");

		ImGui::NewLine();
		for (int volume = 0; volume < mySound.amountOfFrequencies; volume++)
		{
			bbe::String label = "##vol";
			label += volume;
			ImGui::VSliderFloat(label.getRaw(), ImVec2((float)getScaledWindowWidth() / 150, (float)getScaledWindowWidth() / 25), &mySound.volumeMult[volume], 0, 2, "", 0);
			ImGui::SameLine();
		}
		ImGui::Text("Volume");

		ImGui::NewLine();
		for (int offset = 0; offset < mySound.amountOfFrequencies; offset++)
		{
			bbe::String label = "##offset";
			label += offset;
			ImGui::VSliderFloat(label.getRaw(), ImVec2((float)getScaledWindowWidth() / 150, (float)getScaledWindowWidth() / 25), &mySound.offsets[offset], 0, 2 * bbe::Math::PI, "", 0);
			ImGui::SameLine();
		}
		ImGui::Text("Offsets");

		ImGui::SetCursorPos({ (float)getScaledWindowWidth() / 10, (float)getScaledWindowWidth() / 100 });
		static size_t currentCalc = 0;
		for (size_t i = 0; i < 11000 && currentCalc < shownVals.getLength(); i++, currentCalc++)
		{
			shownVals[currentCalc] = mySound.getSample(currentCalc, 0);
		}
		if (currentCalc >= shownVals.getLength()) currentCalc = 0;
		ImGui::PlotLines("##p", shownVals.getRaw(), shownVals.getLength(), 0, 0, FLT_MAX, FLT_MAX, ImVec2((float)getScaledWindowWidth() / 5.f, (float)getScaledWindowWidth() / 25.f));

		ImGui::End();
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame game;
	game.start(1280, 720, "Sound Data Source");
    return 0;
}

