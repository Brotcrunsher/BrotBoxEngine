#pragma once


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
			void destroy(T* data, size_t size)
			{
				delete[] data;
			}
		};
	}
}
