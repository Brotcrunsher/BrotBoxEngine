#pragma once

#include "../BBE/DataType.h"
#include "../BBE/List.h"
#include "../BBE/Stack.h"
#include "../BBE/Math.h"
#include "../BBE/UniquePointer.h"
#include "../BBE/UtilTest.h"
#include "../BBE/EmptyClass.h"
#include "../BBE/Unconstructed.h"
#include "../BBE/GeneralPurposeAllocator.h"
#include "../BBE/Stack.h"
#include "../BBE/Exceptions.h"

namespace bbe
{
	class DefragmentationAllocator
	{
		//TODO use parent allocator
		//TODO defragmentation
	public:
		template<typename T>
		class DefragmentationAllocatorPointer
		{
			friend class DefragmentationAllocator;
		private:
			size_t m_handleIndex;
			size_t m_length;
			DefragmentationAllocator *m_pparent;
		public:
			DefragmentationAllocatorPointer()
				: m_handleIndex(0), m_length(0), m_pparent(nullptr)
			{
				//do nothing
			}

			DefragmentationAllocatorPointer(DefragmentationAllocator *parent, size_t handleIndex, size_t size)
				: m_pparent(parent), m_handleIndex(handleIndex), m_length(size)
			{
				//do nothing
			}

			DefragmentationAllocatorPointer& operator=(T* other)
			{
				assert(other == nullptr);
				m_handleIndex = 0;
				m_length = 0;
				m_pparent = nullptr;

				return *this;
			}

			operator T*() const
			{
				return static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			T* operator ->()
			{
				return static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			const T* operator ->() const
			{
				//UNTESTED
				return static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			T& operator *()
			{
				//UNTESTED
				return *static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			const T& operator *() const
			{
				//UNTESTED
				return *static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			T& operator [](int index)
			{
				return *(static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]) + index);
			}

			const T& operator [](int index) const
			{
				return *(static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]) + index);
			}

			T* operator+(int index)
			{
				//UNTESTED
				return static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]) + index;
			}

			T* getRaw()
			{
				if (m_handleIndex == 0)
				{
					return nullptr;
				}
				return static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			const T* getRaw() const
			{
				if (m_handleIndex == 0)
				{
					return nullptr;
				}
				return static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
			}

			bool operator ==(void* ptr) const
			{
				return getRaw() == ptr;
			}

			bool operator !=(void* ptr) const
			{
				return getRaw() != ptr;
			}
		};

		template<typename T>
		class DefragmentationAllocatorDestroyer
		{
		private:
			DefragmentationAllocator* m_pa;
			DefragmentationAllocatorPointer<T> m_data;
		public:
			DefragmentationAllocatorDestroyer(DefragmentationAllocator *pa, DefragmentationAllocatorPointer<T> data)
				: m_pa(pa), m_data(data)
			{
				//do nothing
			}

			void destroy(void* data)
			{
				m_pa->deallocateObjects(m_data);
			}
		};

		class DefragmentationAllocatorRelocatable
		{
			friend class DefragmentationAllocator;

		private:
			DefragmentationAllocator *m_pparent;
			union {
				size_t m_handleIndex;
				void* m_paddr;
			};
			bool m_usedAddr = false;
			size_t m_amountOfObjects = 0;
			
			byte*(DefragmentationAllocatorRelocatable::*relocate)(void*);

			template<typename T>
			byte* relocateTemplate(void *newAddr)
			{
				static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded");
				byte* allocationLocation = (byte*)Math::nextMultiple(alignof(T), ((size_t)newAddr) + 1);
				size_t amountOfBytes = m_amountOfObjects * sizeof(T);

				byte offset = (byte)(allocationLocation - (byte*)newAddr);
				allocationLocation[-1] = offset;

				T* oldData = static_cast<T*>(m_pparent->m_handleTable[m_handleIndex]);
				T* newData = reinterpret_cast<T*>(allocationLocation);

				if (std::is_trivially_move_constructible<T>::value)
				{
					std::memmove(newData, oldData, amountOfBytes);
				}
				else if(allocationLocation + sizeof(T) < (byte*)oldData)
				{
					for (size_t i = 0; i < m_amountOfObjects; i++)
					{
						new (bbe::addressOf(newData[i])) T(std::move(oldData[i]));
						bbe::addressOf(oldData[i])->~T();
					}
				}
				else
				{
					byte tempByteArr[sizeof(T) + alignof(T)];
					T* tempObj = reinterpret_cast<T*>(Math::nextMultiple((size_t)alignof(T), (size_t)tempByteArr));

					for (size_t i = 0; i < m_amountOfObjects; i++)
					{
						new (tempObj) T(std::move(oldData[i]));
						bbe::addressOf(oldData[i])->~T();

						T* object = bbe::addressOf(newData[i]);
						new (object) T(std::move(*tempObj));
						tempObj->~T();
					}
				}

				m_pparent->m_handleTable[m_handleIndex] = allocationLocation;
				return static_cast<byte*>(allocationLocation + amountOfBytes);
			}

