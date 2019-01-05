#pragma once

#include <cstdio>

namespace bbe
{
	namespace INTERNAL
	{
		class DefaultDestroyer
		{
		public:
			template <typename T>
			void destroy(T* data)
			{
				delete data;
			}

			template <typename T>
			void destroy(T* data, std::size_t size)
			{
				delete[] data;
			}
		};
	}
}
