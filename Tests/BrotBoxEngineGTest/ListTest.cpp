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

namespace bbe
{
	void aaa(const bbe::test::Person& a) {
		std::cout << a;
	}
}

TEST(List, OperatorAssignment)
{
	bbe::test::Person a;
	bbe::aaa(a);
	//bbe::test::assertEqualsImpl(__FILE__, __LINE__, a, b);

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

TEST(List, AddUnordered)
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

TEST(List, AddOrdered)
{
	bbe::List<SomeClass<int>, true> list;
	for (size_t i = 0; i < 51; i++)
	{
		list.add(SomeClass<int>(((i + 10) * bbe::Math::BIGGEST_PRIME_32_SIGNED % 51) + 1));
	}
	for (size_t i = 0; i < 51; i++)
	{
		ASSERT_EQ(list[i].getLength(), i + 1);
	}
}

TEST(List, AddUnorderedMove)
{
	bbe::List<SomeClass<int>, true> list;
	for (size_t i = 0; i < 51; i++)
	{
		SomeClass data = SomeClass<int>(i + 1);
		list.add(std::move(data));
		ASSERT_EQ(data.getLength(), 0);
	}
	for (size_t i = 0; i < 51; i++)
	{
		ASSERT_EQ(list[i].getLength(), i + 1);
	}
}

TEST(List, AddOrderedMove)
{
	bbe::List<SomeClass<int>, true> list;
	for (size_t i = 0; i < 51; i++)
	{
		SomeClass data = SomeClass<int>(((i + 10) * bbe::Math::BIGGEST_PRIME_32_SIGNED % 51) + 1);
		list.add(std::move(data));
		ASSERT_EQ(data.getLength(), 0);
	}
	for (size_t i = 0; i < 51; i++)
	{
		ASSERT_EQ(list[i].getLength(), i + 1);
	}
}

TEST(List, GetIndexOnAddUnordered)
{
	bbe::List<SomeClass<int>> list;
	for (size_t i = 0; i < 51; i++)
	{
		SomeClass data = SomeClass<int>(((i + 10) * bbe::Math::BIGGEST_PRIME_32_SIGNED % 51) + 1);
		ASSERT_EQ(list.getIndexOnAdd(data), i);
		list.add(data);
	}
}

TEST(List, GetIndexOnAddOrdered)
{
	bbe::List<SomeClass<int>, true> list;
	for (size_t i = 0; i < 51; i++)
	{
		SomeClass data = SomeClass<int>(((i + 10) * bbe::Math::BIGGEST_PRIME_32_SIGNED % 51) + 1);
		if(i ==  0) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i ==  1) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i ==  2) ASSERT_EQ(list.getIndexOnAdd(data),  1);
		if(i ==  3) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i ==  4) ASSERT_EQ(list.getIndexOnAdd(data),  2);
		if(i ==  5) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i ==  6) ASSERT_EQ(list.getIndexOnAdd(data),  3);
		if(i ==  7) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i ==  8) ASSERT_EQ(list.getIndexOnAdd(data),  4);
		if(i ==  9) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 10) ASSERT_EQ(list.getIndexOnAdd(data),  5);
		if(i == 11) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 12) ASSERT_EQ(list.getIndexOnAdd(data),  6);
		if(i == 13) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 14) ASSERT_EQ(list.getIndexOnAdd(data),  7);
		if(i == 15) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 16) ASSERT_EQ(list.getIndexOnAdd(data),  8);
		if(i == 17) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 18) ASSERT_EQ(list.getIndexOnAdd(data),  9);
		if(i == 19) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 20) ASSERT_EQ(list.getIndexOnAdd(data), 10);
		if(i == 21) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 22) ASSERT_EQ(list.getIndexOnAdd(data), 11);
		if(i == 23) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 24) ASSERT_EQ(list.getIndexOnAdd(data), 12);
		if(i == 25) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 26) ASSERT_EQ(list.getIndexOnAdd(data), 13);
		if(i == 27) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 28) ASSERT_EQ(list.getIndexOnAdd(data), 14);
		if(i == 29) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 30) ASSERT_EQ(list.getIndexOnAdd(data), 15);
		if(i == 31) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 32) ASSERT_EQ(list.getIndexOnAdd(data), 16);
		if(i == 33) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 34) ASSERT_EQ(list.getIndexOnAdd(data), 17);
		if(i == 35) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 36) ASSERT_EQ(list.getIndexOnAdd(data), 18);
		if(i == 37) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 38) ASSERT_EQ(list.getIndexOnAdd(data), 19);
		if(i == 39) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 40) ASSERT_EQ(list.getIndexOnAdd(data), 20);
		if(i == 41) ASSERT_EQ(list.getIndexOnAdd(data),  0);
		if(i == 42) ASSERT_EQ(list.getIndexOnAdd(data), 21);
		if(i == 43) ASSERT_EQ(list.getIndexOnAdd(data), 43);
		if(i == 44) ASSERT_EQ(list.getIndexOnAdd(data), 21);
		if(i == 45) ASSERT_EQ(list.getIndexOnAdd(data), 44);
		if(i == 46) ASSERT_EQ(list.getIndexOnAdd(data), 21);
		if(i == 47) ASSERT_EQ(list.getIndexOnAdd(data), 45);
		if(i == 48) ASSERT_EQ(list.getIndexOnAdd(data), 21);
		if(i == 49) ASSERT_EQ(list.getIndexOnAdd(data), 46);
		if(i == 50) ASSERT_EQ(list.getIndexOnAdd(data), 21);
		list.add(data);
	}
}

