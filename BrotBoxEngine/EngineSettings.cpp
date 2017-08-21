#include "stdafx.h"
#include "BBE/EngineSettings.h"

static bool started = false;
static int amountOfTransformContainers = 1;
static int amountOfLightSources = 4;

void bbe::Settings::INTERNAL_start()
{
	started = true;
}

void bbe::Settings::setAmountOfTransformContainers(int amount)
{
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
