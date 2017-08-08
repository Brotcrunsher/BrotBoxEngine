#pragma once

#include "../BBE/DataType.h"
#include "../BBE/List.h"
#include "../BBE/UtilMath.h"
#include "../BBE/UniquePointer.h"
#include "../BBE/UtilTest.h"
#include "../BBE/EmptyClass.h"
#include "../BBE/Exceptions.h"

namespace bbe
{
	namespace INTERNAL
	{
		class GeneralPurposeAllocatorFreeChunk
		{
		public:
			byte* m_addr;
			size_t m_length;

			GeneralPurposeAllocatorFreeChunk(byte* addr, size_t size)
				: m_addr(addr), m_length(size)
			{
				//do nothing
			}

			bool touches(const GeneralPurposeAllocatorFreeChunk& other) const
			{
				//UNTESTED
				if (m_addr + m_length == other.m_addr)
				{
					return true;
				}
				if (other.m_addr + other.m_length == m_addr)
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

			template <typename T, int ALIGNMENT, typename... arguments>
			T* allocateObject(size_t amountOfObjects = 1, arguments&&... args)
			{
				//UNTESTED
				static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded");
				static_assert(ALIGNMENT > 0, "Alignment must be positive, none zero.");
				byte* allocationLocation = (byte*)nextMultiple(alignof(T), ((size_t)m_addr) + 1);
				size_t amountOfBytes = amountOfObjects * sizeof(T);
				byte* newAddr = allocationLocation + amountOfBytes;
				if (newAddr <= m_addr + m_length)
				{
					byte offset = (byte)(allocationLocation - m_addr);
					allocationLocation[-1] = offset;
					T* returnPointer = reinterpret_cast<T*>(allocationLocation);
					m_length -= newAddr - m_addr;
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
			size_t m_length;
		public:
			GeneralPurposeAllocatorPointer(T* pdata, size_t size)
				: m_pdata(pdata), m_length(size)
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
				m_pa->deallocate(m_data);
			}
		};
	private:
		static const size_t GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE = 1024;
		byte* m_data;
		size_t m_length;

		List<INTERNAL::GeneralPurposeAllocatorFreeChunk, true> m_freeChunks;

	public:
		explicit GeneralPurposeAllocator(size_t size = GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE)
			: m_length(size)
		{
			//UNTESTED
			m_data = new byte[m_length];
			m_freeChunks.add(INTERNAL::GeneralPurposeAllocatorFreeChunk(m_data, m_length));
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
			if (m_freeChunks[0].m_length != m_length)
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
			return allocateObjectsAligned<T, alignof(T), arguments&&...>(amountOfObjects, std::forward<arguments>(args)...);
		}

		template <typename T, int ALIGNMENT, typename... arguments>
		GeneralPurposeAllocatorPointer<T> allocateObjectsAligned(size_t amountOfObjects = 1, arguments&&... args)
		{
			//UNTESTED
			static_assert(ALIGNMENT <= 128, "Max alignment of 128 was exceeded");
			static_assert(ALIGNMENT >= alignof(T), "Alignment must be at least the alignment of type T!");
			for (size_t i = 0; i < m_freeChunks.getLength(); i++)
			{
				T* data = m_freeChunks[i].allocateObject<T, ALIGNMENT>(amountOfObjects, std::forward<arguments>(args)...);
				if (data != nullptr)
				{
					if (m_freeChunks[i].m_length == 0)
					{
						m_freeChunks.removeIndex(i);
					}
					return GeneralPurposeAllocatorPointer<T>(data, amountOfObjects);
				}
			}

			debugBreak();
			throw AllocatorOutOfMemoryException();
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
		void deallocate(GeneralPurposeAllocatorPointer<T> &pointer)
		{
			//UNTESTED
			for (size_t i = 0; i < pointer.m_length; i++)
			{
				bbe::addressOf(pointer.m_pdata[i])->~T();
			}

			byte* bytePointer = reinterpret_cast<byte*>(pointer.m_pdata);
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

			pointer.m_pdata = nullptr;
		}
	};
}
