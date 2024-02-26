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
			val = T::deserialize(*this);
			if (m_didErr)
			{
				val = default_;
			}
		}

		template<typename T>
		void read(T& val)
		{
			val = T::deserialize(*this);
		}

		template <> void read(  int8_t& val,   int8_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read( uint8_t& val,  uint8_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read( int16_t& val,  int16_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read(uint16_t& val, uint16_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read( int32_t& val,  int32_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read(uint32_t& val, uint32_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read( int64_t& val,  int64_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read(uint64_t& val, uint64_t default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read(float& val,    float    default_) { read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(val)); }
		template <> void read(bool& val,     bool     default_)
		{
			int32_t ival;
			read(ival, default_ ? 1 : 0);
			val = ival != 0;
		}
		template <> void read(  int8_t& val) { read<  int8_t>(val, 0);}
		template <> void read( uint8_t& val) { read< uint8_t>(val, 0);}
		template <> void read( int16_t& val) { read< int16_t>(val, 0);}
		template <> void read(uint16_t& val) { read<uint16_t>(val, 0);}
		template <> void read( int32_t& val) { read< int32_t>(val, 0);}
		template <> void read(uint32_t& val) { read<uint32_t>(val, 0);}
		template <> void read( int64_t& val) { read< int64_t>(val, 0);}
		template <> void read(uint64_t& val) { read<uint64_t>(val, 0);}
		template <> void read(bool& val    ) { read<bool>(val, false);}
		template <> void read(float& val   ) { read<float>(val, 0.0f);}
		template <> void read(bbe::List<float>& val)
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
