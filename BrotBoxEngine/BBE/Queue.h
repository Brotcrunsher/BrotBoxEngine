#pragma once

#include "../BBE/List.h"

namespace bbe
{
	template <typename T>
	class Queue
	{
	private:
		bbe::List<T> data;
		size_t readPointer = 0;
		size_t writePointer = 0;
		size_t length = 0;

	public:
		Queue()
		{
			data.resizeCapacityAndLength(4);
		}

		void add(const T& t)
		{
			if (length == data.getLength())
			{
				bbe::List<T> newData;
				newData.resizeCapacityAndLength(length * 2);
				for (size_t i = 0; i < length; i++)
				{
					newData[i] = data[(readPointer + i) % length];
				}
				readPointer = 0;
				writePointer = length;
				data = newData;
			}
			data[writePointer] = t;
			writePointer++;
			if(writePointer == data.getLength()) writePointer = 0;
			length++;
		}

		T pop()
		{
			if (isEmpty()) bbe::Crash(bbe::Error::IllegalState);
			T retVal = data[readPointer];
			readPointer++;
			if (readPointer == data.getLength()) readPointer = 0;
			length--;
			return retVal;
		}

		const T& peek()
		{
			if (isEmpty()) bbe::Crash(bbe::Error::IllegalState);
			return data[readPointer];
		}

		bool isEmpty() const
		{
			return length == 0;
		}

		size_t getLength() const
		{
			return length;
		}
	};
}
