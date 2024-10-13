#include "BBE/ByteBuffer.h"
#include "BBE/String.h"
#include "BBE/BrotTime.h"

void bbe::ByteBufferSpan::read(bbe::byte* bytes, const bbe::byte* default_, size_t length)
{
	if (getLength() < length)
	{
		memcpy(bytes, default_, length);
		m_start = m_end;
		m_didErr = true;
		return;
	}
	if (m_endiannessFlipped)
	{
		for (size_t i = 0; i < length; i++)
		{
			bytes[length - 1 - i] = m_bytes->getRaw()[m_start + i];
		}
	}
	else
	{
		memcpy(bytes, m_bytes->getRaw() + m_start, length);
	}
	m_start += length;
}

bbe::ByteBufferSpan::ByteBufferSpan(bbe::List<bbe::byte>& bytes) :
	m_bytes(&bytes),
	m_start(0),
	m_end(bytes.getLength())
{
}

bbe::ByteBufferSpan::ByteBufferSpan(bbe::List<bbe::byte>& bytes, size_t start, size_t end) :
	m_bytes(&bytes),
	m_start(start),
	m_end(end)
{
}

bbe::ByteBufferSpan bbe::ByteBufferSpan::readSpan(size_t size)
{
	size_t subSpanStart = m_start;
	size_t subSpanEnd = m_start + size;
	if (subSpanEnd > m_end)
	{
		subSpanEnd = m_end;
		m_start = m_end;
	}
	else
	{
		m_start += size;
	}
	ByteBufferSpan span(*m_bytes, subSpanStart, subSpanEnd);
	return span;
}

const char* bbe::ByteBufferSpan::readNullString()
{
	if (m_start == m_end) return "";
	const char* retVal = (const char*)m_bytes->getRaw() + m_start;
	while ((*m_bytes)[m_start])
	{
		if (m_start == m_end - 1)
		{
			// Didn't find a null!
			bbe::Crash(bbe::Error::IllegalArgument);
		}
		m_start++;
	}
	m_start++;
	return retVal;
}

bool bbe::ByteBufferSpan::hasMore() const
{
	return m_start != m_end;
}

size_t bbe::ByteBufferSpan::getLength() const
{
	return m_end - m_start;
}

void bbe::ByteBufferSpan::reduceLengthTo(size_t length)
{
	if (getLength() < length) bbe::Crash(bbe::Error::IllegalArgument);
	m_end = m_start + length;
}

void bbe::ByteBufferSpan::skipBytes(size_t bytes)
{
	if (getLength() < bytes) m_start = m_end;
	else m_start += bytes;
}

bool bbe::ByteBufferSpan::valid() const
{
	return m_bytes != nullptr;
}

void bbe::ByteBufferSpan::flipEndianness()
{
	m_endiannessFlipped = !m_endiannessFlipped;
}


void bbe::ByteBuffer::write(const bbe::byte* bytes, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		m_bytes.add(bytes[i]);
	}
}

bbe::ByteBuffer::ByteBuffer()
{
}

bbe::ByteBuffer::ByteBuffer(const bbe::byte* data, size_t size)
{
	m_bytes.addArray(data, size);
}

bbe::ByteBuffer::ByteBuffer(bbe::List<bbe::byte>&& bytes) :
	m_bytes(bytes)
{
}

bbe::ByteBuffer::ByteBuffer(const std::initializer_list<bbe::byte>& il)	:
	m_bytes(il)
{
}

void bbe::ByteBuffer::writeNullString(const char* string, bool addNull)
{
	while (char c = *string)
	{
		m_bytes.add((bbe::byte)c);
		string++;
	}
	if(addNull) m_bytes.add((bbe::byte)0);
}

bbe::byte* bbe::ByteBuffer::getRaw()
{
	return m_bytes.getRaw();
}

const bbe::byte* bbe::ByteBuffer::getRaw() const
{
	return m_bytes.getRaw();
}

size_t bbe::ByteBuffer::getLength() const
{
	return m_bytes.getLength();
}

int64_t bbe::ByteBuffer::reserveSizeToken()
{
	int64_t val = 0;
	write(val);
	return m_bytes.getLength();
}

void bbe::ByteBuffer::fillSizeToken(int64_t sizeToken)
{
	int64_t size = m_bytes.getLength() - sizeToken;
	memcpy(m_bytes.getRaw() + sizeToken - sizeof(int64_t), &size, sizeof(size));
}

bbe::ByteBufferSpan bbe::ByteBuffer::getSpan()
{
	return ByteBufferSpan(m_bytes);
}

