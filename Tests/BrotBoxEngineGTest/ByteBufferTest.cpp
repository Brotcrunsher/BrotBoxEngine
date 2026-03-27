#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include <limits>

namespace
{
	struct SerializableIntList
	{
		bbe::List<int32_t> values;

		void serialDescription(bbe::SerializedDescription& desc)
		{
			desc.describe(values);
		}
	};
}

TEST(ByteBuffer, ReadSpanClampsOversizedRequestsWithoutOverflow)
{
	bbe::ByteBuffer buffer({ 1, 2, 3, 4, 5, 6, 7, 8 });
	bbe::ByteBufferSpan span = buffer.getSpan();
	span.skipBytes(4);

	bbe::ByteBufferSpan subSpan = span.readSpan(std::numeric_limits<size_t>::max());

	ASSERT_EQ(subSpan.getLength(), 4u);
	ASSERT_FALSE(span.hasMore());
	ASSERT_EQ(subSpan.readU8(), 5);
}

TEST(ByteBuffer, ReadListFloatRejectsNegativeLength)
{
	bbe::ByteBuffer buffer;
	int64_t negativeSize = -1;
	buffer.write(negativeSize);

	bbe::ByteBufferSpan span = buffer.getSpan();
	bbe::List<float> values;
	span.read(values);

	ASSERT_TRUE(values.isEmpty());
	ASSERT_FALSE(span.hasMore());
}

TEST(ByteBuffer, SerializedDescriptionRejectsNegativeListLength)
{
	bbe::ByteBuffer buffer;
	int64_t negativeSize = -1;
	buffer.write(negativeSize);

	bbe::ByteBufferSpan span = buffer.getSpan();
	SerializableIntList data;
	bbe::SerializedDescription desc;
	data.serialDescription(desc);
	desc.writeFromSpan(span);

	ASSERT_TRUE(data.values.isEmpty());
	ASSERT_FALSE(span.hasMore());
}

TEST(List, ResizeCapacityAndLengthUninitAfterExistingData)
{
	bbe::List<int> list = { 1, 2, 3, 4 };

	list.resizeCapacityAndLengthUninit(2);

	ASSERT_EQ(list.getLength(), 2u);
	list[0] = 11;
	list[1] = 22;
	ASSERT_EQ(list[0], 11);
	ASSERT_EQ(list[1], 22);
}
