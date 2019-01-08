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
		explicit n(const char* msg)	                        \
			: m_msg(msg)							\
		{}											\
													\
		n()					                        \
		{}											\
													\
	}
}
