#include "BBE/AllocBlock.h"
#include "BBE/List.h"
#include "BBE/Math.h"
#include "BBE/Logging.h"
#include <cstdlib>
#include <vector>
#include <array>

static bool isBlockZero(void* ptr, size_t size)
{
	char* cptr = (char*)ptr;
	return cptr[0] == 0 && !memcmp(cptr, cptr + 1, size - 1);
}

static std::array<std::vector<void*>, 64> storedBlocks; // Can't be a bbe::List, because bbe::List depends on this allocator.
static constexpr size_t BBE_PAGE_SIZE = 4096;
static bool allocatorShutdown = false;

void bbe::INTERNAL::allocCleanup()
{
	allocatorShutdown = true;
	for (size_t i = 0; i < storedBlocks.size(); i++)
	{
		for (size_t k = 0; k < storedBlocks[i].size(); k++)
		{
			std::free(storedBlocks[i][k]);
		}
		storedBlocks[i].clear();
	}
}

static void checkBlockHealth()
{
#ifdef _DEBUG
	void* block = nullptr;
	for (int i = 0; i < 64; i++)
	{
		const size_t size = (((size_t)1) << (i));
		for (size_t k = 0; k < storedBlocks[i].size(); k++)
		{
			block = storedBlocks[i][k];
			if (!isBlockZero(block, size))
			{
				throw bbe::IllegalStateException();
			}
		}
	}
#endif
}

bbe::AllocBlock bbe::allocateBlock(size_t size)
{
	checkBlockHealth();
	if (size == 0) return AllocBlock{ nullptr, 0 };

	auto access = bbe::Math::log2Floor(size);
	if (!bbe::Math::isPowerOfTwo(size))
	{
		size = (((size_t)1) << (access + 1));
		access++;
	}
	
	if (storedBlocks[access].size() != 0)
	{
		void* addr = storedBlocks[access].back();
		storedBlocks[access].pop_back();
#ifdef _DEBUG
		if (!isBlockZero(addr, size))
		{
			throw IllegalStateException();
		}
#endif
		return AllocBlock{ addr, size };
	}

	void* ptr = std::malloc(size);
	if (!ptr) throw NullPointerException();
	return AllocBlock{ ptr , size };
}

void bbe::freeBlock(AllocBlock& block)
{
	if (allocatorShutdown)
	{
		std::free(block.data);
	}
	else
	{
		checkBlockHealth();
		if (block.data == nullptr || block.size == 0) return;

		auto access = bbe::Math::log2Floor(block.size);
		storedBlocks[access].push_back(block.data);
#ifdef _DEBUG
		memset(block.data, 0, block.size);
#endif
	}

	block.data = nullptr;
	block.size = 0;
}



// From: https://en.cppreference.com/w/cpp/memory/new/operator_new
// See for license: https://en.cppreference.com/w/Cppreference:FAQ
// Quote:
// What can I do with the material on this site?
// The content is licensed under Creative Commons Attribution - Sharealike 3.0 
// Unported License(CC - BY - SA) and by the GNU Free Documentation License(GFDL)
// (unversioned, with no invariant sections, front - cover texts, or back - cover
// texts).That means that you can use this site in almost any way you like,
// including mirroring, copying, translating, etc.All we would ask is to provide
// link back to cppreference.com so that people know where to get the most up -
// to - date content.In addition to that, any modified content should be released
// under an equivalent license so that everyone could benefit from the modified
// versions.

//#define CHECK_ALLOCS
#ifdef CHECK_ALLOCS
#include <cstdio>
#include <cstdlib>
#include <new>
 
// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
    std::printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
 
// no inline, required by [replacement.functions]/3
void* operator new[](std::size_t sz)
{
    std::printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
 
void operator delete(void* ptr) noexcept
{
    std::puts("3) delete(void*)");
    std::free(ptr);
}
 
void operator delete(void* ptr, std::size_t size) noexcept
{
    std::printf("4) delete(void*, size_t), size = %zu\n", size);
    std::free(ptr);
}
 
void operator delete[](void* ptr) noexcept
{
    std::puts("5) delete[](void* ptr)");
    std::free(ptr);
}
 
void operator delete[](void* ptr, std::size_t size) noexcept
{
    std::printf("6) delete[](void*, size_t), size = %zu\n", size);
    std::free(ptr);
}
#endif
