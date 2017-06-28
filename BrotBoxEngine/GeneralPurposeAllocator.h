#pragma once

#include "DataType.h"
#include "List.h"
#include "UtilMath.h"
#include "UniquePointer.h"
#include "UtilTest.h"

namespace bbe
{
	namespace INTERNAL
	{
		class GeneralPurposeAllocatorFreeChunk
		{
		public:
			byte* m_addr;
			size_t m_size;

			GeneralPurposeAllocatorFreeChunk(byte* addr, size_t size)
				: m_addr(addr), m_size(size)
			{
				//do nothing
			}

			bool touches(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				//UNTESTED
				if (m_addr + m_size == other.m_addr)
				{
					return true;
				}
				if (other.m_addr + other.m_size == m_addr)
				{
					return true;
				}

				return false;
			}

			bool operator>(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				return m_addr > other.m_addr;
			}

			bool operator>=(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				return m_addr >= other.m_addr;
			}

			bool operator<(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				return m_addr < other.m_addr;
			}

			bool operator<=(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				return m_addr <= other.m_addr;
			}

			bool operator==(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				return m_addr == other.m_addr;
			}

			template <typename T, typename... arguments>
			T* allocateObject(size_t amountOfObjects = 1, arguments&&... args)
			{
				//UNTESTED
				static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded");
				byte* allocationLocation = (byte*)nextMultiple(alignof(T), ((size_t)m_addr) + 1);
				size_t amountOfBytes = amountOfObjects * sizeof(T);
				byte* newAddr = allocationLocation + amountOfBytes;
				if (newAddr <= m_addr + m_size)
				{
					byte offset = (byte)(allocationLocation - m_addr);
					allocationLocation[-1] = offset;
					T* returnPointer = reinterpret_cast<T*>(allocationLocation);
					m_size -= newAddr - m_addr;
					m_addr = newAddr;
					for (size_t i = 0; i < amountOfObjects; i++)
					{
						T* object = bbe::addressOf(returnPointer[i]);
						new (object) T(std::forward<arguments>(args)...);
					}
					return returnPointer;
				}
				else
				{
					return nullptr;
				}
			}
		};
	}
	

	class GeneralPurposeAllocator
	{
		//TODO use parent allocator
		//TODO defragmentation
	public:
		template<typename T>
		class GeneralPurposeAllocatorPointer
		{
			friend class GeneralPurposeAllocator;
		private:
			T* m_pdata;
			size_t m_size;
		public:
			GeneralPurposeAllocatorPointer(T* pdata, size_t size)
				: m_pdata(pdata), m_size(size)
			{
				//do nothing
			}

			T* operator ->()
			{
				return m_pdata;
			}

			const T* operator ->() const
			{
				//UNTESTED
				return m_pdata;
			}

			T& operator *()
			{
				//UNTESTED
				return *m_pdata;
			}

			const T& operator *() const
			{
				//UNTESTED
				return *m_pdata;
			}

			T& operator [](int index)
			{
				return *(m_pdata + index);
			}

			const T& operator [](int index) const
			{
				return *(m_pdata + index);
			}

			T* getRaw()
			{
				return m_pdata;
			}

			bool operator ==(void* ptr) const
			{
				return m_pdata == ptr;
			}

			bool operator !=(void* ptr) const
			{
				return m_pdata != ptr;
			}
		};

		template<typename T>
		class GeneralPurposeAllocatorDestroyer
		{
		private:
			GeneralPurposeAllocator* m_pa;
			GeneralPurposeAllocatorPointer<T> m_data;
		public:
			GeneralPurposeAllocatorDestroyer(GeneralPurposeAllocator *pa, GeneralPurposeAllocatorPointer<T> data)
				: m_pa(pa), m_data(data)
			{
				//do nothing
			}

			void destroy(void* data)
			{
				m_pa->deallocateObjects(m_data);
			}
		};
	private:
		static const size_t GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE = 1024;
		byte* m_data;
		size_t m_size;