			template<typename T>
			DefragmentationAllocatorRelocatable(DefragmentationAllocator *parent, size_t handleIndex, size_t amountOfObjects, T *t)
				: m_pparent(parent), m_handleIndex(handleIndex), m_amountOfObjects(amountOfObjects)
			{
				relocate = &DefragmentationAllocatorRelocatable::relocateTemplate<T>;
			}

			DefragmentationAllocatorRelocatable(DefragmentationAllocator *parent, void* addr)
				: m_pparent(parent), m_paddr(addr), m_usedAddr(true)
			{
			}

			byte* operator() (void* newAddr)
			{
				return (this->*relocate)(newAddr);
			}

			void* getAddr() const
			{
				if (m_usedAddr)
				{
					return m_paddr;
				}
				else
				{
					return m_pparent->m_handleTable[m_handleIndex];
				}
			}

		public:

			bool operator>(const DefragmentationAllocatorRelocatable& other) const
			{
				return getAddr() > other.getAddr();
			}

			bool operator>=(const DefragmentationAllocatorRelocatable& other) const
			{
				return getAddr() >= other.getAddr();
			}

			bool operator<(const DefragmentationAllocatorRelocatable& other) const
			{
				return getAddr() < other.getAddr();
			}

			bool operator<=(const DefragmentationAllocatorRelocatable& other) const
			{
				return getAddr() <= other.getAddr();
			}

			bool operator==(const DefragmentationAllocatorRelocatable& other) const
			{
				return getAddr() == other.getAddr();
			}

			bool operator!=(const DefragmentationAllocatorRelocatable& other) const
			{
				return getAddr() != other.getAddr();
			}

		};
	private:
		static const size_t DEFRAGMENTATION_ALLOCAOTR_DEFAULT_SIZE = 1024;
		byte* m_data;
		size_t m_length;

		List<INTERNAL::GeneralPurposeAllocatorFreeChunk, true> m_freeChunks;
		
		size_t m_lengthOfHandleTable;
		void** m_handleTable;
		Stack<size_t> m_unusedHandleStack;
		List<DefragmentationAllocatorRelocatable, true> m_allocatedBlocks;

	public:
		explicit DefragmentationAllocator(size_t size = DEFRAGMENTATION_ALLOCAOTR_DEFAULT_SIZE, size_t lengthOfHandleTable = DEFRAGMENTATION_ALLOCAOTR_DEFAULT_SIZE / 4)
			: m_length(size), m_lengthOfHandleTable(lengthOfHandleTable)
		{
			//UNTESTED
			m_data = new byte[m_length];
			m_freeChunks.add(INTERNAL::GeneralPurposeAllocatorFreeChunk(m_data, m_length));

			m_handleTable = new void*[m_lengthOfHandleTable];
			memset(m_handleTable, 0, sizeof(void*) * m_lengthOfHandleTable);
			//Never add 0, this allows us to use 0 as a nullptr
			for (size_t i = lengthOfHandleTable - 1; i > 0; i--)
			{
				m_unusedHandleStack.push(i);
			}
		}

		~DefragmentationAllocator()
		{
			if (m_freeChunks.getLength() != 1)
			{
				debugBreak();
			}
			if (m_freeChunks[0].m_addr != m_data)
			{
				debugBreak();
			}
			if (m_freeChunks[0].m_length != m_length)
			{
				debugBreak();
			}
			if (m_data != nullptr)
			{
				delete[] m_data;
				m_data = nullptr;
			}

			if (m_handleTable != nullptr)
			{
				delete[] m_handleTable;
				m_handleTable = nullptr;
			}
		}

		DefragmentationAllocator(const DefragmentationAllocator& other) = delete;
		DefragmentationAllocator(DefragmentationAllocator&& other) = delete;
		DefragmentationAllocator& operator=(const DefragmentationAllocator& other) = delete;
		DefragmentationAllocator& operator=(DefragmentationAllocator&& other) = delete;

		template <typename T, int ALIGNMENT, typename... arguments>
		DefragmentationAllocatorPointer<T> allocateObjectsAligned(size_t amountOfObjects = 1, arguments&&... args)
		{
			//UNTESTED
			static_assert(ALIGNMENT <= 128, "Max alignment of 128 was exceeded");
			static_assert(std::is_copy_constructible<T>::value || std::is_move_constructible<T>::value, "Type must be copy or move constructible!");
			for (size_t i = 0; i < m_freeChunks.getLength(); i++)
			{
				T* data = m_freeChunks[i].allocateObject<T, ALIGNMENT>(amountOfObjects, std::forward<arguments>(args)...);
				if (data != nullptr)
				{
					if (m_freeChunks[i].m_length == 0)
					{
						m_freeChunks.removeIndex(i);
					}
					if (m_unusedHandleStack.hasDataLeft() == false)
					{
						debugBreak();
						throw AllocatorOutOfHandlesException();
					}
					size_t index = m_unusedHandleStack.pop();
					m_handleTable[index] = data;
					m_allocatedBlocks.add(DefragmentationAllocatorRelocatable(this, index, amountOfObjects, data));
					return DefragmentationAllocatorPointer<T>(this, index, amountOfObjects);
				}
			}

			debugBreak();
			throw AllocatorOutOfMemoryException();
		}

