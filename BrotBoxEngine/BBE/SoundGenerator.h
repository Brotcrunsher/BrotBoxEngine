#pragma once

#include <variant>
#include "../BBE/BrotTime.h"
#include "../BBE/Sound.h"
#include "../BBE/List.h"

namespace bbe
{
	class SoundGenerator
	{
	public:
		struct RecipeSine
		{
			double frequency = 0.0;
		};

		struct RecipeSquare
		{
			double highDur = 0.0;
			double lowDur = 0.0;
			double highValue = 0.0;
			double lowValue = 0.0;
		};

		struct RecipeSawtooth
		{
			double period;
		};

		struct RecipeTriangle
		{
			double raiseDur = 0.0;
			double fallDur = 0.0;
			double upDur = 0.0;
			double downDur = 0.0;
		};

		struct RecipeADSR
		{
			double attackDur = 0.0;
			double decayDur = 0.0;
			double sustainLevel = 0.0;
			double releaseDur = 0.0;
		};

		struct RecipeNormalization
		{};

		struct RecipeRingModulation
		{
			double frequency = 0.0;
			double modDepth = 0.0;
		};

		struct RecipeChorusEffect
		{
			double delayTime = 0.0;
			double depth = 0.0;
			double rate = 0.0;
		};

		struct RecipeLowPassFilter
		{
			double cutoffFrequency = 0.0;
		};

		struct RecipeBitcrusher
		{
			int bitDepth = 0;
		};

		struct RecipeFrequencyShifter
		{
			double frequencyShift = 0.0;
		};

		struct RecipeEcho
		{
			double delayTime = 0.0;
			double decayFactor = 0.0;
		};

		struct Recipe
		{
			double offset = 0.0;
			double mult = 0.0;
			std::variant<RecipeSine, RecipeSquare, RecipeSawtooth, RecipeTriangle, RecipeADSR, RecipeNormalization, RecipeRingModulation, RecipeChorusEffect, RecipeLowPassFilter, RecipeBitcrusher, RecipeFrequencyShifter, RecipeEcho> details;
		};

		bbe::Duration duration;
		bbe::List<Recipe> recipes;

		constexpr static int64_t hz = 44100;

		SoundGenerator() = default;
		SoundGenerator(const bbe::Duration& duration);
		SoundGenerator(double durationMilliseconds);

		int64_t getAmountOfSamples() const;
		bbe::Sound finalize() const;

		void addRecipeSineWave(double offset = 0.0, double mult = 1.0, double frequency = 440.0);
		void addRecipeSquareWave(double offset = 0.0, double mult = 1.0, double highDur = 0.01, double lowDur = 0.01, double highValue = 1.0, double lowValue = -1.0);
		void addRecipeSawtoothWave(double offset = 0.0, double mult = 1.0, double period = 0.01);
		void addRecipeTriangleWave(double offset = 0.0, double mult = 1.0, double raiseDur = 0.01, double fallDur = 0.01, double upDur = 0.02, double downDur = 0.02);
		void addRecipeADSR(double attackDur = 0.1, double decayDur = 0.1, double sustainLevel = 0.7, double releaseDur = 0.2);
		void addRecipeNormalization();
		void addRecipeRingModulation(double offset = 0.0, double mult = 1.0, double frequency = 30.0, double modDepth = 0.5);
		void addRecipeChorusEffect(double offset = 0.0, double mult = 1.0, double delayTime = 0.03, double depth = 0.003, double rate = 1.5);
		void addRecipeLowPassFilter(double offset = 0.0, double mult = 1.0, double cutoffFrequency = 1000.0);
		void addRecipeBitcrusher(double offset = 0.0, double mult = 1.0, int bitDepth = 8);
		void addRecipeFrequencyShifter(double offset = 0.0, double mult = 1.0, double frequencyShift = 100.0);
		void addRecipeEcho(double offset = 0.0, double mult = 1.0, double delayTime = 0.3, double decayFactor = 0.5);

	private:
		void applyRecipes(bbe::List<double>& signal) const;
		void applyRecipeSineWave(bbe::List<double>& signal, double offset, double mult, double frequency) const;
		void applyRecipeSquareWave(bbe::List<double>& signal, double offset, double mult, double highDur, double lowDur, double highValue, double lowValue) const;
		void applyRecipeSawtoothWave(bbe::List<double>& signal, double offset, double mult, double period) const;
		void applyRecipeTriangleWave(bbe::List<double>& signal, double offset, double mult, double raiseDur, double fallDur, double upDur, double downDur) const;
		void applyRecipeADSR(bbe::List<double>& signal, double attackDur, double decayDur, double sustainLevel, double releaseDur) const;
		void applyRecipeNormalization(bbe::List<double>& signal) const;
		void applyRecipeRingModulation(bbe::List<double>& signal, double offset, double mult, double frequency, double modDepth) const;
		void applyRecipeChorusEffect(bbe::List<double>& signal, double offset, double mult, double delayTime, double depth, double rate) const;
		void applyRecipeLowPassFilter(bbe::List<double>& signal, double offset, double mult, double cutoffFrequency) const;
		void applyRecipeBitcrusher(bbe::List<double>& signal, double offset, double mult, int bitDepth) const;
		void applyRecipeFrequencyShifter(bbe::List<double>& signal, double offset, double mult, double frequencyShift) const;
		void applyRecipeEcho(bbe::List<double>& signal, double offset, double mult, double delayTime, double decayFactor) const;
	};
}
