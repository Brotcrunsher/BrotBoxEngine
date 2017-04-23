#pragma once

#include <memory>
#include "DataType.h"
#include "UtilTest.h"
#include <vector>
#include <iostream>

namespace bbe {
	class StackAllocatorDestructor {
	private:
		const void* m_data;
		void(*destructor)(const void*);
	public:
		template<class T>
		explicit StackAllocatorDestructor(const T& data) noexcept :
			m_data(std::addressof(data)) {
			destructor = [](const void* data) {
				auto originalType = static_cast<const T*>(data);
				originalType->~T();
			};
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

	template <typename T = byte, typename Allocator = std::allocator<T>>
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
		static const size_t STACKALLOCATORDEFAULSIZE = 1024;
		T* m_bottomPointer = nullptr;
		T* m_headPointer = nullptr;
		size_t m_size = 0;

		Allocator* m_parentAllocator = nullptr;
		bool m_needsToDeleteParentAllocator = false;
		
		std::vector<StackAllocatorDestructor> destructors; //TODO change to own container type
	public:
		explicit StackAllocator(size_t size = STACKALLOCATORDEFAULSIZE, Allocator* parentAllocator = nullptr)
			: m_size(size), m_parentAllocator(parentAllocator) 
		{
			if (m_parentAllocator == nullptr) {
				m_parentAllocator = new Allocator();
				m_needsToDeleteParentAllocator = true;
			}
			m_bottomPointer = m_parentAllocator->allocate(m_size);
			m_headPointer = m_bottomPointer;

			memset(m_bottomPointer, 0, m_size);
		}

		~StackAllocator() {
			if (m_bottomPointer != m_headPointer) {
				//TODO add further error handling
				debugBreak();
			}
			if (m_bottomPointer != nullptr && m_parentAllocator != nullptr) {
				m_parentAllocator->deallocate(m_bottomPointer, m_size);
			}
			if (m_needsToDeleteParentAllocator) {
				delete m_parentAllocator;
			}
			m_bottomPointer = nullptr;
			m_headPointer = nullptr;
		}

		StackAllocator(const StackAllocator&  other) = delete; //Copy Constructor
		StackAllocator(const StackAllocator&& other) = delete; //Move Constructor
		StackAllocator& operator=(const StackAllocator&  other) = delete; //Copy Assignment
		StackAllocator& operator=(StackAllocator&& other) = delete; //Move Assignment

		template <typename U, typename... arguments>
		U* allocateObject(size_t amountOfObjects = 1, arguments&&... args) {
			T* allocationLocation = (T*)nextMultiple((size_t)alignof(T), (size_t)m_headPointer);
			T* newHeadPointer = allocationLocation + amountOfObjects * sizeof(U);
			if (newHeadPointer <= m_bottomPointer + m_size) {
				U* returnPointer = reinterpret_cast<U*>(allocationLocation);
				m_headPointer = newHeadPointer;
				for (size_t i = 0; i < amountOfObjects; i++) {
					U* object = new (std::addressof(returnPointer[i])) U(std::forward<arguments>(args)...);
					destructors.push_back(StackAllocatorDestructor(*object));
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
			T* allocationLocation = nextMultiple(alignment, m_headPointer);
			T* newHeadPointer = allocationLocation + amountOfBytes;
			if (newHeadPointer <= m_bottomPointer + m_size) {
				m_headPointer = newHeadPointer;
				return allocationLocation;
			}
			else {
				//TODO add additional errorhandling
				return nullptr;
			}
		}

		StackAllocatorMarker<T> getMarker() {
			return StackAllocatorMarker<T>(m_headPointer, destructors.size());
		}
		
		void deallocateToMarker(StackAllocatorMarker<T> sam) {
			m_headPointer = sam.m_markerValue;
			while (destructors.size() > sam.m_destructorHandle) {
				destructors.back()();
				destructors.pop_back();
			}
		}

		void deallocateAll() {
			//TODO call Destructors
			m_headPointer = m_bottomPointer;
			while (destructors.size() > 0) {
				StackAllocatorDestructor sad = destructors.back();
				sad();
				destructors.pop_back();
			}
		}

	};
	

}