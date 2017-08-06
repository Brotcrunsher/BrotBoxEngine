#pragma once

#include <memory>
#include <algorithm>

namespace bbe
{
	template <typename T>
	T* addressOf(T& t)
	{
		return std::addressof(t);
	}

	template <typename RandomIterator>
	void sortSTL(RandomIterator start, RandomIterator end)
	{
		std::sort(start, end);
	}

	template <typename RandomIterator, typename Predicate>
	void sortSTL(RandomIterator start, RandomIterator end, Predicate pred)
	{
		std::sort(start, end, pred);
	}
}
