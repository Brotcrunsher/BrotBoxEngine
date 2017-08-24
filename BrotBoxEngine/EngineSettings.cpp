#include "stdafx.h"
#include "BBE/EngineSettings.h"
#include "BBE/Exceptions.h"

static bool started = false;
static int amountOfTransformContainers = 1;
static int amountOfLightSources = 4;

void bbe::Settings::INTERNAL_start()
{
	started = true;
}

void bbe::Settings::setAmountOfTransformContainers(int amount)
{
	if (started)
	{
		throw AlreadyStartedException();
	}
	amountOfTransformContainers = amount;
}

int bbe::Settings::getAmountOfTransformContainers()
{
	return amountOfTransformContainers;
}

int bbe::Settings::getAmountOfLightSources()
{
	return amountOfLightSources;
}
