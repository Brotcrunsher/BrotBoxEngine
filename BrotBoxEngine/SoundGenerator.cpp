#include "../BBE/SoundGenerator.h"

bbe::SoundGenerator::SoundGenerator(const bbe::Duration& duration) : duration(duration)
{}

bbe::SoundGenerator::SoundGenerator(double durationMilliseconds) : duration(bbe::Duration::fromMilliseconds(durationMilliseconds))
{}

int64_t bbe::SoundGenerator::getAmountOfSamples() const
{
	int64_t millis = duration.toMillis();
	return millis * hz / 1000;
}

bbe::Sound bbe::SoundGenerator::finalize() const
{
	bbe::List<double> signal;
	signal.resizeCapacityAndLength(getAmountOfSamples());

	applyRecipes(signal);

	bbe::Sound retVal;
	retVal.load(signal.as<float>());
	return retVal;
}

void bbe::SoundGenerator::addRecipeSineWave(double offset, double mult, double frequency)
{
	recipes.add(Recipe(offset, mult, RecipeSine(frequency)));
}

void bbe::SoundGenerator::addRecipeSquareWave(double offset, double mult, double highDur, double lowDur, double highValue, double lowValue)
{
	recipes.add(Recipe(offset, mult, RecipeSquare(highDur, lowDur, highValue, lowValue)));
}

void bbe::SoundGenerator::addRecipeSawtoothWave(double offset, double mult, double period)
{
	recipes.add(Recipe(offset, mult, RecipeSawtooth(period)));
}

void bbe::SoundGenerator::addRecipeTriangleWave(double offset, double mult, double raiseDur, double fallDur, double upDur, double downDur)
{
	recipes.add(Recipe(offset, mult, RecipeTriangle(raiseDur, fallDur, upDur, downDur)));
}

void bbe::SoundGenerator::addRecipeADSR(double attackDur, double decayDur, double sustainLevel, double releaseDur)
{
	recipes.add(Recipe(0, 0, RecipeADSR(attackDur, decayDur, sustainLevel, releaseDur)));
}

void bbe::SoundGenerator::addRecipeNormalization()
{
	recipes.add(Recipe(0, 0, RecipeNormalization()));
}

void bbe::SoundGenerator::addRecipeRingModulation(double offset, double mult, double frequency, double modDepth)
{
	recipes.add(Recipe(offset, mult, RecipeRingModulation{ frequency, modDepth }));
}

void bbe::SoundGenerator::addRecipeChorusEffect(double offset, double mult, double delayTime, double depth, double rate)
{
	recipes.add(Recipe(offset, mult, RecipeChorusEffect{ delayTime, depth, rate }));
}

void bbe::SoundGenerator::addRecipeLowPassFilter(double offset, double mult, double cutoffFrequency)
{
	recipes.add(Recipe(offset, mult, RecipeLowPassFilter{ cutoffFrequency }));
}

void bbe::SoundGenerator::addRecipeBitcrusher(double offset, double mult, int bitDepth)
{
	recipes.add(Recipe(offset, mult, RecipeBitcrusher{ bitDepth }));
}

void bbe::SoundGenerator::addRecipeFrequencyShifter(double offset, double mult, double frequencyShift)
{
	recipes.add(Recipe(offset, mult, RecipeFrequencyShifter{ frequencyShift }));
}

void bbe::SoundGenerator::addRecipeEcho(double offset, double mult, double delayTime, double decayFactor)
{
	recipes.add(Recipe(offset, mult, RecipeEcho{ delayTime, decayFactor }));
}

