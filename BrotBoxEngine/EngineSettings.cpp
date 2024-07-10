#include "BBE/EngineSettings.h"
#include "BBE/Error.h"

static bool started = false;
static int amountOfLightSources = 4;
static bool shaderDoublesAllowed = false;

void bbe::Settings::INTERNAL_start()
{
	started = true;
}

#ifdef BBE_RENDERER_VULKAN
int bbe::Settings::getAmountOfLightSources()
{
	//UNTESTED
	return amountOfLightSources;
}

void bbe::Settings::setAmountOfLightSources(int amount)
{
	//UNTESTED
	if (started)
	{
		bbe::Crash(bbe::Error::AlreadyStarted);
	}

	amountOfLightSources = amount;
}
#endif

void bbe::Settings::setShaderDoublesAllowed(bool allowed)
{
	if (started)
	{
		bbe::Crash(bbe::Error::AlreadyStarted);
	}

	shaderDoublesAllowed = allowed;
}

bool bbe::Settings::getShaderDoublesAllowed()
{
	return shaderDoublesAllowed;
}
