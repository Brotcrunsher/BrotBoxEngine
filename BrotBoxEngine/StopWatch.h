#pragma once

#include <chrono>

namespace bbe
{
	class StopWatch
	{
	private:
		std::chrono::high_resolution_clock::time_point m_start;

		template <typename T>
		long long getTimeExpired()
		{
			std::chrono::high_resolution_clock::time_point current = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<T>(current - m_start).count();
		}

	public:
		StopWatch()
		{
			//UNTESTED
			start();
		}

		long long getTimeExpiredNanoseconds()
		{
			//UNTESTED
			return getTimeExpired<std::chrono::nanoseconds>();
		}

		long long getTimeExpiredMicroseconds()
		{
			//UNTESTED
			return getTimeExpired<std::chrono::microseconds>();
		}

		long long getTimeExpiredMilliseconds()
		{
			//UNTESTED
			return getTimeExpired<std::chrono::milliseconds>();
		}

		long long getTimeExpiredSeconds()
		{
			//UNTESTED
			return getTimeExpired<std::chrono::seconds>();
		}

		long long getTimeExpiredMinutes()
		{
			//UNTESTED
			return getTimeExpired<std::chrono::minutes>();
		}

		long long getTimeExpiredHours()
		{
			//UNTESTED
			return getTimeExpired<std::chrono::hours>();
		}


		void start()
		{
			//UNTESTED
			m_start = std::chrono::high_resolution_clock::now();
		}
	};
}