		List<INTERNAL::GeneralPurposeAllocatorFreeChunk, true> m_freeChunks;

	public:
		explicit GeneralPurposeAllocator(size_t size = GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE)
			: m_size(size)
		{
			//UNTESTED
			m_data = new byte[m_size];
			m_freeChunks.pushBack(INTERNAL::GeneralPurposeAllocatorFreeChunk(m_data, m_size));
		}

		~GeneralPurposeAllocator()
		{
			if (m_freeChunks.getLength() != 1)
			{
				debugBreak();
			}
			if (m_freeChunks[0].m_addr != m_data)
			{
				debugBreak();
			}
			if (m_freeChunks[0].m_size != m_size)
			{
				debugBreak();
			}
			if (m_data != nullptr)
			{
				delete[] m_data;
				m_data = nullptr;
			}
		}

		GeneralPurposeAllocator(const GeneralPurposeAllocator& other) = delete;
		GeneralPurposeAllocator(GeneralPurposeAllocator&& other) = delete;
		GeneralPurposeAllocator& operator=(const GeneralPurposeAllocator& other) = delete;
		GeneralPurposeAllocator& operator=(GeneralPurposeAllocator&& other) = delete;

		template <typename T, typename... arguments>
		GeneralPurposeAllocatorPointer<T> allocateObjects(size_t amountOfObjects = 1, arguments&&... args)
		{
			//UNTESTED
			static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded");
			for (size_t i = 0; i < m_freeChunks.getLength(); i++)
			{
				T* data = m_freeChunks[i].allocateObject<T>(amountOfObjects, std::forward<arguments>(args)...);
				if (data != nullptr)
				{
					if (m_freeChunks[i].m_size == 0)
					{
						m_freeChunks.removeIndex(i);
					}
					return GeneralPurposeAllocatorPointer<T>(data, amountOfObjects);
				}
			}

			//TODO add further error handling
			debugBreak();
			return GeneralPurposeAllocatorPointer<T>(nullptr, 0);
		}

		template <typename T, typename... arguments>
		GeneralPurposeAllocatorPointer<T> allocateObject(arguments&&... args)
		{
			//UNTESTED
			return allocateObjects<T>(1, std::forward<arguments>(args)...);
		}

		template <typename T, typename... arguments>
		UniquePointer<T, GeneralPurposeAllocatorDestroyer<T>> allocateObjectsUniquePointer(size_t amountOfObjects = 1, arguments&&... args)
		{
			auto pointer = allocateObjects<T>(amountOfObjects, std::forward<arguments>(args)...);
			return UniquePointer<T, GeneralPurposeAllocatorDestroyer<T>>(pointer.m_pdata, GeneralPurposeAllocatorDestroyer<T>(this, pointer));
		}

		template <typename T, typename... arguments>
		UniquePointer<T, GeneralPurposeAllocatorDestroyer<T>> allocateObjectUniquePointer(arguments&&... args)
		{
			return allocateObjectsUniquePointer<T>(1, std::forward<arguments>(args)...);
		}

		template<typename T>
		void deallocateObjects(GeneralPurposeAllocatorPointer<T> &pointer)
		{
			//UNTESTED
			for (size_t i = 0; i < pointer.m_size; i++)
			{
				bbe::addressOf(pointer.m_pdata[i])->~T();
			}

			byte* bytePointer = reinterpret_cast<byte*>(pointer.m_pdata);
			size_t amountOfBytes = sizeof(T) * pointer.m_size;
			byte offset = bytePointer[-1];

			//TODO add this to the freeChunks list
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
					left->m_size += p_gpafc->m_size;
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
						p_gpafc->m_size += right->m_size;
						m_freeChunks.removeSingle(*right);
					}
					else
					{
						right->m_size += p_gpafc->m_size;
						right->m_addr = p_gpafc->m_addr;
					}
					didMerge = true;
				}
			}

			if (!didMerge)
			{
				m_freeChunks.pushBack(gpafc);
			}

			pointer.m_pdata = nullptr;
		}
	};
}