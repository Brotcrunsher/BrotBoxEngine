#pragma once

#include "UtilDebug.h"
#include "UniquePointer.h"
#include "STLCapsule.h"

namespace bbe {
	namespace INTERNAL {
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
		typedef typename T                                           value_type;
		typedef typename T*                                          pointer;
		typedef typename const T*                                    const_pointer;
		typedef typename T&                                          reference;
		typedef typename const T&                                    const_reference;
		typedef typename size_t                                      size_type;
		typedef typename std::pointer_traits<T*>::difference_type    difference_type;
		typedef typename std::pointer_traits<T*>::rebind<const void> const_void_pointer;

	private:
		class PoolAllocatorDestroyer {
		private:
			PoolAllocator* m_pa;
		public:
			PoolAllocatorDestroyer(PoolAllocator *pa)
				: m_pa(pa)
			{
				//do nothing
			}

			void destroy(T* data) {
				m_pa->deallocate(data);
			}
		};

		static constexpr size_t POOLALLOCATORDEFAULSIZE = 1024;
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
		size_t m_openAllocations = 0;		//Used to find memory leaks
#endif //!BBE_DISABLE_ALL_SECURITY_CHECKS

		INTERNAL::PoolChunk<T>* m_data = nullptr;
		INTERNAL::PoolChunk<T>* m_head = nullptr;
		size_t m_size;

		Allocator* m_parentAllocator = nullptr;
		bool m_needsToDeleteParentAllocator = false;

	public:
		explicit PoolAllocator(size_t size = POOLALLOCATORDEFAULSIZE, Allocator* parentAllocator = nullptr)
			: m_size(size), m_parentAllocator(parentAllocator)
		{
			if (m_parentAllocator == nullptr) {
				m_parentAllocator = new Allocator();
				m_needsToDeleteParentAllocator = true;
			}
			m_data = m_parentAllocator->allocate(m_size);
			for (size_t i = 0; i < m_size - 1; i++) {
				m_data[i].nextPoolChunk = bbe::addressOf(m_data[i + 1]);
			}
			m_data[m_size - 1].nextPoolChunk = nullptr;
			m_head = m_data;
		}

		PoolAllocator(const PoolAllocator&  other) = delete; //Copy Constructor
		PoolAllocator(PoolAllocator&& other) = delete; //Move Constructor
		PoolAllocator& operator=(const PoolAllocator&  other) = delete; //Copy Assignment
		PoolAllocator& operator=(PoolAllocator&& other) = delete; //Move Assignment

		~PoolAllocator() {
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
			if (m_openAllocations != 0) {
				//TODO add further error handling
				debugBreak();
			}
#endif // !BBE_DISABLE_ALL_SECURITY_CHECKS
			if (m_data != nullptr && m_parentAllocator != nullptr) {
				m_parentAllocator->deallocate(m_data, m_size);
			}
			if (m_needsToDeleteParentAllocator) {
				delete m_parentAllocator;
			}
			m_data = nullptr;
			m_head = nullptr;
		}

		template <typename... arguments>
		UniquePointer<T, PoolAllocatorDestroyer> allocateObjectUniquePointer(arguments&&... args) {
			T* pointer = allocateObject(std::forward<arguments>(args)...);
			return UniquePointer<T, PoolAllocatorDestroyer>(pointer, PoolAllocatorDestroyer(this));
		}

		template <typename... arguments>
		T* allocateObject(arguments&&... args)
		{
			if (m_head == nullptr) {
				debugBreak();
				//TODO throw exception, keep returning nullptr or allocate more space?
				return nullptr;
			}
			INTERNAL::PoolChunk<T>* retVal = m_head;
			m_head = retVal->nextPoolChunk;
			T* realRetVal = new (retVal) T(std::forward<arguments>(args)...);
#ifndef BBE_DISABLE_ALL_SECURITY_CHECKS
			m_openAllocations++;
#endif // !BBE_DISABLE_ALL_SECURITY_CHECKS
			return realRetVal;
		}

		void deallocate(T* data) {
			//TODO check if data is in range of the original array
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