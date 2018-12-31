#pragma once


#include "../BBE/Array.h"
#include <stdint.h>
#include <type_traits>

namespace bbe
{
	template<typename T>
	uint32_t hash(const T &t)
	{
		static_assert(std::is_fundamental<T>::value && !std::is_class<T>::value, "No valid hash function found.");
		return static_cast<uint32_t>(t);
	}





}