TEST(List, getNeighbors)
{
	SomeClass<int> a = 2;
	SomeClass<int> b = 2;
	SomeClass<int> c = 4;
	SomeClass<int> d = 13;
	SomeClass<int> e = 13;
	SomeClass<int> f = 1337;
	bbe::List<SomeClass<int>, true> list1 = { a, b, c, d, e, f };

	SomeClass<int>* left  = nullptr;
	SomeClass<int>* right = nullptr;
	list1.getNeighbors(SomeClass<int>(1), left, right);
	ASSERT_EQ(left, nullptr);
	ASSERT_EQ(right->getLength(), 2);
	list1.getNeighbors(SomeClass<int>(2), left, right);
	ASSERT_EQ(left->getLength() , 2);
	ASSERT_EQ(right->getLength(), 4);
	list1.getNeighbors(SomeClass<int>(3), left, right);
	ASSERT_EQ(left->getLength() , 2);
	ASSERT_EQ(right->getLength(), 4);
	list1.getNeighbors(SomeClass<int>(4), left, right);
	ASSERT_EQ(left->getLength() , 4);
	ASSERT_EQ(right->getLength(), 13);
	list1.getNeighbors(SomeClass<int>(10), left, right);
	ASSERT_EQ(left->getLength() , 4);
	ASSERT_EQ(right->getLength(), 13);
	list1.getNeighbors(SomeClass<int>(13), left, right);
	ASSERT_EQ(left->getLength() , 13);
	ASSERT_EQ(right->getLength(), 1337);
	list1.getNeighbors(SomeClass<int>(200), left, right);
	ASSERT_EQ(left->getLength() , 13);
	ASSERT_EQ(right->getLength(), 1337);
	list1.getNeighbors(SomeClass<int>(1337), left, right);
	ASSERT_EQ(left->getLength() , 13);
	ASSERT_EQ(right->getLength(), 1337);
	list1.getNeighbors(SomeClass<int>(1338), left, right);
	ASSERT_EQ(left->getLength() , 1337);
	ASSERT_EQ(right, nullptr);
}

TEST(List, addAllUnordered)
{
	bbe::List<SomeClass<int>> list;
	list.addAll(SomeClass<int>(3), SomeClass<int>(18), SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(1337), SomeClass<int>(3));
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list[0].getLength(), 3);
	ASSERT_EQ(list[1].getLength(), 18);
	ASSERT_EQ(list[2].getLength(), 1);
	ASSERT_EQ(list[3].getLength(), 2);
	ASSERT_EQ(list[4].getLength(), 1337);
	ASSERT_EQ(list[5].getLength(), 3);
}

TEST(List, addAllOrdered)
{
	bbe::List<SomeClass<int>, true> list;
	list.addAll(SomeClass<int>(3), SomeClass<int>(18), SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(1337), SomeClass<int>(3));
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 2);
	ASSERT_EQ(list[2].getLength(), 3);
	ASSERT_EQ(list[3].getLength(), 3);
	ASSERT_EQ(list[4].getLength(), 18);
	ASSERT_EQ(list[5].getLength(), 1337);
}

TEST(List, addCArray)
{
	SomeClass<int> arr[10];
	for (size_t i = 0; i < 10; i++)
	{
		arr[i] = SomeClass<int>(i + 1);
	}
	bbe::List<SomeClass<int>, true> list;
	list.addArray(arr, 10);
	ASSERT_EQ(list.getLength(), 10);
	for (size_t i = 0; i < 10; i++)
	{
		ASSERT_EQ(list[i].getLength(), i + 1);
	}
}

