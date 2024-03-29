﻿#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"
#include "TestUtils.h"
#include <set>

TEST(Math, squareCantor)
{
	ASSERT_EQ(bbe::Vector2i(0, 0), bbe::Math::squareCantor(0));
	ASSERT_EQ(bbe::Vector2i(1, 0), bbe::Math::squareCantor(1));
	ASSERT_EQ(bbe::Vector2i(1, 1), bbe::Math::squareCantor(2));
	ASSERT_EQ(bbe::Vector2i(0, 1), bbe::Math::squareCantor(3));
	ASSERT_EQ(bbe::Vector2i(2, 0), bbe::Math::squareCantor(4));
	ASSERT_EQ(bbe::Vector2i(2, 1), bbe::Math::squareCantor(5));
	ASSERT_EQ(bbe::Vector2i(2, 2), bbe::Math::squareCantor(6));
	ASSERT_EQ(bbe::Vector2i(0, 2), bbe::Math::squareCantor(7));
	ASSERT_EQ(bbe::Vector2i(1, 2), bbe::Math::squareCantor(8));
	ASSERT_EQ(bbe::Vector2i(3, 0), bbe::Math::squareCantor(9));

	ASSERT_EQ(bbe::Vector2i(7, 31), bbe::Math::squareCantor(1000));

	std::set<bbe::Vector2i> vectors;
	for (uint32_t i = 0; i < 10000; i++)
	{
		bbe::Vector2i newVec = bbe::Math::squareCantor(i);
		auto find = vectors.find(newVec);
		ASSERT_EQ(vectors.count(newVec), 0);
		vectors.insert(newVec);
	}
}

TEST(Rectangle, packing)
{
	bbe::Random rand;
	for (uint32_t repeats = 0; repeats < 1000; repeats++)
	{
		bbe::List<bbe::Rectanglei> rects;
		for (uint32_t i = 0; i < 32; i++)
		{
			rects.add(bbe::Rectanglei(
				rand.randomInt(100),
				rand.randomInt(100),
				rand.randomInt(100),
				rand.randomInt(100)
			));
		}
		bbe::Vector2i boundingBox = bbe::Rectanglei::pack(rects);

		for (uint32_t i = 0; i < rects.getLength(); i++)
		{
			for (uint32_t k = i + 1; k < rects.getLength(); k++)
			{
				ASSERT_FALSE(rects[i].intersects(rects[k]));
			}
		}
	}
}
