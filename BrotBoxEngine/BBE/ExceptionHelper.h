#pragma once

namespace bbe
{
#define CREATE_EXCEPTION(n)							\
	class n					                        \
	{												\
	private:										\
		const wchar_t* m_msg = L"";					\
													\
	public:											\
		n(const wchar_t* msg)	                        \
			: m_msg(msg)							\
		{}											\
													\
		n()					                        \
		{}											\
													\
	};
}
