#include "BBE/BrotBoxEngine.h"

/*
* An example that utilizes both the bbe::SoundDataSource and the bbe::PixelObserver.
* One is able to enter any arbitrary pixel position into the provided gui. Once the
* given pixel changes the color, a sound is played with the bbe::SoundDataSource.
*/


class MySoundSource : public bbe::SoundDataSource
{
public:
	virtual float getSample(size_t i, uint32_t channel) const override
	{
		const size_t access_i = i % 1000000;
		constexpr size_t amountOfFrequencies = 30;
		float val = 0;
		for (size_t k = 0; k < amountOfFrequencies; k++)
		{
			float volume = (float)i / 44000.f / 1.f / (k + 1);
			if (volume > 1) volume = 1;
			val += bbe::Math::sin((float)access_i / ((k + 10.f) * 1.f)) * volume / (float)amountOfFrequencies;
		}

		val *= 2;

		return val;
	}
	virtual size_t getAmountOfSamples() const override
	{
		return (size_t)-1;
	}

	virtual uint32_t getHz() const override
	{
		return 44000;
	}

	virtual uint32_t getAmountOfChannels() const override
	{
		return 1;
	}
};

class MyGame : public bbe::Game
{
	bool armed = false;
	bool armedLastFrame = false;
	bbe::Color protectionColor;
	bbe::SoundInstance soundInstance;
	bbe::PixelObserver pixelObserver;

	MySoundSource mySound;

	virtual void onStart() override
	{
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
		ImGui::Text("Mouse Pos: %d / %d", (int)mouse.x, (int)mouse.y);
		auto mouseColor = pixelObserver.getColor(mouse);
		ImGui::Text("Mouse Color: ");
		ImGui::SameLine();
		ImGui::ColorEdit4("mouseColor", (float*)&mouseColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		static int x = 0;
		ImGui::InputInt("x pos", &x);
		static int y = 0;
		ImGui::InputInt("y pos", &y);
		auto selectedColor = pixelObserver.getColor(bbe::Vector2(x, y));
		ImGui::Text("Selected Color: ");
		ImGui::SameLine();
		ImGui::ColorEdit4("selectedColor", (float*)&selectedColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		ImGui::Checkbox("Armed", &armed);

		if (armed && !armedLastFrame)
		{
			protectionColor = selectedColor;
		}
		if (armed && protectionColor != selectedColor)
		{
			if(!soundInstance.isPlaying()) soundInstance = mySound.play();
		}
		if (armed && protectionColor == selectedColor)
		{
			if (soundInstance.isPlaying())
			{
				soundInstance.stop();
			}
		}
		if (!armed && soundInstance.isPlaying())
		{
			soundInstance.stop();
		}


		armedLastFrame = armed;

		ImGui::End();
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame game;
	game.start(1280, 720, "Pixel Observer");
    return 0;
}

