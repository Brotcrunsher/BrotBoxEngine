#pragma once

#include "../BBE/Error.h"
#include <memory>

namespace bbe
{
	/// Produces a value asynchronously (e.g. GPU query). Owned by \c Future via \c std::shared_ptr — must be heap-managed.
	template<typename T>
	class DataProvider
	{
	public:
		virtual ~DataProvider() = default;
		virtual bool isValueReady() const = 0;
		virtual T getValue() const = 0;
	};

	/// Shared ownership of a \c DataProvider. Not thread-safe unless the provider itself is.
	template<typename T>
	class Future
	{
	private:
		std::shared_ptr<DataProvider<T>> m_provider;

	public:
		Future() = default;

		explicit Future(std::shared_ptr<DataProvider<T>> dataProvider)
			: m_provider(std::move(dataProvider))
		{
		}

		bool isValueReady() const
		{
			return m_provider != nullptr && m_provider->isValueReady();
		}

		T getValue() const
		{
			if (!m_provider || !m_provider->isValueReady())
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
			return m_provider->getValue();
		}
	};
}
