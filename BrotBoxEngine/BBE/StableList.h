#pragma once

#include <bitset>
#include "../BBE/AllocBlock.h"
#include "../BBE/Error.h"

namespace bbe
{
	// Stable = Pointer Stable. Adding and removing elements from the list does not invalidate pointers.
	template<typename T, size_t blockSize = 64>
	class StableList
	{
	private:
		struct Block
		{
			std::bitset<blockSize> m_used;
			bbe::AllocBlock m_data;

			Block* nextBlock = nullptr;
			Block* previousBlock = nullptr;

			T* get(size_t i)
			{
				return reinterpret_cast<T*>(m_data.data) + i;
			}

			const T* get(size_t i) const
			{
				return reinterpret_cast<const T*>(m_data.data) + i;
			}

			Block() : m_data(bbe::allocateBlock(sizeof(T)* blockSize)) {}

			~Block()
			{
				clear();
				bbe::freeBlock(m_data);
			}

			Block(const Block& other)
			{
				m_used = other.m_used;
				m_data = bbe::allocateBlock(sizeof(T) * blockSize);
				for (size_t i = 0; i < blockSize; i++)
				{
					if (m_used[i])
					{
						new (get(i)) T(*other.get(i));
					}
				}
			}

			Block(Block&& other) noexcept : 
				m_used(std::move(other.m_used)),
				m_data(std::move(other.m_data))
			{
				other.m_used = {};
				other.m_data = {};
			}

			Block& operator=(const Block& other)
			{
				if (this == &other) return *this;
				clear();
				m_used = other.m_used;
				for (size_t i = 0; i < blockSize; i++)
				{
					if (m_used[i])
					{
						new (get(i)) T(*other.get(i));
					}
				}
				return *this;
			}

			Block& operator=(Block&& other)
			{
				if (this == &other) return *this;
				clear();
				bbe::freeBlock(m_data);

				m_used = other.m_used;
				m_data = other.m_data;

				other.m_used = {};
				other.m_data = {};
				return *this;
			}

			void clear()
			{
				for (size_t i = 0; i < blockSize; i++)
				{
					if (m_used[i])
					{
						get(i)->~T();
					}
				}
				m_used = {};
			}

			int32_t getAddLocation() noexcept
			{
				if (m_used[blockSize - 1]) return -1; // Block might not be full, but the last element is blocked. We always want to add at the end of the list, never before that.

				for (int32_t i = blockSize - 2; i >= 0; i--)
				{
					if (m_used[i]) return i + 1;
				}

				return 0;
			}
		};

		Block* m_firstBlock = nullptr;
		Block* m_lastBlock = nullptr;

		void addNewBlock()
		{
			Block* newBlock = new Block();
			if (!m_firstBlock)
			{
				m_firstBlock = newBlock;
				m_lastBlock = newBlock;
			}
			else
			{
				m_lastBlock->nextBlock = newBlock;
				newBlock->previousBlock = m_lastBlock;
				m_lastBlock = newBlock;
			}
		}

		void copy(const StableList& other)
		{
			Block* nextBlock = other.m_firstBlock;
			while (nextBlock)
			{
				addNewBlock();
				*m_lastBlock = *nextBlock;

				nextBlock = nextBlock->nextBlock;
			}
		}

		int32_t getAddLocation()
		{
			if (!m_lastBlock)
			{
				addNewBlock();
				return 0;
			}
			const int32_t addLocation = m_lastBlock->getAddLocation();
			if (addLocation == -1)
			{
				addNewBlock();
				return 0;
			}
			return addLocation;
		}

	public:
		class Iterator
		{
			friend class StableList;

		public:
			Iterator() : list(nullptr), currentBlock(nullptr), index(0) {}
			Iterator(StableList* list, Block* block, size_t idx) : list(list), currentBlock(block), index(idx) { advanceToValid(); }

			T& operator*() const
			{
				return *(currentBlock->get(index));
			}

			T* operator->() const
			{
				return currentBlock->get(index);
			}

			Iterator& operator++()
			{
				++index;
				advanceToValid();
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator temp = *this;
				++(*this);
				return temp;
			}

			bool operator==(const Iterator& other) const
			{
				return list == other.list && currentBlock == other.currentBlock && index == other.index;
			}

			bool operator!=(const Iterator& other) const
			{
				return !(*this == other);
			}

