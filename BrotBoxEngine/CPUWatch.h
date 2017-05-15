#pragma once

#include <ctime>

namespace bbe
{
	class CPUWatch
	{
	private:
		std::clock_t m_start;

	public:
		CPUWatch()
		{
			//UNTESTED
			start();
		}

		double getTimeExpiredNanoseconds()
		{
			//UNTESTED
			return getTimeExpiredMicroseconds() * 1000.0;
		}

		double getTimeExpiredMicroseconds()
		{
			//UNTESTED
			return getTimeExpiredMilliseconds() * 1000.0;
		}

		double getTimeExpiredMilliseconds()
		{
			//UNTESTED
			return getTimeExpiredSeconds() * 1000.0;
		}

		double getTimeExpiredSeconds()
		{
			//UNTESTED
			return (std::clock() - m_start) / (double)CLOCKS_PER_SEC;
		}


		void start()
		{
			//UNTESTED
			m_start = std::clock();
		}
	};
}