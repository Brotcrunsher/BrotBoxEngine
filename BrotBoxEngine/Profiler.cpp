#include "BBE/Profiler.h"

static float renderTime = 0;
static float cpuTime = 0;

float bbe::Profiler::getRenderTime()
{
	return renderTime;
}

float bbe::Profiler::getCPUTime()
{
	return cpuTime;
}

void bbe::Profiler::INTERNAL::setRenderTime(float time)
{
	renderTime = time;
}

void bbe::Profiler::INTERNAL::setCPUTime(float time)
{
	cpuTime = time;
}