void bbe::SoundGenerator::applyRecipes(bbe::List<double>& signal) const
{
	for (size_t i = 0; i < recipes.getLength(); i++)
	{
		const Recipe& r = recipes[i];

		std::visit([&](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, RecipeSine>)
				{
					applyRecipeSineWave(signal, r.offset, r.mult, arg.frequency);
				}
				else if constexpr (std::is_same_v<T, RecipeSquare>)
				{
					applyRecipeSquareWave(signal, r.offset, r.mult, arg.highDur, arg.lowDur, arg.highValue, arg.lowValue);
				}
				else if constexpr (std::is_same_v<T, RecipeSawtooth>)
				{
					applyRecipeSawtoothWave(signal, r.offset, r.mult, arg.period);
				}
				else if constexpr (std::is_same_v<T, RecipeTriangle>)
				{
					applyRecipeTriangleWave(signal, r.offset, r.mult, arg.raiseDur, arg.fallDur, arg.upDur, arg.downDur);
				}
				else if constexpr (std::is_same_v<T, RecipeADSR>)
				{
					applyRecipeADSR(signal, arg.attackDur, arg.decayDur, arg.sustainLevel, arg.releaseDur);
				}
				else if constexpr (std::is_same_v<T, RecipeNormalization>)
				{
					applyRecipeNormalization(signal);
				}
				else if constexpr (std::is_same_v<T, RecipeRingModulation>)
				{
					applyRecipeRingModulation(signal, r.offset, r.mult, arg.frequency, arg.modDepth);
				}
				else if constexpr (std::is_same_v<T, RecipeChorusEffect>)
				{
					applyRecipeChorusEffect(signal, r.offset, r.mult, arg.delayTime, arg.depth, arg.rate);
				}
				else if constexpr (std::is_same_v<T, RecipeLowPassFilter>)
				{
					applyRecipeLowPassFilter(signal, r.offset, r.mult, arg.cutoffFrequency);
				}
				else if constexpr (std::is_same_v<T, RecipeBitcrusher>)
				{
					applyRecipeBitcrusher(signal, r.offset, r.mult, arg.bitDepth);
				}
				else if constexpr (std::is_same_v<T, RecipeFrequencyShifter>)
				{
					applyRecipeFrequencyShifter(signal, r.offset, r.mult, arg.frequencyShift);
				}
				else if constexpr (std::is_same_v<T, RecipeEcho>)
				{
					applyRecipeEcho(signal, r.offset, r.mult, arg.delayTime, arg.decayFactor);
				}
				else
				{
					bbe::Crash(bbe::Error::IllegalArgument, "Illegal Recipe");
				}
			}, r.details);
	}
}

void bbe::SoundGenerator::applyRecipeSineWave(bbe::List<double>& signal, double offset, double mult, double frequency) const
{
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		signal[i] += bbe::Math::sin(offset + (double)i / hz * frequency * bbe::Math::PI * 2) * mult;
	}
}

void bbe::SoundGenerator::applyRecipeSquareWave(bbe::List<double>& signal, double offset, double mult, double highDur, double lowDur, double highValue, double lowValue) const
{
	const double totalDur = highDur + lowDur;
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		const double time = (double)i / hz + offset;
		if (fmod(time, totalDur) < highDur)
		{
			signal[i] += highValue * mult;
		}
		else
		{
			signal[i] += lowValue * mult;
		}
	}
}

void bbe::SoundGenerator::applyRecipeSawtoothWave(bbe::List<double>& signal, double offset, double mult, double period) const
{
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		double time = (double)i / hz + offset;
		double frac = fmod(time, period) / period;
		signal[i] += (frac * 2.0 - 1.0) * mult;
	}
}

void bbe::SoundGenerator::applyRecipeTriangleWave(bbe::List<double>& signal, double offset, double mult, double raiseDur, double fallDur, double upDur, double downDur) const
{
	const double totalDur = raiseDur + fallDur;
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		const double time = (double)i / hz + offset;
		const double modTime = fmod(time, upDur + downDur);

		if (modTime < upDur) // Rising part of the triangle wave
		{
			signal[i] += ((modTime / upDur) * raiseDur) * mult;
		}
		else // Falling part of the triangle wave
		{
			signal[i] += ((1.0 - ((modTime - upDur) / downDur)) * fallDur) * mult;
		}
	}
}