TEST(List, addArray)
{
	bbe::Array<SomeClass<int>, 10> arr;
	for (size_t i = 0; i < 10; i++)
	{
		arr[i] = SomeClass<int>(i + 1);
	}
	bbe::List<SomeClass<int>, true> list;
	list.addArray(arr);
	ASSERT_EQ(list.getLength(), 10);
	for (size_t i = 0; i < 10; i++)
	{
		ASSERT_EQ(list[i].getLength(), i + 1);
	}
}

TEST(List, popBack)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4) };
	ASSERT_EQ(list.popBack(), 4);
	ASSERT_EQ(list.getLength(), 3);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 2);
	ASSERT_EQ(list[2].getLength(), 3);

	ASSERT_EQ(list.popBack(), 3);
	ASSERT_EQ(list.getLength(), 2);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 2);

	list.add(SomeClass<int>(1337));
	ASSERT_EQ(list.getLength(), 3);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 2);
	ASSERT_EQ(list[2].getLength(), 1337);
}

TEST(List, clear)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4) };
	const size_t oldCapacity = list.getCapacity();
	list.clear();
	ASSERT_EQ(list.getLength(), 0);
	ASSERT_EQ(list.getCapacity(), oldCapacity);
}

TEST(List, shrink)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4) };
	const size_t oldCapacity = list.getCapacity();
	list.popBack();
	ASSERT_EQ(list.getCapacity(), oldCapacity);
	ASSERT_NE(list.getCapacity(), 3);

	list.shrink();
	ASSERT_EQ(list.getCapacity(), 3);
	ASSERT_EQ(list.getLength(), 3);
}

TEST(List, resizeCapacity)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4) };
	list.resizeCapacity(100);
	ASSERT_EQ(list.getCapacity(), 100);
	ASSERT_EQ(list.getLength(), 4);
}

TEST(List, resizeCapacityAndLength)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4) };
	list.resizeCapacityAndLength(100);
	ASSERT_EQ(list.getCapacity(), 100);
	ASSERT_EQ(list.getLength(), 100);
}

TEST(List, removeAllByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	list.removeAll(SomeClass<int>(2));
	ASSERT_EQ(list.getLength(), 4);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 3);
	ASSERT_EQ(list[2].getLength(), 4);
	ASSERT_EQ(list[3].getLength(), 4);
}

TEST(List, removeAllByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	list.removeAll([](const SomeClass<int>& s) { return s.getLength() % 2 == 0; });
	ASSERT_EQ(list.getLength(), 2);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 3);
}

TEST(List, removeSingleByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	list.removeSingle(SomeClass<int>(2));
	ASSERT_EQ(list.getLength(), 5);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 3);
	ASSERT_EQ(list[2].getLength(), 4);
	ASSERT_EQ(list[3].getLength(), 2);
	ASSERT_EQ(list[4].getLength(), 4);
}

TEST(List, removeSingleByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	list.removeSingle([](const SomeClass<int>& s) { return s.getLength() % 2 == 0; });
	ASSERT_EQ(list.getLength(), 5);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 3);
	ASSERT_EQ(list[2].getLength(), 4);
	ASSERT_EQ(list[3].getLength(), 2);
	ASSERT_EQ(list[4].getLength(), 4);
}

TEST(List, removeIndex)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(6) };
	ASSERT_EQ(list.getLength(), 6);
	list.removeIndex(4);
	ASSERT_EQ(list.getLength(), 5);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 2);
	ASSERT_EQ(list[2].getLength(), 3);
	ASSERT_EQ(list[3].getLength(), 4);
	ASSERT_EQ(list[4].getLength(), 6);
	list.removeIndex(0);
	ASSERT_EQ(list.getLength(), 4);
	ASSERT_EQ(list[0].getLength(), 2);
	ASSERT_EQ(list[1].getLength(), 3);
	ASSERT_EQ(list[2].getLength(), 4);
	ASSERT_EQ(list[3].getLength(), 6);
	list.removeIndex(3);
	ASSERT_EQ(list.getLength(), 3);
	ASSERT_EQ(list[0].getLength(), 2);
	ASSERT_EQ(list[1].getLength(), 3);
	ASSERT_EQ(list[2].getLength(), 4);
}

TEST(List, containsAmountByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list.containsAmount(SomeClass<int>(1)), 1);
	ASSERT_EQ(list.containsAmount(SomeClass<int>(2)), 2);
	ASSERT_EQ(list.containsAmount(SomeClass<int>(3)), 1);
	ASSERT_EQ(list.containsAmount(SomeClass<int>(4)), 2);
	ASSERT_EQ(list.containsAmount(SomeClass<int>(5)), 0);
}

