#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include "TestUtils.h"

TEST(List, ConstructorDefault)
{
	bbe::List<SomeClass<int>> list;
	bbe::List<SomeClass<int>> const * pList = &list;
	ASSERT_EQ(list.getCapacity(), 0);
	ASSERT_EQ(list.getLength(), 0);
	ASSERT_EQ(list.getRaw(), nullptr);
	ASSERT_EQ(pList->getRaw(), nullptr);
	ASSERT_TRUE(list.isEmpty());
	ASSERT_FALSE(list.shrink());
}

TEST(List, ConstructorAmountOfData)
{
	bbe::List<SomeClass<int>> list(17);
	bbe::List<SomeClass<int>> const* pList = &list;
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
	bbe::List<SomeClass<int>> list(17, 1337);
	bbe::List<SomeClass<int>> const* pList = &list;
	ASSERT_EQ(list.getCapacity(), 17);
	ASSERT_EQ(list.getLength(), 17);
	ASSERT_NE(list.getRaw(), nullptr);
	ASSERT_NE(pList->getRaw(), nullptr);
	ASSERT_FALSE(list.isEmpty());
	ASSERT_FALSE(list.shrink());
	for (size_t i = 0; i < list.getLength(); i++)
	{
		ASSERT_EQ(list[i].getLength(), 1337);
	}
}

TEST(List, ConstructorCopy)
{
	bbe::List<SomeClass<int>> list(100, 42);
	bbe::List<SomeClass<int>> copy(list);
	ASSERT_EQ(list.getCapacity(), 100);
	ASSERT_EQ(list.getLength(), 100);
	ASSERT_EQ(copy.getCapacity(), 100);
	ASSERT_EQ(copy.getLength(), 100);
	for (size_t i = 0; i < list.getLength(); i++)
	{
		ASSERT_EQ(list[i].getLength(), 42);
		ASSERT_EQ(copy[i].getLength(), 42);
	}

	copy[17].resize(13);
	ASSERT_EQ(copy[17].getLength(), 13);
	ASSERT_EQ(list[17].getLength(), 42);
}

