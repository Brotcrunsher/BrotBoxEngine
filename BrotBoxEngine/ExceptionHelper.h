#pragma once

#include "String.h"

namespace bbe
{
#define CREATE_EXCEPTION(n)							\
	class n					\
	{												\
	private:										\
		bbe::String m_msg = L"";					\
													\
	public:											\
		n(bbe::String msg)	\
			: m_msg(msg)							\
		{}											\
													\
		n()					\
		{}											\
													\
	};
}