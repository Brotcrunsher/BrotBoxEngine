#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include "TestUtils.h"

TEST(List, ConstructorDefault)
{
	bbe::List<int> list;
	bbe::List<int> const * pList = &list;
	ASSERT_EQ(list.getCapacity(), 0);
	ASSERT_EQ(list.getLength(), 0);
	ASSERT_EQ(list.getRaw(), nullptr);
	ASSERT_EQ(pList->getRaw(), nullptr);
	ASSERT_TRUE(list.isEmpty());
	ASSERT_FALSE(list.shrink());
}

TEST(List, ConstructorAmountOfData)
{
	bbe::List<int> list(17);
	bbe::List<int> const* pList = &list;
	ASSERT_EQ(list.getCapacity(), 17);
	ASSERT_EQ(list.getLength(), 0);
	ASSERT_NE(list.getRaw(), nullptr);
	ASSERT_NE(pList->getRaw(), nullptr);
	ASSERT_TRUE(list.isEmpty());
	ASSERT_TRUE(list.shrink());
	ASSERT_EQ(list.getCapacity(), 0);
}

TEST(List, ConstructorWithData)
{
	bbe::List<int> list(17, 1337);
	bbe::List<int> const* pList = &list;
	ASSERT_EQ(list.getCapacity(), 17);
	ASSERT_EQ(list.getLength(), 17);
	ASSERT_NE(list.getRaw(), nullptr);
	ASSERT_NE(pList->getRaw(), nullptr);
	ASSERT_FALSE(list.isEmpty());
	ASSERT_FALSE(list.shrink());
	for (size_t i = 0; i < list.getLength(); i++)
	{
		ASSERT_EQ(list[i], 1337);
	}
}

TEST(List, ConstructorCopy)
{
	bbe::List<int> list(1000, 42);
	bbe::List<int> copy(list);
	ASSERT_EQ(list.getCapacity(), 1000);
	ASSERT_EQ(list.getLength(), 1000);
	ASSERT_EQ(copy.getCapacity(), 1000);
	ASSERT_EQ(copy.getLength(), 1000);
	for (size_t i = 0; i < list.getLength(); i++)
	{
		ASSERT_EQ(list[i], 42);
		ASSERT_EQ(copy[i], 42);
	}

	copy[17] = 13;
	ASSERT_EQ(copy[17], 13);
	ASSERT_EQ(list[17], 42);
}

TEST(List, ConstructorMove)
{
	bbe::List<int> list(400, 123);
	bbe::List<int> move(std::move(list));
	ASSERT_EQ(list.getCapacity(), 0);
	ASSERT_EQ(list.getLength(), 0);
	ASSERT_EQ(list.getRaw(), nullptr);
	ASSERT_EQ(move.getCapacity(), 400);
	ASSERT_EQ(move.getLength(), 400);
	ASSERT_NE(move.getRaw(), nullptr);
	for (size_t i = 0; i < move.getLength(); i++)
	{
		ASSERT_EQ(move[i], 123);
	}
}

TEST(List, ConstructorInitializer)
{
	bbe::List<int> list = { 17, 32, 19, 2, 15 };
	ASSERT_EQ(list.getCapacity(), 5);
	ASSERT_EQ(list.getLength(), 5);
	ASSERT_NE(list.getRaw(), nullptr);
	ASSERT_EQ(list[0], 17);
	ASSERT_EQ(list[1], 32);
	ASSERT_EQ(list[2], 19);
	ASSERT_EQ(list[3], 2);
	ASSERT_EQ(list[4], 15);
}

TEST(List, OperatorAssignment)
{
	bbe::List<int> list1(17, 100);
	bbe::List<int> list2(32, 1000);

	list1 = list2;

	ASSERT_NE(list2.getLength(), 0);
	ASSERT_NE(list2.getCapacity(), 0);
	ASSERT_NE(list2.getRaw(), nullptr);

	ASSERT_EQ(list1.getLength(), 32);
	for (size_t i = 0; i < list1.getLength(); i++)
	{
		ASSERT_EQ(list1[i], 1000);
	}
}

TEST(List, OperatorMove)
{
	bbe::List<int> list1(17, 100);
	bbe::List<int> list2(32, 1000);

	list1 = std::move(list2);

	ASSERT_EQ(list2.getLength(), 0);
	ASSERT_EQ(list2.getCapacity(), 0);
	ASSERT_EQ(list2.getRaw(), nullptr);

	ASSERT_EQ(list1.getLength(), 32);
	for (size_t i = 0; i < list1.getLength(); i++)
	{
		ASSERT_EQ(list1[i], 1000);
	}
}

TEST(List, RecursiveList)
{
	bbe::List<SomeClass<int>> layer1_1(12, SomeClass<int>(18));
	bbe::List<SomeClass<int>> layer1_2(32, SomeClass<int>(20));

	bbe::List<bbe::List<SomeClass<int>>> layer2_1(3, layer1_1);
	bbe::List<bbe::List<SomeClass<int>>> layer2_2 = { layer1_1, layer1_2 };
}
