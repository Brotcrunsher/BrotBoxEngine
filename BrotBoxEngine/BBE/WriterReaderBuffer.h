#pragma once
#include <atomic>
#include "../BBE/Error.h"

namespace bbe
{
	template<typename T, size_t N>	class WriterReaderBuffer;

	template<typename T, size_t N>
	class ReaderAccess
	{
		friend class WriterReaderBuffer<T, N>;
	private:
		WriterReaderBuffer<T, N>& buffer;
		size_t readHead;

		explicit ReaderAccess(WriterReaderBuffer<T, N>& buffer) : buffer(buffer), readHead(buffer.writeHead.load()) {};

	public:
		bool hasNext()
		{
			return readHead != buffer.writeHead.load();
		}

		const T& next()
		{
			if (!hasNext()) bbe::Crash(bbe::Error::IllegalState);

			const T& retVal = buffer.data[readHead];
			readHead++;
			if (readHead >= N) readHead = 0;
			return retVal;
		}
	};

	template<typename T, size_t N>
	class WriterReaderBuffer
	{
		friend class ReaderAccess<T, N>;
	private:
		T data[N] = {};
		std::atomic<size_t> writeHead;

		void incWriteHead()
		{
			size_t val = writeHead.load();
			val++;
			if (val >= N) val = 0;
			writeHead = val;
		}

	public:
		void add(T&& t)
		{
			data[writeHead.load()] = std::move(t);
			incWriteHead();
		}
		void add(const T& t)
		{
			data[writeHead.load()] = t;
			incWriteHead();
		}

		ReaderAccess<T, N> reader()
		{
			return ReaderAccess<T, N>(*this);
		}
	};
}
