#pragma once


#ifdef WIN32

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "../BBE/List.h"

namespace bbe
{
	class Monitor
	{
	private:
		std::mutex mutex;
		std::condition_variable gate;
		std::thread thread;
		bbe::List<float> targetBrightness = { };
		bbe::List<float> currentBrightness = { };
		std::atomic_bool stopRequested = false;

		void threadMain();

	public:
		Monitor();
		~Monitor();

		Monitor& operator=(Monitor&) = delete;
		Monitor(Monitor&) = delete;

		void setBrightness(const bbe::List<float>& brightnessPercentag);
	};
}
#endif
