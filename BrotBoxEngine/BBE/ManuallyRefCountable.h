#pragma once

#include <cstdint>

namespace bbe
{
	class ManuallyRefCountable
	{
	private:
		int32_t refCount = 1;
	public:
		virtual ~ManuallyRefCountable();

		void incRef();
		void decRef();
	};
}