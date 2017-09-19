#include "stdafx.h"
#include "BBE/EngineSettings.h"
#include "BBE/Exceptions.h"

static bool started = false;
static int amountOfLightSources = 4;

void bbe::Settings::INTERNAL_start()
{
	started = true;
}

int bbe::Settings::getAmountOfLightSources()
{
	return amountOfLightSources;
}

void bbe::Settings::setAmountOfLightSources(int amount)
{
	if (started)
	{
		throw AlreadyStartedException();
	}

	amountOfLightSources = amount;
}

int bbe::Settings::getTerrainAdditionalTextures()
{
	return 2;
}
