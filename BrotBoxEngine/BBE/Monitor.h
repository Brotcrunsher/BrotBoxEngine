#pragma once


#ifdef WIN32

#include <thread>
#include <mutex>
#include <condition_variable>

namespace bbe
{
	class Monitor
	{
	private:
		std::mutex mutex;
		std::condition_variable gate;
		std::thread thread;
		float targetBrightness = -1.0f;
		float currentBrightness = -1.0f;
		bool stopRequested = false;

		void threadMain();

	public:
		Monitor();
		~Monitor();

		Monitor& operator=(Monitor&) = delete;
		Monitor(Monitor&) = delete;

		void setBrightness(float brightnessPercentag);
	};
}
#endif