void bbe::SoundGenerator::applyRecipeADSR(bbe::List<double>& signal, double attackDur, double decayDur, double sustainLevel, double releaseDur) const
{
	size_t attackSamples = (size_t)(attackDur * hz);
	size_t decaySamples = (size_t)(decayDur * hz);
	size_t releaseSamples = (size_t)(releaseDur * hz);

	for (size_t i = 0; i < signal.getLength(); i++)
	{
		if (i < attackSamples) // Attack phase
		{
			signal[i] *= (double)i / attackSamples;
		}
		else if (i < attackSamples + decaySamples) // Decay phase
		{
			size_t decayIndex = i - attackSamples;
			signal[i] *= (1.0 - (1.0 - sustainLevel) * ((double)decayIndex / decaySamples));
		}
		else if (i >= signal.getLength() - releaseSamples) // Release phase
		{
			size_t releaseIndex = i - (signal.getLength() - releaseSamples);
			signal[i] *= (1.0 - (double)releaseIndex / releaseSamples);
		}
		else // Sustain phase
		{
			signal[i] *= sustainLevel;
		}
	}
}

void bbe::SoundGenerator::applyRecipeNormalization(bbe::List<double>& signal) const
{
	double maxAmplitude = 0.0;
	// Find the maximum absolute amplitude
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		if (fabs(signal[i]) > maxAmplitude)
		{
			maxAmplitude = fabs(signal[i]);
		}
	}
	// Normalize the signal to the maximum amplitude
	if (maxAmplitude > 0.0)
	{
		for (size_t i = 0; i < signal.getLength(); i++)
		{
			signal[i] /= maxAmplitude;
		}
	}
}

void bbe::SoundGenerator::applyRecipeRingModulation(bbe::List<double>& signal, double offset, double mult, double frequency, double modDepth) const
{
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		double modulator = bbe::Math::sin((double)i / hz * frequency * bbe::Math::PI * 2);
		signal[i] *= (1.0 + modDepth * modulator);
	}
}

void bbe::SoundGenerator::applyRecipeChorusEffect(bbe::List<double>& signal, double offset, double mult, double delayTime, double depth, double rate) const
{
	size_t delaySamples = (size_t)(delayTime * hz);
	for (size_t i = delaySamples; i < signal.getLength(); i++)
	{
		double mod = depth * bbe::Math::sin((double)i / hz * rate * bbe::Math::PI * 2);
		size_t modDelay = delaySamples + (size_t)(mod * hz);
		if (i >= modDelay)
		{
			signal[i] += signal[i - modDelay] * mult;
		}
	}
}

void bbe::SoundGenerator::applyRecipeLowPassFilter(bbe::List<double>& signal, double offset, double mult, double cutoffFrequency) const
{
	double RC = 1.0 / (cutoffFrequency * 2 * bbe::Math::PI);
	double dt = 1.0 / hz;
	double alpha = dt / (RC + dt);

	double previousValue = signal[0];
	for (size_t i = 1; i < signal.getLength(); i++)
	{
		signal[i] = previousValue + alpha * (signal[i] - previousValue);
		previousValue = signal[i];
	}
}

void bbe::SoundGenerator::applyRecipeBitcrusher(bbe::List<double>& signal, double offset, double mult, int bitDepth) const
{
	double maxValue = (1 << (bitDepth - 1)) - 1;
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		signal[i] = round(signal[i] * maxValue) / maxValue * mult;
	}
}

void bbe::SoundGenerator::applyRecipeFrequencyShifter(bbe::List<double>& signal, double offset, double mult, double frequencyShift) const
{
	for (size_t i = 0; i < signal.getLength(); i++)
	{
		signal[i] *= bbe::Math::sin((double)i / hz * frequencyShift * bbe::Math::PI * 2) * mult;
	}
}

void bbe::SoundGenerator::applyRecipeEcho(bbe::List<double>& signal, double offset, double mult, double delayTime, double decayFactor) const
{
	size_t delaySamples = (size_t)(delayTime * hz);
	for (size_t i = delaySamples; i < signal.getLength(); i++)
	{
		signal[i] += signal[i - delaySamples] * decayFactor * mult;
	}
}