			void remove()
			{
				if (!currentBlock || index >= blockSize || !currentBlock->m_used[index])
				{
					bbe::Crash(bbe::Error::IllegalState);
					return;
				}

				currentBlock->get(index)->~T();
				currentBlock->m_used.reset(index);

				if (currentBlock->m_used.any())
				{
					++(*this);
				}
				else
				{
					if (currentBlock->nextBlock)
					{
						currentBlock->nextBlock->previousBlock = currentBlock->previousBlock;
					}
					if (currentBlock->previousBlock)
					{
						currentBlock->previousBlock->nextBlock = currentBlock->nextBlock;
					}

					if (list->m_firstBlock == currentBlock)
					{
						list->m_firstBlock = currentBlock->nextBlock;
					}
					if (list->m_lastBlock == currentBlock)
					{
						list->m_lastBlock = currentBlock->previousBlock;
					}

					Block* deleteBlock = currentBlock;
					currentBlock = currentBlock->nextBlock;
					delete deleteBlock;
					index = 0;
					if (currentBlock && !currentBlock->m_used[0])
					{
						++(*this);
					}
				}

			}

		private:
			StableList* list = nullptr;
			Block* currentBlock = nullptr;
			size_t index = 0;

			void advanceToValid()
			{
				while (currentBlock)
				{
					while (index < blockSize && !currentBlock->m_used[index])
					{
						++index;
					}
					if (index < blockSize)
					{
						break;
					}
					currentBlock = currentBlock->nextBlock;
					index = 0;
				}
			}
		};

		class ConstIterator
		{
			friend class StableList;

		public:
			ConstIterator() : currentBlock(nullptr), index(0) {}
			ConstIterator(const Block* block, size_t idx) : currentBlock(block), index(idx) { advanceToValid(); }
			ConstIterator(const Iterator& it) : currentBlock(it.currentBlock), index(it.index) { }

			const T& operator*() const
			{
				return *(currentBlock->get(index));
			}

			const T* operator->() const
			{
				return currentBlock->get(index);
			}

			ConstIterator& operator++()
			{
				++index;
				advanceToValid();
				return *this;
			}

			ConstIterator operator++(int)
			{
				ConstIterator temp = *this;
				++(*this);
				return temp;
			}

			bool operator==(const ConstIterator& other) const
			{
				return currentBlock == other.currentBlock && index == other.index;
			}

			bool operator!=(const ConstIterator& other) const
			{
				return !(*this == other);
			}

		private:
			const Block* currentBlock = nullptr;
			size_t index = 0;

			void advanceToValid()
			{
				while (currentBlock)
				{
					while (index < blockSize && !currentBlock->m_used[index])
					{
						++index;
					}
					if (index < blockSize)
					{
						break;
					}
					currentBlock = currentBlock->nextBlock;
					index = 0;
				}
			}
		};

		Iterator begin()
		{
			return Iterator(this, m_firstBlock, 0);
		}

		Iterator end()
		{
			return Iterator(this, nullptr, 0);
		}

		ConstIterator begin() const
		{
			return ConstIterator(m_firstBlock, 0);
		}

		ConstIterator end() const
		{
			return ConstIterator(nullptr, 0);
		}

		ConstIterator cbegin() const
		{
			return ConstIterator(m_firstBlock, 0);
		}

		ConstIterator cend() const
		{
			return ConstIterator(nullptr, 0);
		}

		StableList() = default;

		StableList(const StableList& other)
		{
			copy(other);
		}

		StableList(StableList&& other) noexcept
		{
			m_firstBlock = other.m_firstBlock;
			m_lastBlock = other.m_lastBlock;
			other.m_firstBlock = nullptr;
			other.m_lastBlock = nullptr;
		}

		StableList& operator=(const StableList& other)
		{
			if (this == &other) return *this;
			clear();
			copy(other);
			return *this;
		}

		StableList& operator=(StableList&& other) noexcept
		{
			if (this == &other) return *this;
			clear();
			m_firstBlock = other.m_firstBlock;
			m_lastBlock = other.m_lastBlock;
			other.m_firstBlock = nullptr;
			other.m_lastBlock = nullptr;
			return *this;
		}

		~StableList()
		{
			clear();
		}

		void clear() noexcept
		{
			Block* nextBlock = m_firstBlock;
			while (nextBlock)
			{
				Block* curBlock = nextBlock;
				nextBlock = nextBlock->nextBlock;
				delete curBlock;
			}
			m_firstBlock = nullptr;
			m_lastBlock = nullptr;
		}

		void add(const T& t)
		{
			const int32_t location = getAddLocation();
			new (m_lastBlock->get(location)) T(t);
			m_lastBlock->m_used.set(location);
		}

		void add(T&& t)
		{
			const int32_t location = getAddLocation();
			new (m_lastBlock->get(location)) T(std::move(t));
			m_lastBlock->m_used.set(location);
		}

		size_t getAmountOfBlocks() const noexcept
		{
			Block* currentBlock = this->m_firstBlock;
			size_t retVal = 0;
			while (currentBlock)
			{
				currentBlock = currentBlock->nextBlock;
				retVal++;
			}
			return retVal;
		}
	};
}