TEST(List, containsAmountByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list.containsAmount([](const SomeClass<int> s) { return s.getLength() % 2 == 0;  }), 4);
	ASSERT_EQ(list.containsAmount([](const SomeClass<int> s) { return s.getLength() % 2 == 1;  }), 2);
	ASSERT_EQ(list.containsAmount([](const SomeClass<int> s) { return s.getLength() == 10000;  }), 0);
}

TEST(List, containsByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list.contains(SomeClass<int>(1)), true);
	ASSERT_EQ(list.contains(SomeClass<int>(2)), true);
	ASSERT_EQ(list.contains(SomeClass<int>(3)), true);
	ASSERT_EQ(list.contains(SomeClass<int>(4)), true);
	ASSERT_EQ(list.contains(SomeClass<int>(5)), false);
}

TEST(List, containsByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list.contains([](const SomeClass<int> s) { return s.getLength() % 2 == 0;  }), true);
	ASSERT_EQ(list.contains([](const SomeClass<int> s) { return s.getLength() % 2 == 1;  }), true);
	ASSERT_EQ(list.contains([](const SomeClass<int> s) { return s.getLength() == 10000;  }), false);
}

TEST(List, containsUniqueByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list.containsUnique(SomeClass<int>(1)), true);
	ASSERT_EQ(list.containsUnique(SomeClass<int>(2)), false);
	ASSERT_EQ(list.containsUnique(SomeClass<int>(3)), true);
	ASSERT_EQ(list.containsUnique(SomeClass<int>(4)), false);
	ASSERT_EQ(list.containsUnique(SomeClass<int>(5)), false);
}

TEST(List, containsUniqueByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(4) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list.containsUnique([](const SomeClass<int> s) { return s.getLength() % 2 == 0;  }), false);
	ASSERT_EQ(list.containsUnique([](const SomeClass<int> s) { return s.getLength() % 2 == 1;  }), false);
	ASSERT_EQ(list.containsUnique([](const SomeClass<int> s) { return s.getLength() == 10000;  }), false);
	ASSERT_EQ(list.containsUnique([](const SomeClass<int> s) { return s.getLength() == 3;      }), true);
}

TEST(List, beginEnd)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(1), SomeClass<int>(2), SomeClass<int>(3), SomeClass<int>(4), SomeClass<int>(5), SomeClass<int>(6) };
	size_t i = 1;
	for (auto it = list.begin(); it != list.end(); it++, i++)
	{
		ASSERT_EQ(it->getLength(), i);
	}

	const bbe::List<SomeClass<int>>& con = list;
	i = 1;
	for (auto it = con.begin(); it != con.end(); it++, i++)
	{
		ASSERT_EQ(it->getLength(), i);
	}
}

TEST(List, sort)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	ASSERT_EQ(list.getLength(), 6);
	list.sort();
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list[0].getLength(), 1);
	ASSERT_EQ(list[1].getLength(), 2);
	ASSERT_EQ(list[2].getLength(), 3);
	ASSERT_EQ(list[3].getLength(), 4);
	ASSERT_EQ(list[4].getLength(), 4);
	ASSERT_EQ(list[5].getLength(), 7);

	list.sort([](const SomeClass<int> a, const SomeClass<int> b) { return a > b; });
	ASSERT_EQ(list[0].getLength(), 7);
	ASSERT_EQ(list[1].getLength(), 4);
	ASSERT_EQ(list[2].getLength(), 4);
	ASSERT_EQ(list[3].getLength(), 3);
	ASSERT_EQ(list[4].getLength(), 2);
	ASSERT_EQ(list[5].getLength(), 1);
}

TEST(List, first)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list[0], list.first());
}

TEST(List, last)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	ASSERT_EQ(list.getLength(), 6);
	ASSERT_EQ(list[5], list.last());
}

TEST(List, findByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };

	SomeClass<int>* ptr = list.find(SomeClass<int>(4));
	ASSERT_EQ(ptr, &list[0]);
	ptr = list.find(SomeClass<int>(2));
	ASSERT_EQ(ptr, &list[1]);
	ptr = list.find(SomeClass<int>(1));
	ASSERT_EQ(ptr, &list[2]);
	ptr = list.find(SomeClass<int>(7));
	ASSERT_EQ(ptr, &list[4]);
	ptr = list.find(SomeClass<int>(3));
	ASSERT_EQ(ptr, &list[5]);
	ptr = list.find(SomeClass<int>(100));
	ASSERT_EQ(ptr, nullptr);
}

