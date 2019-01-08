#pragma once

#include "../BBE/UtilDebug.h"
#include "../BBE/UniquePointer.h"
#include "../BBE/STLAllocator.h"
#include "../BBE/STLCapsule.h"
#include "../BBE/Exceptions.h"
#include <memory>

namespace bbe
{
	namespace INTERNAL
	{
		template <typename T>
		union PoolChunk
		{
			T value;
			PoolChunk<T>* nextPoolChunk;

			~PoolChunk() = delete;
		};
	}


	

	template <typename T, typename Allocator = STLAllocator<INTERNAL::PoolChunk<T>>>
	class PoolAllocator
	{
	public:
		typedef T                                                          value_type;
		typedef T*                                                         pointer;
		typedef const T*                                                   const_pointer;
		typedef T&                                                         reference;
		typedef const T&                                                   const_reference;
		typedef std::size_t                                                size_type;

	private:
		class PoolAllocatorDestroyer
		{
		private:
			PoolAllocator* m_pa;
		public:
			explicit PoolAllocatorDestroyer(PoolAllocator *pa)
				: m_pa(pa)
			{
				//do nothing
			}

			void destroy(T* data)
			{
				m_pa->deallocate(data);
			}
		};

		static constexpr std::size_t POOL_ALLOCATOR_DEFAULT_SIZE = 1024;
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
		std::size_t m_openAllocations = 0;		//Used to find memory leaks
#endif //!BBE_DISABLE_ALL_SECURITY_CHECKS

		INTERNAL::PoolChunk<T>* m_data = nullptr;
		INTERNAL::PoolChunk<T>* m_head = nullptr;
		std::size_t m_length;

		Allocator* m_parentAllocator = nullptr;
		bool m_needsToDeleteParentAllocator = false;

	public:
		explicit PoolAllocator(std::size_t size = POOL_ALLOCATOR_DEFAULT_SIZE, Allocator* parentAllocator = nullptr)
			: m_length(size), m_parentAllocator(parentAllocator)
		{
			if (m_parentAllocator == nullptr)
			{
				m_parentAllocator = new Allocator();
				m_needsToDeleteParentAllocator = true;
			}
			m_data = m_parentAllocator->allocate(m_length);
			for (std::size_t i = 0; i < m_length - 1; i++)
			{
				m_data[i].nextPoolChunk = bbe::addressOf(m_data[i + 1]);
			}
			m_data[m_length - 1].nextPoolChunk = nullptr;
			m_head = m_data;
		}

		PoolAllocator(const PoolAllocator&  other) = delete; //Copy Constructor
		PoolAllocator(PoolAllocator&& other) = delete; //Move Constructor
		PoolAllocator& operator=(const PoolAllocator&  other) = delete; //Copy Assignment
		PoolAllocator& operator=(PoolAllocator&& other) = delete; //Move Assignment

		~PoolAllocator()
		{
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
			if (m_openAllocations != 0)
			{
				debugBreak();
			}
#endif // !BBE_DISABLE_ALL_SECURITY_CHECKS
			if (m_data != nullptr && m_parentAllocator != nullptr)
			{
				m_parentAllocator->deallocate(m_data, m_length);
			}
			if (m_needsToDeleteParentAllocator)
			{
				delete m_parentAllocator;
			}
			m_data = nullptr;
			m_head = nullptr;
		}

		template <typename... arguments>
		UniquePointer<T, PoolAllocatorDestroyer> allocateObjectUniquePointer(arguments&&... args)
		{
			T* pointer = allocateObject(std::forward<arguments>(args)...);
			return UniquePointer<T, PoolAllocatorDestroyer>(pointer, PoolAllocatorDestroyer(this));
		}

		template <typename... arguments>
		T* allocateObject(arguments&&... args)
		{
			if (m_head == nullptr)
			{
				debugBreak();
				throw AllocatorOutOfMemoryException();
			}
			INTERNAL::PoolChunk<T>* retVal = m_head;
			m_head = retVal->nextPoolChunk;
			T* realRetVal = new (retVal) T(std::forward<arguments>(args)...);
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
			m_openAllocations++;
#endif // !BBE_DISABLE_ALL_SECURITY_CHECKS
			return realRetVal;
		}

		void deallocate(T* data)
		{
			if (data < reinterpret_cast<T*>(m_data))
			{
				throw MalformedPointerException();
			}
			if (data > reinterpret_cast<T*>(m_data) + m_length)
			{
				throw MalformedPointerException();
			}
			data->~T();
			INTERNAL::PoolChunk<T>* poolChunk = reinterpret_cast<INTERNAL::PoolChunk<T>*>(data);
			poolChunk->nextPoolChunk = m_head;
			m_head = poolChunk;
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
			m_openAllocations--;
#endif //!BBE_DISABLE_ALL_SECURITY_CHECKS
		}
	};
}
