#pragma once
#include <stddef.h>

namespace bbe
{
	namespace INTERNAL
	{
		void allocCleanup();
	}
	struct AllocBlock
	{
		void* data = nullptr;
		size_t size = 0;
	};

	AllocBlock allocateBlock(size_t size);
	void freeBlock(AllocBlock& block);
}
