#pragma once

#include "../BBE/List.h"
#include "../BBE/Hash.h"
#include "../BBE/Exceptions.h"

namespace bbe
{


	//TODO use own allocators
	template<typename Key, typename Value>
	class HashMap
	{
	private:
		class HashMapNode
		{
			friend class HashMap<Key, Value>;
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
		List<HashMapNode> *m_pcontainers = nullptr;
		size_t             m_amountOfContainers = 0;

	public:
		HashMap()
			: m_amountOfContainers(1 << 4)
		{
			m_pcontainers = new List<HashMapNode>[m_amountOfContainers];
		}

		HashMap(const HashMap& hm)
		{
			//UNTESTED
			m_amountOfContainers = hm.m_amountOfContainers;
			m_pcontainers = new List<Value>[m_amountOfContainers];
			for (size_t i = 0; i < m_amountOfContainers; i++)
			{
				for (size_t k = 0; k < hm.m_pcontainers[i].getLength(); k++)
				{
					m_pcontainers[i].add(hm.m_pcontainers[i][k]);
				}
			}
		}

		HashMap(HashMap&& hm)
		{
			//UNTESTED
			m_pcontainers = hm.m_pcontainers;
			m_amountOfContainers = hm.m_pcontainers;
			
			hm.m_pcontainers = nullptr;
			hm.m_amountOfContainers = 0;
		}

		HashMap& operator=(const HashMap& hm)
		{
			//UNTESTED
			if (this == &hm)
			{
				return *this;
			}
			if (m_pcontainers != nullptr)
			{
				delete[] m_pcontainers;
			}

			m_amountOfContainers = hm.m_amountOfContainers;
			m_pcontainers = new List<Value>[m_amountOfContainers];
			for (size_t i = 0; i < m_amountOfContainers; i++)
			{
				for (size_t k = 0; k < hm.m_pcontainers[i].getLength(); k++)
				{
					m_pcontainers[i].add(hm.m_pcontainers[i][k]);
				}
			}

			return *this;
		}

		HashMap& operator=(HashMap&& hm)
		{
			//UNTESTED
			if (this == &hm)
			{
				return *this;
			}
			if (m_pcontainers != nullptr)
			{
				delete[] m_pcontainers;
			}

			m_pcontainers = hm.m_pcontainers;
			m_amountOfContainers = hm.m_pcontainers;

			hm.m_pcontainers = nullptr;
			hm.m_amountOfContainers = 0;

			return *this;
		}

		~HashMap()
		{
			del();
		}

		void del()
		{
			if (m_pcontainers != nullptr)
			{
				delete[] m_pcontainers;
			}

			m_amountOfContainers = 0;
			m_pcontainers = nullptr;
		}

		void clear()
		{
			del();

			m_amountOfContainers = 1 << 4;
			m_pcontainers = new List<HashMapNode>[m_amountOfContainers];
		}

		void add(const Key &key, const Value &value)
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash & (m_amountOfContainers - 1);

			if (m_pcontainers[index].getLength() > 4)
			{
				uint32_t differentHashes = 0;
				for (size_t i = 0; i < m_pcontainers[index].getLength(); i++)
				{
					uint32_t otherHash = hash(m_pcontainers[index][i].m_key);
					if (otherHash != _hash)
					{
						differentHashes++;
						if (differentHashes >= 3)
						{
							resize();
							index = _hash & (m_amountOfContainers - 1);
							break;
						}
					}
				}
			}

			for (size_t i = 0; i < m_pcontainers[index].getLength(); i++)
			{
				if (key == m_pcontainers[index][i].m_key)
				{
					throw KeyAlreadyUsedException();
				}
			}

			m_pcontainers[index].add(HashMapNode(key, value, _hash));
		}

		bool contains(const Key &key) const
		{
			return get(key) != nullptr;
		}

		Value* get(const Key &key) const
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash & (m_amountOfContainers - 1);

			for (size_t i = 0; i < m_pcontainers[index].getLength(); i++)
			{
				if (m_pcontainers[index][i].m_key == key)
				{
					return &(m_pcontainers[index][i].m_value);
				}
			}
			return nullptr;
		}

		bool remove(const Key &key)
		{
			//TODO
			//UNTESTED
			return false;
		}

	private:
		void resize()
		{
			size_t newAmountOfContainers = m_amountOfContainers << 1;
			List<HashMapNode> *newContainers = new List<HashMapNode>[newAmountOfContainers];

			for (size_t i = 0; i < m_amountOfContainers; i++)
			{
				for (size_t k = 0; k < m_pcontainers[i].getLength(); k++)
				{
					uint32_t _hash = m_pcontainers[i][k].m_hash;
					uint32_t index = _hash & (newAmountOfContainers - 1);

					newContainers[index].add(std::move(m_pcontainers[i][k]));
				}
			}

			delete[] m_pcontainers;
			m_pcontainers = newContainers;
			m_amountOfContainers = newAmountOfContainers;
		}
	};
}
