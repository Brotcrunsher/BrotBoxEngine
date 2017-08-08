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
			Key key;
			Value value;
			uint32_t _hash;

			HashMapNode(const Key &key, const Value &value, const uint32_t _hash)
				: key(key), value(value), _hash(_hash)
			{
				//do nothing
			}
		};
		List<HashMapNode>* m_containers = nullptr;
		size_t m_amountOfContainers = 0;

	public:
		HashMap()
			: m_amountOfContainers(1 << 4)
		{
			m_containers = new List<HashMapNode>[m_amountOfContainers];
		}

		HashMap(const HashMap& hm)
		{
			//UNTESTED
			m_amountOfContainers = hm.m_amountOfContainers;
			m_containers = new List<Value>[m_amountOfContainers];
			for (size_t i = 0; i < m_amountOfContainers; i++)
			{
				for (size_t k = 0; k < hm.m_containers[i].getLength(); k++)
				{
					m_containers[i].add(hm.m_containers[i][k]);
				}
			}
		}

		HashMap(HashMap&& hm)
		{
			//UNTESTED
			m_containers = hm.m_containers;
			m_amountOfContainers = hm.m_containers;
			
			hm.m_containers = nullptr;
			hm.m_amountOfContainers = 0;
		}

		HashMap& operator=(const HashMap& hm)
		{
			//UNTESTED
			if (m_containers != nullptr)
			{
				delete[] m_containers;
			}

			m_amountOfContainers = hm.m_amountOfContainers;
			m_containers = new List<Value>[m_amountOfContainers];
			for (size_t i = 0; i < m_amountOfContainers; i++)
			{
				for (size_t k = 0; k < hm.m_containers[i].getLength(); k++)
				{
					m_containers[i].add(hm.m_containers[i][k]);
				}
			}
		}

		HashMap& operator=(HashMap&& hm)
		{
			//UNTESTED
			if (m_containers != nullptr)
			{
				delete[] m_containers;
			}

			m_containers = hm.m_containers;
			m_amountOfContainers = hm.m_containers;

			hm.m_containers = nullptr;
			hm.m_amountOfContainers = 0;
		}

		~HashMap()
		{
			if (m_containers != nullptr)
			{
				delete[] m_containers;
			}
			m_containers = nullptr;
			m_amountOfContainers = 0;
		}

		void add(const Key &key, const Value &value)
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash & (m_amountOfContainers - 1);

			if (m_containers[index].getLength() > 4)
			{
				uint32_t differentHashes = 0;
				for (size_t i = 0; i < m_containers[index].getLength(); i++)
				{
					uint32_t otherHash = hash(m_containers[index][i].key);
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

			for (size_t i = 0; i < m_containers[index].getLength(); i++)
			{
				if (key == m_containers[index][i].key)
				{
					throw KeyAlreadyUsedException();
				}
			}

			m_containers[index].add(HashMapNode(key, value, _hash));
		}

		Value* get(const Key &key)
		{
			uint32_t _hash = hash(key);
			uint32_t index = _hash & (m_amountOfContainers - 1);

			for (size_t i = 0; i < m_containers[index].getLength(); i++)
			{
				if (m_containers[index][i].key == key)
				{
					return &(m_containers[index][i].value);
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
				for (size_t k = 0; k < m_containers[i].getLength(); k++)
				{
					uint32_t _hash = m_containers[i][k]._hash;
					uint32_t index = _hash & (newAmountOfContainers - 1);

					newContainers[index].add(std::move(m_containers[i][k]));
				}
			}

			delete[] m_containers;
			m_containers = newContainers;
			m_amountOfContainers = newAmountOfContainers;
		}
	};
}