void bbe::ByteBuffer::removeFirstBytes(size_t amount)
{
	m_bytes.removeRange(0, amount);
}

uint8_t  bbe::ByteBufferSpan::readU8()  { uint8_t  val; read(val); return val; }
uint16_t bbe::ByteBufferSpan::readU16() { uint16_t val; read(val); return val; }
uint32_t bbe::ByteBufferSpan::readU32() { uint32_t val; read(val); return val; }
uint64_t bbe::ByteBufferSpan::readU64() { uint64_t val; read(val); return val; }
 int8_t  bbe::ByteBufferSpan::readI8()  {  int8_t  val; read(val); return val; }
 int16_t bbe::ByteBufferSpan::readI16() {  int16_t val; read(val); return val; }
 int32_t bbe::ByteBufferSpan::readI32() {  int32_t val; read(val); return val; }
 int64_t bbe::ByteBufferSpan::readI64() {  int64_t val; read(val); return val; }


void bbe::SerializedDescription::toByteBuffer(bbe::ByteBuffer& buffer) const
{
	for (size_t i = 0; i < descriptors.getLength(); i++)
	{
		int64_t elementLength = 1;
		if (descriptors[i].getRawVoid)
		{
			elementLength = descriptors[i].listLength;
			buffer.write(elementLength);
		}

		for (int64_t k = 0; k < elementLength; k++)
		{
			     if (descriptors[i].type == typeid(uint8_t))  buffer.write(*((uint8_t*) descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(uint16_t)) buffer.write(*((uint16_t*)descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(uint32_t)) buffer.write(*((uint32_t*)descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(uint64_t)) buffer.write(*((uint64_t*)descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(int8_t))   buffer.write(*((int8_t*)  descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(int16_t))  buffer.write(*((int16_t*) descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(int32_t))  buffer.write(*((int32_t*) descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(int64_t))  buffer.write(*((int64_t*) descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(float))    buffer.write(*((float*)   descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(bool))     buffer.write(*((bool*)    descriptors[i].addr + k));
			else if (descriptors[i].type == typeid(bbe::String)) ((bbe::String*)descriptors[i].addr + k)->serialize(buffer);
			else if (descriptors[i].type == typeid(bbe::TimePoint)) ((bbe::TimePoint*)descriptors[i].addr + k)->serialize(buffer);
			else bbe::Crash(bbe::Error::IllegalArgument);
		}
	}
}

void bbe::SerializedDescription::writeFromSpan(bbe::ByteBufferSpan& span) const
{
	for (size_t i = 0; i < descriptors.getLength(); i++)
	{
		int64_t elementLength = 1;
		const void* addr = descriptors[i].addr;
		if (descriptors[i].getRawVoid)
		{
			span.read(elementLength);
			descriptors[i].resize(elementLength);
			addr = descriptors[i].getRawVoid();
		}

		for(int64_t k = 0; k < elementLength; k++)
		{
			bool defaultValueAccepted = false;
			     if (descriptors[i].type == typeid(uint8_t )) { span.read(*((uint8_t* )addr + k), *(uint8_t* )&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid(uint16_t)) { span.read(*((uint16_t*)addr + k), *(uint16_t*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid(uint32_t)) { span.read(*((uint32_t*)addr + k), *(uint32_t*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid(uint64_t)) { span.read(*((uint64_t*)addr + k), *(uint64_t*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid( int8_t )) { span.read(*(( int8_t* )addr + k), *( int8_t* )&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid( int16_t)) { span.read(*(( int16_t*)addr + k), *( int16_t*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid( int32_t)) { span.read(*(( int32_t*)addr + k), *( int32_t*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid( int64_t)) { span.read(*(( int64_t*)addr + k), *( int64_t*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid( float  )) { span.read(*(( float*  )addr + k), *( float*  )&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid( bool   )) { span.read(*(( bool*   )addr + k), *( bool*   )&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else if (descriptors[i].type == typeid(bbe::String)) span.read(*((bbe::String*)addr + k));
			else if (descriptors[i].type == typeid(bbe::TimePoint)) { span.read(*((bbe::TimePoint*)addr + k), *(bbe::TimePoint*)&descriptors[i].defaultValueStorage); defaultValueAccepted = true; }
			else bbe::Crash(bbe::Error::IllegalArgument);
			
			if((!defaultValueAccepted || descriptors[i].getRawVoid) && descriptors[i].defaultValueStorage != 0)
			{
				// This type does not suppoert default values (yet).
				bbe::Crash(bbe::Error::IllegalArgument);
			}
		}
	}
}