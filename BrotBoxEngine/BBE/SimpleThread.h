#pragma once

#include <thread>

namespace bbe
{
	namespace simpleThread
	{
		void setName(std::thread& thread, const char* name);
	}
}
