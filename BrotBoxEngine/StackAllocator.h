#pragma once

#include "DataType.h"
#include "UtilTest.h"
#include "List.h"
#include <iostream>
#include "STLCapsule.h"

namespace bbe {
	template <typename T>
	void executeDestructor(const void* data) {
		auto originalType = static_cast<const T*>(data);
		originalType->~T();
	}

	class StackAllocatorDestructor {
	private:
		const void* m_data;
		void(*destructor)(const void*);
	public:
		template<typename T>
		explicit StackAllocatorDestructor(const T& data) noexcept :
			m_data(bbe::addressOf(data)) {
			destructor = executeDestructor<T>;
		}

		void operator () () noexcept {
			destructor(m_data);
		}
	};

	template <typename T>
	class StackAllocatorMarker {
	public:
		T* m_markerValue;
		size_t m_destructorHandle;
		StackAllocatorMarker(T* markerValue, size_t destructorHandle) :
			m_markerValue(markerValue), m_destructorHandle(destructorHandle) {

		}
	};

	template <typename T = byte, typename Allocator = STLAllocator<T>>
	class StackAllocator {
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
		static constexpr size_t STACKALLOCATORDEFAULSIZE = 1024;
		T* m_data = nullptr;
		T* m_head = nullptr;
		size_t m_size = 0;

		Allocator* m_parentAllocator = nullptr;
		bool m_needsToDeleteParentAllocator = false;
		
		List<StackAllocatorDestructor> destructors;

		template<typename U>
		inline typename std::enable_if<std::is_trivially_destructible<U>::value>::type
			addDestructorToList(U* object)
		{
			//do nothing
		}

		template<typename U>
		inline typename std::enable_if<!std::is_trivially_destructible<U>::value>::type
			addDestructorToList(U* object)
		{
			destructors.pushBack(StackAllocatorDestructor(*object));
		}

	public:
		explicit StackAllocator(size_t size = STACKALLOCATORDEFAULSIZE, Allocator* parentAllocator = nullptr)
			: m_size(size), m_parentAllocator(parentAllocator) 
		{
			if (m_parentAllocator == nullptr) {
				m_parentAllocator = new Allocator();
				m_needsToDeleteParentAllocator = true;
			}
			m_data = m_parentAllocator->allocate(m_size);
			m_head = m_data;

			memset(m_data, 0, m_size);	//TODO evaluate if this should be here
		}

		~StackAllocator() {
			if (m_data != m_head) {
				//TODO add further error handling
				debugBreak();
			}
			if (m_data != nullptr && m_parentAllocator != nullptr) {
				m_parentAllocator->deallocate(m_data, m_size);
			}
			if (m_needsToDeleteParentAllocator) {
				delete m_parentAllocator;
			}
			m_data = nullptr;
			m_head = nullptr;
		}

		StackAllocator(const StackAllocator&  other) = delete; //Copy Constructor
		StackAllocator(StackAllocator&& other) = delete; //Move Constructor
		StackAllocator& operator=(const StackAllocator&  other) = delete; //Copy Assignment
		StackAllocator& operator=(StackAllocator&& other) = delete; //Move Assignment

		template <typename U, typename... arguments>
		U* allocateObject(size_t amountOfObjects = 1, arguments&&... args) {
			T* allocationLocation = (T*)nextMultiple(alignof(U), (size_t)m_head);
			T* newHeadPointer = allocationLocation + amountOfObjects * sizeof(U);
			if (newHeadPointer <= m_data + m_size) {
				U* returnPointer = reinterpret_cast<U*>(allocationLocation);
				m_head = newHeadPointer;
				for (size_t i = 0; i < amountOfObjects; i++) {
					U* object = bbe::addressOf(returnPointer[i]);
					new (object) U(std::forward<arguments>(args)...);
					addDestructorToList(object);
				}
				return returnPointer;
			}
			else {
				//TODO add additional errorhandling
				return nullptr;
			}
		}


		void* allocate(size_t amountOfBytes, size_t alignment = 1)
		{
			T* allocationLocation = (T*)nextMultiple(alignment, (size_t)m_head);
			T* newHeadPointer = allocationLocation + amountOfBytes;
			if (newHeadPointer <= m_data + m_size) {
				m_head = newHeadPointer;
				return allocationLocation;
			}
			else {
				//TODO add additional errorhandling
				return nullptr;
			}
		}

		StackAllocatorMarker<T> getMarker() {
			return StackAllocatorMarker<T>(m_head, destructors.getLength());
		}
		
		void deallocateToMarker(StackAllocatorMarker<T> sam) {
			m_head = sam.m_markerValue;
			while (destructors.getLength() > sam.m_destructorHandle) {
				destructors.last()();
				destructors.popBack();
			}
		}

		void deallocateAll() {
			m_head = m_data;
			while (destructors.size() > 0) {
				destructors.back()();
				destructors.pop_back();
			}
		}

	};
	

}