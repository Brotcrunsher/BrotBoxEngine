#pragma once

#include "DataType.h"
#include "List.h"
#include "UtilMath.h"
#include "UtilTest.h"

namespace bbe {
	class GeneralPurposeAllocatorFreeChunk {
	public:
		byte* m_addr;
		size_t m_size;

		GeneralPurposeAllocatorFreeChunk(byte* addr, size_t size) 
			: m_addr(addr), m_size(size)
		{
			//do nothing
		}

		bool touches(const GeneralPurposeAllocatorFreeChunk& other) const {
			//UNTESTED
			if (m_addr + m_size == other.m_addr) {
				return true;
			}
			if (other.m_addr + other.m_size == m_addr) {
				return true;
			}

			return false;
		}

		bool operator>(const GeneralPurposeAllocatorFreeChunk& other) const {
			return m_addr > other.m_addr;
		}

		bool operator>=(const GeneralPurposeAllocatorFreeChunk& other) const {
			return m_addr >= other.m_addr;
		}

		bool operator<(const GeneralPurposeAllocatorFreeChunk& other) const {
			return m_addr < other.m_addr;
		}

		bool operator<=(const GeneralPurposeAllocatorFreeChunk& other) const {
			return m_addr <= other.m_addr;
		}

		bool operator==(const GeneralPurposeAllocatorFreeChunk& other) const {
			return m_addr == other.m_addr;
		}

		template <typename T, typename... arguments>
		T* allocateObject(size_t amountOfObjects = 1, arguments&&... args) {
			//UNTESTED
			byte* allocationLocation = (byte*)nextMultiple(alignof(T), ((size_t)m_addr) + 1);
			size_t amountOfBytes = amountOfObjects * sizeof(T);
			byte* newAddr = allocationLocation + amountOfBytes;
			if (newAddr <= m_addr + m_size) {
				byte offset = allocationLocation - m_addr;
				allocationLocation[-1] = offset;
				T* returnPointer = reinterpret_cast<T*>(allocationLocation);
				m_size -= newAddr - m_addr;
				m_addr = newAddr;
				for (size_t i = 0; i < amountOfObjects; i++) {
					T* object = bbe::addressOf(returnPointer[i]);
					new (object) T(std::forward<arguments>(args)...);
				}
				return returnPointer;
			}
			else {
				return nullptr;
			}
		}
	};

	class GeneralPurposeAllocator {
		//TODO use parent allocator
		//TODO defragmentation
	private:
		static const size_t GENERALPURPOSEALLOCATORDEFAULTSIZE = 1024;
		byte* m_data;
		size_t m_size;

		List<GeneralPurposeAllocatorFreeChunk, true> freeChunks;

	public:
		GeneralPurposeAllocator(size_t size = GENERALPURPOSEALLOCATORDEFAULTSIZE)
			: m_size(size)
		{
			//UNTESTED
			m_data = new byte[m_size];
			freeChunks.pushBack(GeneralPurposeAllocatorFreeChunk(m_data, m_size));
		}

		~GeneralPurposeAllocator()
		{
			if (freeChunks.getLength() != 1) {
				debugBreak();
			}
			if (freeChunks[0].m_addr != m_data) {
				debugBreak();
			}
			if (freeChunks[0].m_size != m_size) {
				debugBreak();
			}
			if (m_data != nullptr) {
				delete[] m_data;
				m_data = nullptr;
			}
		}

		GeneralPurposeAllocator(const GeneralPurposeAllocator& other) = delete;
		GeneralPurposeAllocator(GeneralPurposeAllocator&& other) = delete;
		GeneralPurposeAllocator& operator=(const GeneralPurposeAllocator& other) = delete;
		GeneralPurposeAllocator& operator=(GeneralPurposeAllocator&& other) = delete;

		template <typename T, typename... arguments>
		T* allocateObjects(size_t amountOfObjects = 1, arguments&&... args) {
			//UNTESTED
			for (size_t i = 0; i < freeChunks.getLength(); i++) {
				T* data = freeChunks[i].allocateObject<T>(amountOfObjects, std::forward<arguments>(args)...);
				if (data != nullptr) {
					return data;
				}
			}

			//TODO add further error handling
			return nullptr;
		}

		template<typename T>
		void deallocateObjects(T* dataPointer, size_t amountOfObjects = 1) {
			//UNTESTED
			for (size_t i = 0; i < amountOfObjects; i++) {
				bbe::addressOf(dataPointer[i])->~T();
			}

			byte* bytePointer = reinterpret_cast<byte*>(dataPointer);
			size_t amountOfBytes = sizeof(T) * amountOfObjects;
			byte offset = bytePointer[-1];

			//TODO add this to the freeChunks list
			GeneralPurposeAllocatorFreeChunk gpafc(bytePointer - offset, amountOfBytes + offset);

			GeneralPurposeAllocatorFreeChunk* p_gpafc = &gpafc;
			GeneralPurposeAllocatorFreeChunk* left;
			bool didTouchLeft = false;
			bool didMerge = false;
			GeneralPurposeAllocatorFreeChunk* right;

			freeChunks.getNeighbors(*p_gpafc, left, right);
			if (left != nullptr) {
				if (left->touches(*p_gpafc)) {
					left->m_size += p_gpafc->m_size;
					didTouchLeft = true;
					p_gpafc = left;
					didMerge = true;
				}
			}
			if (right != nullptr) {
				if (right->touches(*p_gpafc)) {
					if (didTouchLeft) {
						p_gpafc->m_size += right->m_size;
						freeChunks.removeSingle(*right);
					}
					else {
						right->m_size += p_gpafc->m_size;
						right->m_addr = p_gpafc->m_addr;
					}
					didMerge = true;
				}
			}

			if (!didMerge) {
				freeChunks.pushBack(gpafc);
			}
		}
	};
}