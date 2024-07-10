#pragma once

#include "../BBE/Error.h"
#include "../BBE/AutoRefCountable.h"

namespace bbe
{
	template <typename T>
	class DataProvider : public bbe::AutoRefCountable
	{
	public:
		~DataProvider() {}
		virtual bool isValueReady() const = 0;
		virtual T    getValue()     const = 0;
	};

	template<typename T>
	class Future
	{
	private:
		AutoRef ref;

	public:
		Future() {}
		Future(DataProvider<T>* dataProvider)
			: ref(dataProvider) // TODO: This is far from ideal. What if the dataProvider wasn't created with new?
			                    //       Right now the alternative would be to take an AutoRef as parameter, but then
			                    //       the API doesn't enforce that the value is actually a DataProvider. We need
			                    //       some AutoRef kinda Template that has type infomration about the underlying
			                    //       type.
		{
		}

		bool isValueReady() const
		{
			return ((const DataProvider<T>*)ref.get())->isValueReady();
		}

		T getValue() const
		{
			if (!isValueReady())
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
			return ((const DataProvider<T>*)ref.get())->getValue();
		}
	};
}
