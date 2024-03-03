#pragma once

#include "../BBE/List.h"
#include "../BBE/DataType.h"

namespace bbe
{
	class ByteBufferSpan
	{
	private:
		bbe::List<bbe::byte>& m_bytes; // List reference instead of byte* to ensure we are safe when the list resizes.
		size_t m_start = 0;
		size_t m_end = 0;
		bool m_didErr = false;

		void read(bbe::byte* bytes, bbe::byte* default_, size_t length);

	public:
		ByteBufferSpan(bbe::List<bbe::byte>& bytes);
		ByteBufferSpan(bbe::List<bbe::byte>& bytes, size_t start, size_t end);

		template<typename T>
		void read(T& val, T default_)
		{
			if constexpr (
				   std::is_same_v<T, int8_t>
				|| std::is_same_v<T, uint8_t>
				|| std::is_same_v<T, int16_t>
				|| std::is_same_v<T, uint16_t>
				|| std::is_same_v<T, int32_t>
				|| std::is_same_v<T, uint32_t>
				|| std::is_same_v<T, int64_t>
				|| std::is_same_v<T, uint64_t>
				|| std::is_same_v<T, float>
				)
			{
				read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(T));
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				int32_t ival;
				read(ival, default_ ? 1 : 0);
				val = ival != 0;
			}
			else
			{
				val = T::deserialize(*this);
				if (m_didErr)
				{
					val = default_;
				}
			}
		}

		template<typename T>
		void read(T& val)
		{
			if constexpr (
				   std::is_same_v<T, int8_t>
				|| std::is_same_v<T, uint8_t>
				|| std::is_same_v<T, int16_t>
				|| std::is_same_v<T, uint16_t>
				|| std::is_same_v<T, int32_t>
				|| std::is_same_v<T, uint32_t>
				|| std::is_same_v<T, int64_t>
				|| std::is_same_v<T, uint64_t>
				|| std::is_same_v<T, bool>
				|| std::is_same_v<T, float>
				)
			{
				read<T>(val, (T)0);
			}
			else if constexpr (std::is_same_v<T, bbe::List<float>>)
			{
				int64_t size;
				read(size);
				val.resizeCapacityAndLengthUninit(size);
				for (int64_t i = 0; i < size; i++)
				{
					float f;
					read(f);
					val[i] = f;
				}
			}
			else
			{
				val = T::deserialize(*this);
			}
		}

		ByteBufferSpan readSpan(size_t size);
		const char* readNullString();

		bool hasMore() const;
		size_t getLength() const;
	};

	class ByteBuffer
	{
	private:
		bbe::List<bbe::byte> m_bytes;
		
		void write(const bbe::byte* bytes, size_t length);

	public:
		ByteBuffer();
		ByteBuffer(bbe::List<bbe::byte>&& bytes);
		ByteBuffer(const std::initializer_list<bbe::byte>& il);

		template<typename T>
		void write(const T& val)
		{
			val.serialize(*this);
		}
		template<> void write(const   int8_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const  uint8_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const  int16_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const uint16_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const  int32_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const uint32_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const  int64_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const uint64_t &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const float    &val) { write((bbe::byte*) &val, sizeof(val)); }
		template<> void write(const bool     &val) 
		{
			int32_t ival = val ? 1 : 0;
			write(ival);
		}
		template<> void write(const bbe::List<float>& vals)
		{
			write((int64_t)vals.getLength());
			for (int64_t i = 0; i < vals.getLength(); i++)
			{
				write(vals[i]);
			}
		}
		void writeNullString(const char* string);

		bbe::byte* getRaw();
		const bbe::byte* getRaw() const;
		size_t getLength() const;

		int64_t reserveSizeToken();
		void fillSizeToken(int64_t sizeToken);

		ByteBufferSpan getSpan();
	};
}