TEST(List, ConstructorMove)
{
	bbe::List<SomeClass<int>> list(81, 123);
	bbe::List<SomeClass<int>> move(std::move(list));
	ASSERT_EQ(list.getCapacity(), 0);
	ASSERT_EQ(list.getLength(), 0);
	ASSERT_EQ(list.getRaw(), nullptr);
	ASSERT_EQ(move.getCapacity(), 81);
	ASSERT_EQ(move.getLength(), 81);
	ASSERT_NE(move.getRaw(), nullptr);
	for (size_t i = 0; i < move.getLength(); i++)
	{
		ASSERT_EQ(move[i].getLength(), 123);
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
	bbe::List<SomeClass<int>> list1(17, 100);
	bbe::List<SomeClass<int>> list2(32, 1000);

	list1 = list2;

	ASSERT_NE(list2.getLength(), 0);
	ASSERT_NE(list2.getCapacity(), 0);
	ASSERT_NE(list2.getRaw(), nullptr);

	ASSERT_EQ(list1.getLength(), 32);
	for (size_t i = 0; i < list1.getLength(); i++)
	{
		ASSERT_EQ(list1[i].getLength(), 1000);
	}
}

TEST(List, OperatorMove)
{
	bbe::List<SomeClass<int>> list1(17, 100);
	bbe::List<SomeClass<int>> list2(32, 1000);

	list1 = std::move(list2);

	ASSERT_EQ(list2.getLength(), 0);
	ASSERT_EQ(list2.getCapacity(), 0);
	ASSERT_EQ(list2.getRaw(), nullptr);

	ASSERT_EQ(list1.getLength(), 32);
	for (size_t i = 0; i < list1.getLength(); i++)
	{
		ASSERT_EQ(list1[i].getLength(), 1000);
	}
}

TEST(List, RecursiveList)
{
	bbe::List<SomeClass<int>> layer1_1(12, SomeClass<int>(18));
	bbe::List<SomeClass<int>> layer1_2(32, SomeClass<int>(20));

	bbe::List<bbe::List<SomeClass<int>>> layer2_1(3, layer1_1);
	bbe::List<bbe::List<SomeClass<int>>> layer2_2 = { layer1_1, layer1_2 };

	ASSERT_EQ(layer1_1.getLength(), 12);
	ASSERT_EQ(layer1_2.getLength(), 32);

	ASSERT_EQ(layer2_1.getLength(), 3);
	ASSERT_EQ(layer2_2.getLength(), 2);

	for (size_t i = 0; i < layer2_1.getLength(); i++)
	{
		ASSERT_EQ(layer2_1[i].getLength(), 12);
	}

	ASSERT_EQ(layer2_2[0].getLength(), 12);
	ASSERT_EQ(layer2_2[1].getLength(), 32);

	for (size_t i = 0; i < layer2_1.getLength(); i++)
	{
		for (size_t k = 0; k < layer2_1[i].getLength(); k++)
		{
			ASSERT_EQ(layer2_1[i][k].getLength(), 18);
		}
	}

	layer2_1[1][6].resize(1000);
	for (size_t i = 0; i < layer2_1.getLength(); i++)
	{
		for (size_t k = 0; k < layer2_1[i].getLength(); k++)
		{
			if(i == 1 && k == 6)
				ASSERT_EQ(layer2_1[i][k].getLength(), 1000);
			else
				ASSERT_EQ(layer2_1[i][k].getLength(), 18);
		}
	}
}

TEST(List, Add)
{
	bbe::List<SomeClass<int>> list;
	for (size_t i = 1; i < 52; i++)
	{
		list.add(SomeClass<int>(i));
	}
	bbe::List<SomeClass<int>> copy = list;
	list[13] = SomeClass<int>(1337);

	ASSERT_EQ(list.getLength(), copy.getLength());
	for (size_t i = 0; i < list.getLength(); i++)
	{
		ASSERT_EQ(copy[i].getLength(), i + 1);
		if (i != 13) ASSERT_EQ(list[i].getLength(), i + 1);
		else ASSERT_EQ(list[i].getLength(), 1337);
	}
}

TEST(List, CombineUnorderedLists)
{
	SomeClass<int> a = 17;
	SomeClass<int> b = 13;
	SomeClass<int> c = 1337;
	SomeClass<int> d = 2;
	SomeClass<int> e = 4;
	SomeClass<int> f = 2;

	bbe::List<SomeClass<int>> list1 = { a, b, c, d, e, f };
	ASSERT_EQ(list1[0].getLength(), 17);
	ASSERT_EQ(list1[1].getLength(), 13);
	ASSERT_EQ(list1[2].getLength(), 1337);
	ASSERT_EQ(list1[3].getLength(), 2);
	ASSERT_EQ(list1[4].getLength(), 4);
	ASSERT_EQ(list1[5].getLength(), 2);

	SomeClass<int> g = 18;
	SomeClass<int> h = 4;
	SomeClass<int> i = 100;
	SomeClass<int> j = 1;
	SomeClass<int> k = 1;
	SomeClass<int> l = 2;

	bbe::List<SomeClass<int>> list2 = { g, h, i, j, k, l };
	ASSERT_EQ(list2[0].getLength(), 18);
	ASSERT_EQ(list2[1].getLength(), 4);
	ASSERT_EQ(list2[2].getLength(), 100);
	ASSERT_EQ(list2[3].getLength(), 1);
	ASSERT_EQ(list2[4].getLength(), 1);
	ASSERT_EQ(list2[5].getLength(), 2);

	list1 += list2;
	ASSERT_EQ(list1[ 0].getLength(), 17);
	ASSERT_EQ(list1[ 1].getLength(), 13);
	ASSERT_EQ(list1[ 2].getLength(), 1337);
	ASSERT_EQ(list1[ 3].getLength(), 2);
	ASSERT_EQ(list1[ 4].getLength(), 4);
	ASSERT_EQ(list1[ 5].getLength(), 2);
	ASSERT_EQ(list1[ 6].getLength(), 18);
	ASSERT_EQ(list1[ 7].getLength(), 4);
	ASSERT_EQ(list1[ 8].getLength(), 100);
	ASSERT_EQ(list1[ 9].getLength(), 1);
	ASSERT_EQ(list1[10].getLength(), 1);
	ASSERT_EQ(list1[11].getLength(), 2);
}

TEST(List, CombineOrderedLists)
{
	SomeClass<int> a = 17;
	SomeClass<int> b = 13;
	SomeClass<int> c = 1337;
	SomeClass<int> d = 2;
	SomeClass<int> e = 4;
	SomeClass<int> f = 2;

	bbe::List<SomeClass<int>, true> list1 = { a, b, c, d, e, f };
	ASSERT_EQ(list1[0].getLength(),    2);
	ASSERT_EQ(list1[1].getLength(),    2);
	ASSERT_EQ(list1[2].getLength(),    4);
	ASSERT_EQ(list1[3].getLength(),   13);
	ASSERT_EQ(list1[4].getLength(),   17);
	ASSERT_EQ(list1[5].getLength(), 1337);

	SomeClass<int> g = 18;
	SomeClass<int> h = 4;
	SomeClass<int> i = 100;
	SomeClass<int> j = 1;
	SomeClass<int> k = 1;
	SomeClass<int> l = 2;

	bbe::List<SomeClass<int>, true> list2 = { g, h, i, j, k, l };
	ASSERT_EQ(list2[0].getLength(),   1);
	ASSERT_EQ(list2[1].getLength(),   1);
	ASSERT_EQ(list2[2].getLength(),   2);
	ASSERT_EQ(list2[3].getLength(),   4);
	ASSERT_EQ(list2[4].getLength(),  18);
	ASSERT_EQ(list2[5].getLength(), 100);

	list1 += list2;
	ASSERT_EQ(list1[ 0].getLength(),    1);
	ASSERT_EQ(list1[ 1].getLength(),    1);
	ASSERT_EQ(list1[ 2].getLength(),    2);
	ASSERT_EQ(list1[ 3].getLength(),    2);
	ASSERT_EQ(list1[ 4].getLength(),    2);
	ASSERT_EQ(list1[ 5].getLength(),    4);
	ASSERT_EQ(list1[ 6].getLength(),    4);
	ASSERT_EQ(list1[ 7].getLength(),   13);
	ASSERT_EQ(list1[ 8].getLength(),   17);
	ASSERT_EQ(list1[ 9].getLength(),   18);
	ASSERT_EQ(list1[10].getLength(),  100);
	ASSERT_EQ(list1[11].getLength(), 1337);
}
