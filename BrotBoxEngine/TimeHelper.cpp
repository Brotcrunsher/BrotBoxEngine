#include "BBE/TimeHelper.h"
#include <chrono>

uint64_t bbe::TimeHelper::getTimeStamp()
{
	return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
}
