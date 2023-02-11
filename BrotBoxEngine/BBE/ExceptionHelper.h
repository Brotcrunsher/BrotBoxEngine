#pragma once
#include <iostream>

namespace bbe
{
#define CREATE_EXCEPTION(n)							\
	class n					                        \
	{												\
	private:										\
		const char* m_msg = "";					    \
													\
	public:											\
		explicit n(const char* msg)	                \
			: m_msg(msg)							\
		{                                           \
			std::cout << #n << std::endl;           \
			std::cout << msg << std::endl;          \
		}											\
													\
		n()					                        \
		{                                           \
			std::cout << #n << std::endl;           \
		}											\
													\
		const char* getMessage() const				\
		{											\
			return m_msg;							\
		}											\
													\
	}
}