TEST(List, findByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };

	SomeClass<int>* ptr = list.find([](const SomeClass<int>& s) { return s.getLength() == 4; });
	ASSERT_EQ(ptr, &list[0]);
	ptr = list.find([](const SomeClass<int>& s) { return s.getLength() == 2; });
	ASSERT_EQ(ptr, &list[1]);
	ptr = list.find([](const SomeClass<int>& s) { return s.getLength() == 1; });
	ASSERT_EQ(ptr, &list[2]);
	ptr = list.find([](const SomeClass<int>& s) { return s.getLength() == 7; });
	ASSERT_EQ(ptr, &list[4]);
	ptr = list.find([](const SomeClass<int>& s) { return s.getLength() == 3; });
	ASSERT_EQ(ptr, &list[5]);
	ptr = list.find([](const SomeClass<int>& s) { return s.getLength() == 100; });
	ASSERT_EQ(ptr, nullptr);
}

TEST(List, findLastByExample)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };

	SomeClass<int>* ptr = list.findLast(SomeClass<int>(4));
	ASSERT_EQ(ptr, &list[3]);
	ptr = list.findLast(SomeClass<int>(2));
	ASSERT_EQ(ptr, &list[1]);
	ptr = list.findLast(SomeClass<int>(1));
	ASSERT_EQ(ptr, &list[2]);
	ptr = list.findLast(SomeClass<int>(7));
	ASSERT_EQ(ptr, &list[4]);
	ptr = list.findLast(SomeClass<int>(3));
	ASSERT_EQ(ptr, &list[5]);
	ptr = list.findLast(SomeClass<int>(100));
	ASSERT_EQ(ptr, nullptr);
}

TEST(List, findLastByPredicate)
{
	bbe::List<SomeClass<int>> list = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };

	SomeClass<int>* ptr = list.findLast([](const SomeClass<int>& s) { return s.getLength() == 4; });
	ASSERT_EQ(ptr, &list[3]);
	ptr = list.findLast([](const SomeClass<int>& s) { return s.getLength() == 2; });
	ASSERT_EQ(ptr, &list[1]);
	ptr = list.findLast([](const SomeClass<int>& s) { return s.getLength() == 1; });
	ASSERT_EQ(ptr, &list[2]);
	ptr = list.findLast([](const SomeClass<int>& s) { return s.getLength() == 7; });
	ASSERT_EQ(ptr, &list[4]);
	ptr = list.findLast([](const SomeClass<int>& s) { return s.getLength() == 3; });
	ASSERT_EQ(ptr, &list[5]);
	ptr = list.findLast([](const SomeClass<int>& s) { return s.getLength() == 100; });
	ASSERT_EQ(ptr, nullptr);
}

TEST(List, equalsOperator)
{
	bbe::List<SomeClass<int>> list1 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	bbe::List<SomeClass<int>> list2 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	bbe::List<SomeClass<int>> list3 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(5), SomeClass<int>(7), SomeClass<int>(3) };
	bbe::List<SomeClass<int>> list4 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(4) };
	bbe::List<SomeClass<int>> list5 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7) };

	ASSERT_EQ(list1 == list1, true);
	ASSERT_EQ(list1 == list2, true);
	ASSERT_EQ(list1 == list3, false);
	ASSERT_EQ(list1 == list4, false);
	ASSERT_EQ(list1 == list5, false);
}

TEST(List, unequalsOperator)
{
	bbe::List<SomeClass<int>> list1 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	bbe::List<SomeClass<int>> list2 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(3) };
	bbe::List<SomeClass<int>> list3 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(5), SomeClass<int>(7), SomeClass<int>(3) };
	bbe::List<SomeClass<int>> list4 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7), SomeClass<int>(4) };
	bbe::List<SomeClass<int>> list5 = { SomeClass<int>(4), SomeClass<int>(2), SomeClass<int>(1), SomeClass<int>(4), SomeClass<int>(7) };

	ASSERT_EQ(list1 != list1, false);
	ASSERT_EQ(list1 != list2, false);
	ASSERT_EQ(list1 != list3, true);
	ASSERT_EQ(list1 != list4, true);
	ASSERT_EQ(list1 != list5, true);
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
	ASSERT_EQ(list1.getLength(), 6);

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
	ASSERT_EQ(list2.getLength(), 6);

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
	ASSERT_EQ(list1.getLength(), 12);
}