		template <typename T, typename... arguments>
		DefragmentationAllocatorPointer<T> allocateObjects(size_t amountOfObjects = 1, arguments&&... args)
		{
			return allocateObjectsAligned<T, alignof(T), arguments&&...>(amountOfObjects, std::forward<arguments>(args)...);
		}

		template <typename T, typename... arguments>
		DefragmentationAllocatorPointer<T> allocateObject(arguments&&... args)
		{
			//UNTESTED
			return allocateObjects<T>(1, std::forward<arguments>(args)...);
		}

		template <typename T, typename... arguments>
		UniquePointer<T, DefragmentationAllocatorDestroyer<T>> allocateObjectsUniquePointer(size_t amountOfObjects = 1, arguments&&... args)
		{
			auto pointer = allocateObjects<T>(amountOfObjects, std::forward<arguments>(args)...);
			return UniquePointer<T, DefragmentationAllocatorDestroyer<T>>(pointer.m_pdata, DefragmentationAllocatorDestroyer<T>(this, pointer));
		}

		template <typename T, typename... arguments>
		UniquePointer<T, DefragmentationAllocatorDestroyer<T>> allocateObjectUniquePointer(arguments&&... args)
		{
			return allocateObjectsUniquePointer<T>(1, std::forward<arguments>(args)...);
		}

		template<typename T>
		void deallocate(DefragmentationAllocatorPointer<T> &pointer)
		{
			if (pointer.m_pparent != this)
			{
				debugBreak();
				throw NullptrDeallocationException();
			}

			//UNTESTED
			for (size_t i = 0; i < pointer.m_length; i++)
			{
				bbe::addressOf(pointer[(int)i])->~T();
			}

			byte* bytePointer = reinterpret_cast<byte*>(m_handleTable[pointer.m_handleIndex]);
			size_t amountOfBytes = sizeof(T) * pointer.m_length;
			byte offset = bytePointer[-1];

			INTERNAL::GeneralPurposeAllocatorFreeChunk gpafc(bytePointer - offset, amountOfBytes + offset);

			INTERNAL::GeneralPurposeAllocatorFreeChunk* p_gpafc = &gpafc;
			INTERNAL::GeneralPurposeAllocatorFreeChunk* left;
			bool didTouchLeft = false;
			bool didMerge = false;
			INTERNAL::GeneralPurposeAllocatorFreeChunk* right;

			m_freeChunks.getNeighbors(*p_gpafc, left, right);
			if (left != nullptr)
			{
				if (left->touches(*p_gpafc))
				{
					left->m_length += p_gpafc->m_length;
					didTouchLeft = true;
					p_gpafc = left;
					didMerge = true;
				}
			}
			if (right != nullptr)
			{
				if (right->touches(*p_gpafc))
				{
					if (didTouchLeft)
					{
						p_gpafc->m_length += right->m_length;
						m_freeChunks.removeSingle(*right);
					}
					else
					{
						right->m_length += p_gpafc->m_length;
						right->m_addr = p_gpafc->m_addr;
					}
					didMerge = true;
				}
			}

			if (!didMerge)
			{
				m_freeChunks.add(gpafc);
			}

			m_unusedHandleStack.push(pointer.m_handleIndex);
			Empty e;
			if (!m_allocatedBlocks.removeSingle(DefragmentationAllocatorRelocatable(this, pointer.m_handleIndex, 0, &e)))
			{
				//If this is triggered, an allocated Block could not get removed!
				//This should never happen, when a well behaved pointer was passed to this method
				debugBreak();
				throw MalformedPointerException();
			}
			pointer.m_handleIndex = 0;
		}

		bool needsDefragmentation()
		{
			//UNTESTED
			if (
				(
					m_freeChunks.getLength() == 1
					&& (m_freeChunks[0].m_addr + m_freeChunks[0].m_length) == (m_data + m_length)
				) 
				|| m_freeChunks.getLength() == 0)
			{
				return false;
			}
			return true;
		}

		bool defragment()
		{
			//UNTESTED
			if (!needsDefragmentation())
			{
				return false;
			}

			byte* addr = m_freeChunks[0].m_addr;
			DefragmentationAllocatorRelocatable *left;
			DefragmentationAllocatorRelocatable *right;
			DefragmentationAllocatorRelocatable indexLocator(this, addr);
			m_allocatedBlocks.getNeighbors(indexLocator, left, right);
			byte oldOffset = static_cast<byte*>(right->getAddr())[-1];
			byte* newAddr = (*right)(addr);
			byte newOffset = static_cast<byte*>(right->getAddr())[-1];

			m_freeChunks[0].m_addr = newAddr;
			if (newOffset != oldOffset)
			{
				m_freeChunks[0].m_length += oldOffset;
				m_freeChunks[0].m_length -= newOffset;
			}

			if (m_freeChunks.getLength() > 1)
			{
				if (m_freeChunks[0].touches(m_freeChunks[1]))
				{
					m_freeChunks[0].m_length += m_freeChunks[1].m_length;
					m_freeChunks.removeIndex(1);
				}
			}
			
			
			return true;
		}
	};
}
