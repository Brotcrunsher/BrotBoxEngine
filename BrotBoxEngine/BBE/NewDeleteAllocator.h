#pragma once

#include <cassert>
#include <utility>

namespace bbe
{
	class NewDeleteAllocator
	{
	public:
		NewDeleteAllocator(size_t size)
		{
			assert(size == 0 && "size can't be set for a NewDeleteAllocator!");
		}

		template <typename T, typename... arguments>
		T* allocateObject(arguments&&... args)
		{
			return new T(std::forward<arguments>(args)...);
		}

		template <typename T>
		T* allocateObjects(size_t amountOfObjects = 1)
		{
			return new T[amountOfObjects];
		}

		template <typename T>
		void deallocate(T* data)
		{
			delete data;
		}

		template <typename T>
		void deallocateArray(T* data)
		{
			delete[] data;
		}
	};
}