#pragma once

namespace bbe
{
	class NewDeleteAllocator
	{
	public:
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