#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include "TestUtils.h"

TEST(StringIteratorTest, nullptr_test)
{
	bbe::Utf8Iterator iter;
	ASSERT_EQ(iter, nullptr);
}

TEST(StringIteratorTest, simpleIncrementPost)
{
	bbe::Utf8Iterator iter("1234");
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, "234"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, "34"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, "4"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, ""));
}

TEST(StringIteratorTest, simpleIncrementPre)
{
	bbe::Utf8Iterator iter("1234");
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
	++iter;
	ASSERT_TRUE(0 == strcmp(iter, "234"));
	++iter;
	ASSERT_TRUE(0 == strcmp(iter, "34"));
	++iter;
	ASSERT_TRUE(0 == strcmp(iter, "4"));
	++iter;
	ASSERT_TRUE(0 == strcmp(iter, ""));
}

TEST(StringIteratorTest, simpleDecrementPost)
{
	const char* str = "1234";
	bbe::Utf8Iterator iter(str + 4);
	ASSERT_TRUE(0 == strcmp(iter, ""));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, "4"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, "34"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, "234"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
}

TEST(StringIteratorTest, simpleDecrementPre)
{
	const char* str = "1234";
	bbe::Utf8Iterator iter(str + 4);
	ASSERT_TRUE(0 == strcmp(iter, ""));
	--iter;
	ASSERT_TRUE(0 == strcmp(iter, "4"));
	--iter;
	ASSERT_TRUE(0 == strcmp(iter, "34"));
	--iter;
	ASSERT_TRUE(0 == strcmp(iter, "234"));
	--iter;
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
}

TEST(StringIteratorTest, mixDeIncrementPrePost)
{
	bbe::Utf8Iterator iter("1234");
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, "234"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
	++iter;
	ASSERT_TRUE(0 == strcmp(iter, "234"));
	--iter;
	ASSERT_TRUE(0 == strcmp(iter, "1234"));
}

TEST(StringIteratorTest, utf8Increment)
{
	bbe::Utf8Iterator iter(u8"💣1🍣💃");
	ASSERT_TRUE(0 == strcmp(iter, u8"💣1🍣💃"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, u8"1🍣💃"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, u8"🍣💃"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, u8"💃"));
	iter++;
	ASSERT_TRUE(0 == strcmp(iter, u8""));
}

TEST(StringIteratorTest, ut8fDecrement)
{
	const char* str = u8"💣1🍣💃";
	bbe::Utf8Iterator iter(str + ::strlen(str));
	ASSERT_TRUE(0 == strcmp(iter, u8""));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, u8"💃"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, u8"🍣💃"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, u8"1🍣💃"));
	iter--;
	ASSERT_TRUE(0 == strcmp(iter, u8"💣1🍣💃"));
}

TEST(StringIteratorTest, intManipulation)
{
	const int32_t negativeTwo = -2;
	bbe::Utf8Iterator iter(u8"💣1🍣💃");
	ASSERT_TRUE(0 == strcmp(iter, u8"💣1🍣💃"));
	iter += 2;
	ASSERT_TRUE(0 == strcmp(iter, u8"🍣💃"));
	iter -= 2;
	ASSERT_TRUE(0 == strcmp(iter, u8"💣1🍣💃"));
	iter -= negativeTwo;
	ASSERT_TRUE(0 == strcmp(iter, u8"🍣💃"));
	iter += negativeTwo;
	ASSERT_TRUE(0 == strcmp(iter, u8"💣1🍣💃"));
}

TEST(StringIteratorTest, intManipulationCopy)
{
	const int32_t negativeTwo = -2;
	bbe::Utf8Iterator iter(u8"💣1🍣💃");
	bbe::Utf8Iterator iter2 = iter + 2;
	bbe::Utf8Iterator iter3 = iter2 - 2;
	bbe::Utf8Iterator iter4 = iter3 - negativeTwo;
	bbe::Utf8Iterator iter5 = iter4 + negativeTwo;
	ASSERT_TRUE(0 == strcmp(iter, u8"💣1🍣💃"));
	ASSERT_TRUE(0 == strcmp(iter2, u8"🍣💃"));
	ASSERT_TRUE(0 == strcmp(iter3, u8"💣1🍣💃"));
	ASSERT_TRUE(0 == strcmp(iter4, u8"🍣💃"));
	ASSERT_TRUE(0 == strcmp(iter5, u8"💣1🍣💃"));
}

TEST(StringIteratorTest, distance)
{
	bbe::Utf8Iterator iter(u8"💣1🍣💃");
	bbe::Utf8Iterator iter2 = iter + 2;
	ASSERT_TRUE(iter - iter2, -2);
	ASSERT_TRUE(iter2 - iter,  2);
}
