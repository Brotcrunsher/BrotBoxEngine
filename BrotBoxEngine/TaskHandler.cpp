#include "BBE/TaskHandler.h"
#include "BBE/BrotTime.h"
#include "BBE/SimpleThread.h"
#include <mutex>

void bbe::TaskHandler::threadMain(double targetExecDurSeconds, std::function<void(std::atomic_bool&)>&& func)
{
	// Unfortunately atomics don't have a wait_for yet :(
	std::mutex dummyMutex;
	std::unique_lock dummyLock(dummyMutex);

	while (!stopRequested)
	{
		const bbe::TimePoint start;
		func(stopRequested);
		const bbe::TimePoint end;
		const auto dur = end - start;
		const double requiredMillis = dur.toMillis();
		const double fastBonus = targetExecDurSeconds * 1000 - requiredMillis;

		if (fastBonus <= 0)
		{
			std::this_thread::yield();
		}
		else
		{
			stopCondition.wait_for(
				dummyLock, 
				std::chrono::duration<double, std::milli>(fastBonus), 
				[&] { 
					return stopRequested.load(); 
				});
		}
	}
}

bbe::TaskHandler::~TaskHandler()
{
	stop();
}

void bbe::TaskHandler::stop()
{
	stopRequested = true;
	stopCondition.notify_all();
	for (size_t i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
	threads.clear();
	stopRequested = false;
}

void bbe::TaskHandler::addTask(double targetExecDurSeconds, std::function<void(std::atomic_bool&)>&& func)
{
	threads.emplace_back(std::thread(&TaskHandler::threadMain, this, targetExecDurSeconds, func));
	bbe::simpleThread::setName(threads[threads.size() - 1], "BBE TaskHandler");
}
