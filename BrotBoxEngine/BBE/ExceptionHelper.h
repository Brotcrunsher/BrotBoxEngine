#pragma once

namespace bbe
{
#define CREATE_EXCEPTION(n)							\
	class n					                        \
	{												\
	private:										\
		const char* m_msg = "";					\
													\
	public:											\
		n(const char* msg)	                        \
			: m_msg(msg)							\
		{}											\
													\
		n()					                        \
		{}											\
													\
	}
}
