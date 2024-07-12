#pragma once

#include "../BBE/List.h"
#include "../BBE/Hash.h"
#include "../BBE/Error.h"

namespace bbe
{
	template<typename Key, typename Value>
	class HashMap
	{
	private:
		class HashMapNode
		{
			friend class HashMap<Key, Value>;
		public:
			HashMapNode() = default;
		private:
			Key      m_key;
			Value    m_value;
			uint32_t m_hash;


			HashMapNode(const Key &key, const Value &value, const uint32_t _hash)
				: m_key(key), m_value(value), m_hash(_hash)
			{
				//do nothing
			}
		};
		List<List<HashMapNode>> m_buckets;

	public:
		HashMap()
		{
			m_buckets.resizeCapacityAndLength(1 << 4);
		}

		HashMap(const HashMap&)                = default;
		HashMap(HashMap&&) noexcept            = default;
		HashMap& operator=(const HashMap&)     = default;
		HashMap& operator=(HashMap&&) noexcept = default;

		void clear()
		{
			m_buckets.clear();
			m_buckets.resizeCapacityAndLength(1 << 4);
		}

		size_t getAmountOfBuckets() const
		{
			return m_buckets.getLength();
		}

		void add(const Key &key, const Value &value)
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash % getAmountOfBuckets();

			for (size_t i = 0; i < m_buckets[index].getLength(); i++)
			{
				if (key == m_buckets[index][i].m_key)
				{
					bbe::Crash(bbe::Error::KeyAlreadyUsed);
				}
			}

			if (m_buckets[index].getLength() > 4)
			{
				uint32_t differentHashes = 0;
				for (size_t i = 0; i < m_buckets[index].getLength(); i++)
				{
					if (m_buckets[index][i].m_hash != _hash)
					{
						differentHashes++;
						if (differentHashes >= 3)
						{
							resize();
							index = _hash % getAmountOfBuckets();
							break;
						}
					}
				}
			}

			m_buckets[index].add(HashMapNode(key, value, _hash));
		}

		bool contains(const Key &key) const
		{
			return get(key) != nullptr;
		}

		const Value* get(const Key &key) const
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash % getAmountOfBuckets();

			for (size_t i = 0; i < m_buckets[index].getLength(); i++)
			{
				if (m_buckets[index][i].m_key == key)
				{
					return &(m_buckets[index][i].m_value);
				}
			}
			return nullptr;
		}

		Value* get(const Key& key)
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash % getAmountOfBuckets();

			for (size_t i = 0; i < m_buckets[index].getLength(); i++)
			{
				if (m_buckets[index][i].m_key == key)
				{
					return &(m_buckets[index][i].m_value);
				}
			}
			return nullptr;
		}

	private:
		void resize()
		{
			const size_t newAmountOfContainers = getAmountOfBuckets() * 2;
			List<List<HashMapNode>> newBuckets;
			newBuckets.resizeCapacityAndLength(newAmountOfContainers);

			for (size_t i = 0; i < m_buckets.getLength(); i++)
			{
				for (size_t k = 0; k < m_buckets[i].getLength(); k++)
				{
					uint32_t _hash = m_buckets[i][k].m_hash;
					uint32_t index = _hash % newAmountOfContainers;

					newBuckets[index].add(std::move(m_buckets[i][k]));
				}
			}

			m_buckets = newBuckets;
		}
	};
}
