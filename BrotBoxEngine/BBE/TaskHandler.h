#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <condition_variable>
#include <functional>

namespace bbe
{
	class TaskHandler
	{
	private:
		std::atomic_bool stopRequested = false;
		std::vector<std::thread> threads;
		std::condition_variable stopCondition;

		void threadMain(double targetExecDurSeconds, std::function<void(std::atomic_bool&)>&& func);

	public:
		TaskHandler() = default;
		~TaskHandler();

		TaskHandler(const TaskHandler&) = delete;
		TaskHandler(TaskHandler&&) = delete;
		TaskHandler& operator=(const TaskHandler&) = delete;
		TaskHandler& operator=(TaskHandler&&) = delete;

		void stop();
		void addTask(double targetExecDurSeconds, std::function<void(std::atomic_bool&)>&& func);
	};
}
