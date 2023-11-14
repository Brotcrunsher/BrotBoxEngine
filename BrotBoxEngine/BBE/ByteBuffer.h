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

		void read(bbe::byte* bytes, bbe::byte* default_, size_t length);

	public:
		ByteBufferSpan(bbe::List<bbe::byte>& bytes);
		ByteBufferSpan(bbe::List<bbe::byte>& bytes, size_t start, size_t end);

		void read(  int8_t& val,   int8_t default_ = 0);
		void read( uint8_t& val,  uint8_t default_ = 0);
		void read( int16_t& val,  int16_t default_ = 0);
		void read(uint16_t& val, uint16_t default_ = 0);
		void read( int32_t& val,  int32_t default_ = 0);
		void read(uint32_t& val, uint32_t default_ = 0);
		void read( int64_t& val,  int64_t default_ = 0);
		void read(uint64_t& val, uint64_t default_ = 0);
		void read(bool& val, bool default_ = false);
		void read(float& val, float default_ = 0.f);
		void read(bbe::List<float>& val);
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

		void write(  int8_t val);
		void write( uint8_t val);
		void write( int16_t val);
		void write(uint16_t val);
		void write( int32_t val);
		void write(uint32_t val);
		void write( int64_t val);
		void write(uint64_t val);
		void write(bool val);
		void write(float val);
		void write(const bbe::List<float>& vals);
		void writeNullString(const char* string);

		bbe::byte* getRaw();
		const bbe::byte* getRaw() const;
		size_t getLength() const;

		int64_t reserveSizeToken();
		void fillSizeToken(int64_t sizeToken);

		ByteBufferSpan getSpan();
	};
}
