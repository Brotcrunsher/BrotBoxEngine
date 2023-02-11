#pragma once

#include <future>

namespace bbe
{
	template<typename... Args>
	auto async(Args&&... args)
	{
		return std::async(
#ifdef __EMSCRIPTEN__
			std::launch::deferred,
#else
			std::launch::async,
#endif
			args...);
	}
}
