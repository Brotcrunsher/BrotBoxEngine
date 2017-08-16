#include "stdafx.h"
#include "BBE/EngineSettings.h"

static bool started = false;
static int amountOfTransformContainers = 1;

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
