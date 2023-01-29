#include "BBE/EngineSettings.h"
#include "BBE/Exceptions.h"

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
		throw AlreadyStartedException();
	}

	amountOfLightSources = amount;
}
#endif

int bbe::Settings::getTerrainAdditionalTextures()
{
	//UNTESTED
	return 2;
}

void bbe::Settings::setShaderDoublesAllowed(bool allowed)
{
	if (started)
	{
		throw AlreadyStartedException();
	}

	shaderDoublesAllowed = allowed;
}

bool bbe::Settings::getShaderDoublesAllowed()
{
	return shaderDoublesAllowed;
}
