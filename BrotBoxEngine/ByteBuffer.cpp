#include "BBE/ByteBuffer.h"

void bbe::ByteBufferSpan::read(bbe::byte* bytes, bbe::byte* default_, size_t length)
{
	if (getLength() < length)
	{
		memcpy(bytes, default_, length);
		m_start = m_end;
		m_didErr = true;
		return;
	}
	memcpy(bytes, m_bytes.getRaw() + m_start, length);
	m_start += length;
}

bbe::ByteBufferSpan::ByteBufferSpan(bbe::List<bbe::byte>& bytes) :
	m_bytes(bytes),
	m_start(0),
	m_end(m_bytes.getLength())
{
}

bbe::ByteBufferSpan::ByteBufferSpan(bbe::List<bbe::byte>& bytes, size_t start, size_t end) :
	m_bytes(bytes),
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
	ByteBufferSpan span(m_bytes, subSpanStart, subSpanEnd);
	return span;
}

const char* bbe::ByteBufferSpan::readNullString()
{
	if (m_start == m_end) return "";
	const char* retVal = (const char*)m_bytes.getRaw() + m_start;
	while (m_bytes[m_start])
	{
		if (m_start == m_end - 1)
		{
			// Didn't find a null!
			throw IllegalArgumentException();
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

bbe::ByteBuffer::ByteBuffer(bbe::List<bbe::byte>&& bytes) :
	m_bytes(bytes)
{
}

bbe::ByteBuffer::ByteBuffer(const std::initializer_list<bbe::byte>& il)	:
	m_bytes(il)
{
}

void bbe::ByteBuffer::writeNullString(const char* string)
{
	while (char c = *string)
	{
		m_bytes.add((bbe::byte)c);
		string++;
	}
	m_bytes.add((bbe::byte)0);
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
	write((int64_t)0);
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
