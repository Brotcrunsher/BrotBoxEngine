#include "gtest/gtest.h"
#include "BBE/Math.h"
#include "BBE/Vector2.h"

static constexpr float allowableError = 0.0001f;

TEST(Vector2, ConstructionDefault)
{
	for (int i = 0; i < 16; i++)
	{
		bbe::Vector2 a;
		ASSERT_EQ(a.x, 0);
		ASSERT_EQ(a.y, 0);
	}
}

TEST(Vector2, ConstructionXYSeparate)
{
	for (int i = 0; i < 16; i++)
	{
		bbe::Vector2 a(1337.f + i, 100.f / i);
		ASSERT_EQ(a.x, 1337.f + i);
		ASSERT_EQ(a.y, 100.f / i);
	}
}

TEST(Vector2, ConstructionXYSame)
{
	for (int i = 0; i < 16; i++)
	{
		bbe::Vector2 a(1337.f + i);
		ASSERT_EQ(a.x, 1337.f + i);
		ASSERT_EQ(a.y, 1337.f + i);
	}
}

TEST(Vector2, OnUnitCircle)
{
	for (int i = 0; i < 128; i++)
	{
		float circles = bbe::Math::PI * 2 * i;

		bbe::Vector2 a = bbe::Vector2::createVector2OnUnitCircle(circles);
		ASSERT_NEAR(a.x, 1, allowableError);
		ASSERT_NEAR(a.y, 0, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 1 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, bbe::Math::SQRT2INV, allowableError);
		ASSERT_NEAR(a.y, bbe::Math::SQRT2INV, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 2 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, 0, allowableError);
		ASSERT_NEAR(a.y, 1, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 3 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, -bbe::Math::SQRT2INV, allowableError);
		ASSERT_NEAR(a.y, bbe::Math::SQRT2INV, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 4 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, -1, allowableError);
		ASSERT_NEAR(a.y, 0, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 5 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, -bbe::Math::SQRT2INV, allowableError);
		ASSERT_NEAR(a.y, -bbe::Math::SQRT2INV, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 6 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, 0, allowableError);
		ASSERT_NEAR(a.y, -1, allowableError);

		a = bbe::Vector2::createVector2OnUnitCircle(circles + 7 * bbe::Math::PI / 4);
		ASSERT_NEAR(a.x, bbe::Math::SQRT2INV, allowableError);
		ASSERT_NEAR(a.y, -bbe::Math::SQRT2INV, allowableError);
	}
}

TEST(Vector2, OperatorAssignPlusVec2)
{
	bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(30, 20);
	a += b;
	ASSERT_NEAR(a.x, 31, allowableError);
	ASSERT_NEAR(a.y, 120, allowableError);
}

TEST(Vector2, OperatorAssignMinusVec2)
{
	bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(30, 20);
	a -= b;
	ASSERT_NEAR(a.x, -29, allowableError);
	ASSERT_NEAR(a.y, 80, allowableError);
}

TEST(Vector2, OperatorAssignMultVec2)
{
	bbe::Vector2 a(2, 100);
	const bbe::Vector2 b(30, 20);
	a *= b;
	ASSERT_NEAR(a.x, 60, allowableError);
	ASSERT_NEAR(a.y, 2000, allowableError);
}

TEST(Vector2, OperatorAssignDivVec2)
{
	bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(30, 20);
	a /= b;
	ASSERT_NEAR(a.x, 1.f / 30.f, allowableError);
	ASSERT_NEAR(a.y, 5, allowableError);
}

TEST(Vector2, OperatorAssignMultFloat)
{
	bbe::Vector2 a(2, 100);
	a *= 2;
	ASSERT_NEAR(a.x, 4, allowableError);
	ASSERT_NEAR(a.y, 200, allowableError);
}

TEST(Vector2, OperatorAssignDivFloat)
{
	bbe::Vector2 a(1, 100);
	a /= 2;
	ASSERT_NEAR(a.x, 0.5, allowableError);
	ASSERT_NEAR(a.y, 50, allowableError);
}

TEST(Vector2, OperatorMultFloat)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 c = a * 2;
	ASSERT_NEAR(c.x, 2, allowableError);
	ASSERT_NEAR(c.y, 200, allowableError);
}

TEST(Vector2, OperatorDivFloat)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 c = a / 2;
	ASSERT_NEAR(c.x, 0.5, allowableError);
	ASSERT_NEAR(c.y, 50, allowableError);
}

TEST(Vector2, OperatorDotProduct)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(40, 20);
	float dotProduct = a * b;
	ASSERT_NEAR(dotProduct, 2040, allowableError);
}

TEST(Vector2, OperatorPlusVec2)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(30, 20);
	const bbe::Vector2 c = a + b;
	ASSERT_NEAR(c.x, 31, allowableError);
	ASSERT_NEAR(c.y, 120, allowableError);
}

TEST(Vector2, OperatorMinusVec2)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(30, 20);
	const bbe::Vector2 c = a - b;
	ASSERT_NEAR(c.x, -29, allowableError);
	ASSERT_NEAR(c.y, 80, allowableError);
}

TEST(Vector2, OperatorUnaryMinus)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 b = -a;
	ASSERT_NEAR(b.x, -1, allowableError);
	ASSERT_NEAR(b.y, -100, allowableError);
}

TEST(Vector2, OperatorArray)
{
	bbe::Vector2 a(1, 100);
	ASSERT_NEAR(a[0], 1, allowableError);
	ASSERT_NEAR(a[1], 100, allowableError);
	ASSERT_THROW(a[2], bbe::IllegalIndexException);
	ASSERT_EQ(&a[0], &a.x);
	ASSERT_EQ(&a[1], &a.y);

	const bbe::Vector2 b(1, 100);
	ASSERT_NEAR(b[0], 1, allowableError);
	ASSERT_NEAR(b[1], 100, allowableError);
	ASSERT_THROW(b[2], bbe::IllegalIndexException);
	ASSERT_EQ(&b[0], &b.x);
	ASSERT_EQ(&b[1], &b.y);

	a[0] = 12;
	ASSERT_NEAR(a[0], 12, allowableError);
	ASSERT_NEAR(a[1], 100, allowableError);
	a[1] = 200;
	ASSERT_NEAR(a[0], 12, allowableError);
	ASSERT_NEAR(a[1], 200, allowableError);
}

TEST(Vector2, OperatorComparison)
{
	const bbe::Vector2 a(1, 100);
	const bbe::Vector2 b(1, 100);
	const bbe::Vector2 c(2, 100);
	const bbe::Vector2 d(1, 101);

	ASSERT_TRUE (a == a); ASSERT_TRUE (a == b); ASSERT_FALSE(a == c); ASSERT_FALSE(a == d);
	ASSERT_TRUE (b == a); ASSERT_TRUE (b == b); ASSERT_FALSE(b == c); ASSERT_FALSE(b == d);
	ASSERT_FALSE(c == a); ASSERT_FALSE(c == b); ASSERT_TRUE (c == c); ASSERT_FALSE(c == d);
	ASSERT_FALSE(d == a); ASSERT_FALSE(d == b); ASSERT_FALSE(d == c); ASSERT_TRUE (d == d);

	ASSERT_FALSE(a != a); ASSERT_FALSE(a != b); ASSERT_TRUE (a != c); ASSERT_TRUE (a != d);
	ASSERT_FALSE(b != a); ASSERT_FALSE(b != b); ASSERT_TRUE (b != c); ASSERT_TRUE (b != d);
	ASSERT_TRUE (c != a); ASSERT_TRUE (c != b); ASSERT_FALSE(c != c); ASSERT_TRUE (c != d);
	ASSERT_TRUE (d != a); ASSERT_TRUE (d != b); ASSERT_TRUE (d != c); ASSERT_FALSE(d != d);
}

TEST(Vector2, TestEquals)
{
	{
		const bbe::Vector2 a(1,      100);
		const bbe::Vector2 b(1,      100);
		const bbe::Vector2 c(1.01,   100);
		const bbe::Vector2 d(1.105,  100);

		ASSERT_TRUE (a.equals(a, 0.1f)); ASSERT_TRUE (a.equals(b, 0.1f)); ASSERT_TRUE (a.equals(c, 0.1f)); ASSERT_FALSE(a.equals(d, 0.1f));
		ASSERT_TRUE (b.equals(a, 0.1f)); ASSERT_TRUE (b.equals(b, 0.1f)); ASSERT_TRUE (b.equals(c, 0.1f)); ASSERT_FALSE(b.equals(d, 0.1f));
		ASSERT_TRUE (c.equals(a, 0.1f)); ASSERT_TRUE (c.equals(b, 0.1f)); ASSERT_TRUE (c.equals(c, 0.1f)); ASSERT_TRUE (c.equals(d, 0.1f));
		ASSERT_FALSE(d.equals(a, 0.1f)); ASSERT_FALSE(d.equals(b, 0.1f)); ASSERT_TRUE (d.equals(c, 0.1f)); ASSERT_TRUE (d.equals(d, 0.1f));
	}
	{
		//Same as above but coordinates have been flipped.
		const bbe::Vector2 a(100, 1);
		const bbe::Vector2 b(100, 1);
		const bbe::Vector2 c(100, 1.01);
		const bbe::Vector2 d(100, 1.105);

		ASSERT_TRUE (a.equals(a, 0.1f)); ASSERT_TRUE (a.equals(b, 0.1f)); ASSERT_TRUE (a.equals(c, 0.1f)); ASSERT_FALSE(a.equals(d, 0.1f));
		ASSERT_TRUE (b.equals(a, 0.1f)); ASSERT_TRUE (b.equals(b, 0.1f)); ASSERT_TRUE (b.equals(c, 0.1f)); ASSERT_FALSE(b.equals(d, 0.1f));
		ASSERT_TRUE (c.equals(a, 0.1f)); ASSERT_TRUE (c.equals(b, 0.1f)); ASSERT_TRUE (c.equals(c, 0.1f)); ASSERT_TRUE (c.equals(d, 0.1f));
		ASSERT_FALSE(d.equals(a, 0.1f)); ASSERT_FALSE(d.equals(b, 0.1f)); ASSERT_TRUE (d.equals(c, 0.1f)); ASSERT_TRUE (d.equals(d, 0.1f));
	}
}

TEST(Vector2, TestIsSameLength)
{
	const bbe::Vector2 a(100, 50);
	const bbe::Vector2 b(100, 50);
	const bbe::Vector2 c(80, 60);

	ASSERT_TRUE(a.isSameLength(a));
	ASSERT_TRUE(a.isSameLength(b));
	ASSERT_TRUE(b.isSameLength(a));
	ASSERT_FALSE(a.isSameLength(c));
	ASSERT_FALSE(c.isSameLength(a));

	ASSERT_FALSE(a.isSameLength(c, 49));
	ASSERT_TRUE(a.isSameLength(c, 50));
	ASSERT_TRUE(a.isSameLength(c, 51));
}

TEST(Vector2, TestIsSameDirection)
{
	const bbe::Vector2 a(1, 1);
	const bbe::Vector2 b(2, 1);
	const bbe::Vector2 c(-1, -1);

	ASSERT_TRUE(a.isSameDirection(a));
	ASSERT_TRUE(a.isSameDirection(b));
	ASSERT_FALSE(a.isSameDirection(c));

	ASSERT_FALSE(a.isOppositeDirection(a));
	ASSERT_FALSE(a.isOppositeDirection(b));
	ASSERT_TRUE(a.isOppositeDirection(c));
}

TEST(Vector2, TestLeftRight)
{
	const bbe::Vector2 a(1, 1);
	const bbe::Vector2 b(2, 1);
	const bbe::Vector2 c(-1, -1);

	ASSERT_FALSE(a.isLeft(a));
	ASSERT_TRUE (a.isLeft(b));
	ASSERT_FALSE(a.isLeft(c));
	ASSERT_FALSE(b.isLeft(a));
	ASSERT_TRUE (b.isLeft(c));
	ASSERT_FALSE(c.isLeft(b));

	ASSERT_FALSE(a.isRight(a));
	ASSERT_FALSE(a.isRight(b));
	ASSERT_FALSE(a.isRight(c));
	ASSERT_TRUE (b.isRight(a));
	ASSERT_FALSE(b.isRight(c));
	ASSERT_TRUE (c.isRight(b));
}

TEST(Vector2, TestContainingNaN)
{
	const bbe::Vector2 a(1, 1);
	const bbe::Vector2 b(bbe::Math::NaN, 1);
	const bbe::Vector2 c(1, bbe::Math::NaN);
	const bbe::Vector2 d(bbe::Math::NaN, bbe::Math::NaN);
	ASSERT_FALSE(a.isContainingNaN());
	ASSERT_TRUE(b.isContainingNaN());
	ASSERT_TRUE(c.isContainingNaN());
	ASSERT_TRUE(d.isContainingNaN());
}

TEST(Vector2, TestContainingInfinity)
{
	const bbe::Vector2 a0 (bbe::Math::NaN, 1);
	const bbe::Vector2 a1 (1, 1);
	const bbe::Vector2 a2 (bbe::Math::INFINITY_POSITIVE, 1);
	const bbe::Vector2 a3 (bbe::Math::INFINITY_POSITIVE, bbe::Math::NaN);
	const bbe::Vector2 a4 (bbe::Math::INFINITY_NEGATIVE, 1);
	const bbe::Vector2 a5 (1, bbe::Math::INFINITY_POSITIVE);
	const bbe::Vector2 a6 (1, bbe::Math::INFINITY_NEGATIVE);
	const bbe::Vector2 a7 (bbe::Math::INFINITY_POSITIVE, bbe::Math::INFINITY_POSITIVE);
	const bbe::Vector2 a8 (bbe::Math::INFINITY_NEGATIVE, bbe::Math::INFINITY_POSITIVE);
	const bbe::Vector2 a9 (bbe::Math::INFINITY_POSITIVE, bbe::Math::INFINITY_NEGATIVE);
	const bbe::Vector2 a10(bbe::Math::INFINITY_NEGATIVE, bbe::Math::INFINITY_NEGATIVE);
	ASSERT_FALSE(a0 .isContainingInfinity());
	ASSERT_FALSE(a1 .isContainingInfinity());
	ASSERT_TRUE (a2 .isContainingInfinity());
	ASSERT_TRUE (a3 .isContainingInfinity());
	ASSERT_TRUE (a4 .isContainingInfinity());
	ASSERT_TRUE (a5 .isContainingInfinity());
	ASSERT_TRUE (a6 .isContainingInfinity());
	ASSERT_TRUE (a7 .isContainingInfinity());
	ASSERT_TRUE (a8 .isContainingInfinity());
	ASSERT_TRUE (a9 .isContainingInfinity());
	ASSERT_TRUE (a10.isContainingInfinity());
}

TEST(Vector2, TestIsUnit)
{
	for (float rad = 0; rad < bbe::Math::TAU; rad += 0.001)
	{
		const bbe::Vector2 a = bbe::Vector2::createVector2OnUnitCircle(rad);
		ASSERT_TRUE(a.isUnit());
		ASSERT_FALSE((a * 0.99).isUnit());
		ASSERT_FALSE((a * 1.01).isUnit());
	}
}

TEST(Vector2, TestIsCloseTo)
{
	const bbe::Vector2 a(100, 90);
	const bbe::Vector2 b(100, 80);
	const bbe::Vector2 c(100, 70);

	ASSERT_TRUE(a.isCloseTo(a, 10));
	ASSERT_TRUE(a.isCloseTo(b, 10));
	ASSERT_FALSE(a.isCloseTo(c, 10));

	ASSERT_TRUE(b.isCloseTo(a, 10));
	ASSERT_TRUE(b.isCloseTo(b, 10));
	ASSERT_TRUE(b.isCloseTo(c, 10));

	ASSERT_FALSE(c.isCloseTo(a, 10));
	ASSERT_TRUE(c.isCloseTo(b, 10));
	ASSERT_TRUE(c.isCloseTo(c, 10));
}

TEST(Vector2, TestZero)
{
	const bbe::Vector2 a(0, 0);
	const bbe::Vector2 b(0.01, 0);
	const bbe::Vector2 c(0, 0.01);
	const bbe::Vector2 d(0.01, 0.01);

	ASSERT_TRUE(a.isZero());
	ASSERT_FALSE(b.isZero());
	ASSERT_FALSE(c.isZero());
	ASSERT_FALSE(d.isZero());
}

TEST(Vector2, TestRotate)
{
	const bbe::Vector2 sa(1, 0);
	const bbe::Vector2 sb = sa.rotate(bbe::Math::PI / 2);
	ASSERT_NEAR(sb.x, 0, allowableError);
	ASSERT_NEAR(sb.y, 1, allowableError);

	for (float rad = 0; rad < bbe::Math::TAU * 10; rad += 0.001)
	{
		const bbe::Vector2 a = bbe::Vector2::createVector2OnUnitCircle(rad) * rad;
		const bbe::Vector2 b = bbe::Vector2(rad, 0).rotate(rad);
		ASSERT_TRUE(a == b);
	}
}

TEST(Vector2, TestRotateAround)
{
	const bbe::Vector2 sa(1, 0);
	const bbe::Vector2 sb = sa.rotate(bbe::Math::PI / 2, bbe::Vector2(1, 1));
	ASSERT_NEAR(sb.x, 2, allowableError);
	ASSERT_NEAR(sb.y, 1, allowableError);

	for (float rad = 0; rad < bbe::Math::TAU * 10; rad += 0.001)
	{
		const bbe::Vector2 center = bbe::Vector2(rad, rad * 2);
		const bbe::Vector2 base = bbe::Vector2(rad, 0);
		const bbe::Vector2 a = (base - center).rotate(rad) + center;
		const bbe::Vector2 b = base.rotate(rad, center);
		ASSERT_TRUE(a == b);
	}
}

TEST(Vector2, TestRotate90)
{
	const bbe::Vector2 a(100, 1);
	const bbe::Vector2 b(0, 0);

	const bbe::Vector2 a1 = a.rotate90Clockwise();
	ASSERT_NEAR(a1.x, -1, allowableError);
	ASSERT_NEAR(a1.y, 100, allowableError);
	const bbe::Vector2 a2 = a.rotate90CounterClockwise();
	ASSERT_NEAR(a2.x, 1, allowableError);
	ASSERT_NEAR(a2.y, -100, allowableError);

	const bbe::Vector2 b1 = b.rotate90Clockwise();
	ASSERT_NEAR(b1.x, 0, allowableError);
	ASSERT_NEAR(b1.y, 0, allowableError);
	const bbe::Vector2 b2 = b.rotate90CounterClockwise();
	ASSERT_NEAR(b2.x, 0, allowableError);
	ASSERT_NEAR(b2.y, 0, allowableError);
}

TEST(Vector2, TestSetLength)
{
	for (int i = 0; i < 16; i++)
	{
		const bbe::Vector2 a(i, 10);
		const bbe::Vector2 b = a.withLenght(i);

		ASSERT_EQ(b, a.normalize() * i);
		ASSERT_NEAR(b.getLength(), i, allowableError);
	}
}

TEST(Vector2, TestNormalize)
{
	const bbe::Vector2 zero(0, 0);
	const bbe::Vector2 normalized = zero.normalize();
	ASSERT_EQ(normalized.x, 1);
	ASSERT_EQ(normalized.y, 0);
	const bbe::Vector2 normalized2 = zero.normalize(bbe::Vector2(100, 50));
	ASSERT_EQ(normalized2.x, 100);
	ASSERT_EQ(normalized2.y, 50);


	for (float rad = 0.001f; rad < bbe::Math::PI * 2; rad += 0.001f)
	{
		const bbe::Vector2 base = bbe::Vector2::createVector2OnUnitCircle(rad) * rad;
		const bbe::Vector2 a = base.normalize();
		const bbe::Vector2 b = base / base.getLength();
		ASSERT_TRUE(a.isUnit());
		ASSERT_EQ(a, b);
	}
}

TEST(Vector2, TestAbs)
{
	{
		const bbe::Vector2 abs = bbe::Vector2(1, 100).abs();
		ASSERT_NEAR(abs.x, 1, allowableError);
		ASSERT_NEAR(abs.y, 100, allowableError);
	}
	{
		const bbe::Vector2 abs = bbe::Vector2(-1, 100).abs();
		ASSERT_NEAR(abs.x, 1, allowableError);
		ASSERT_NEAR(abs.y, 100, allowableError);
	}
	{
		const bbe::Vector2 abs = bbe::Vector2(1, -100).abs();
		ASSERT_NEAR(abs.x, 1, allowableError);
		ASSERT_NEAR(abs.y, 100, allowableError);
	}
	{
		const bbe::Vector2 abs = bbe::Vector2(-1, -100).abs();
		ASSERT_NEAR(abs.x, 1, allowableError);
		ASSERT_NEAR(abs.y, 100, allowableError);
	}
}

TEST(Vector2, TestClampComponents)
{
	{
		const bbe::Vector2 clamp = bbe::Vector2(1, 100).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 1, allowableError);
		ASSERT_NEAR(clamp.y, 10, allowableError);
	}
	{
		const bbe::Vector2 clamp = bbe::Vector2(-1, 100).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 0, allowableError);
		ASSERT_NEAR(clamp.y, 10, allowableError);
	}
	{
		const bbe::Vector2 clamp = bbe::Vector2(1, 2).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 1, allowableError);
		ASSERT_NEAR(clamp.y, 2, allowableError);
	}
	{
		const bbe::Vector2 clamp = bbe::Vector2(-101, -202).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 0, allowableError);
		ASSERT_NEAR(clamp.y, 0, allowableError);
	}
	{
		const bbe::Vector2 clamp = bbe::Vector2(101, -202).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 10, allowableError);
		ASSERT_NEAR(clamp.y, 0, allowableError);
	}
	{
		const bbe::Vector2 clamp = bbe::Vector2(-101, 202).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 0, allowableError);
		ASSERT_NEAR(clamp.y, 10, allowableError);
	}
	{
		const bbe::Vector2 clamp = bbe::Vector2(101, 202).clampComponents(0, 10);
		ASSERT_NEAR(clamp.x, 10, allowableError);
		ASSERT_NEAR(clamp.y, 10, allowableError);
	}
}

TEST(Vector2, TestProject)
{
	const bbe::Vector2 toProject(100, 200);
	const bbe::Vector2 projector(10, 30);

	const bbe::Vector2 result = toProject.project(projector);
	ASSERT_NEAR(result.x, 70, allowableError);
	ASSERT_NEAR(result.y, 210, allowableError);
}

TEST(Vector2, TestReflect)
{
	const bbe::Vector2 toReflect(100, 200);
	const bbe::Vector2 reflector(10, 30);

	const bbe::Vector2 result = toReflect.reflect(reflector);
	ASSERT_NEAR(result.x, -40, allowableError);
	ASSERT_NEAR(result.y, -220, allowableError);
}

TEST(Vector2, TestGetLength)
{
	ASSERT_NEAR(bbe::Vector2(10, 10).getLength(), 14.14213562373095048801688724209698078569671875376948073176, allowableError);
	ASSERT_NEAR(bbe::Vector2(20, 10).getLength(), 22.36067977499789696409173668731276235440618359611525724270, allowableError);
	ASSERT_NEAR(bbe::Vector2(0, 0).getLength(), 0, allowableError);
	ASSERT_NEAR(bbe::Vector2(-10, 10).getLength(), 14.14213562373095048801688724209698078569671875376948073176, allowableError);
	ASSERT_NEAR(bbe::Vector2(-20, 10).getLength(), 22.36067977499789696409173668731276235440618359611525724270, allowableError);
	ASSERT_NEAR(bbe::Vector2(10, -10).getLength(), 14.14213562373095048801688724209698078569671875376948073176, allowableError);
	ASSERT_NEAR(bbe::Vector2(20, -10).getLength(), 22.36067977499789696409173668731276235440618359611525724270, allowableError);
	ASSERT_NEAR(bbe::Vector2(-10, -10).getLength(), 14.14213562373095048801688724209698078569671875376948073176, allowableError);
	ASSERT_NEAR(bbe::Vector2(-20, -10).getLength(), 22.36067977499789696409173668731276235440618359611525724270, allowableError);
}

TEST(Vector2, TestGetLengthSq)
{
	ASSERT_NEAR(bbe::Vector2(10, 10).getLengthSq(), 200, allowableError);
	ASSERT_NEAR(bbe::Vector2(20, 10).getLengthSq(), 500, allowableError);
	ASSERT_NEAR(bbe::Vector2(0, 0).getLengthSq(), 0, allowableError);
	ASSERT_NEAR(bbe::Vector2(-10, 10).getLengthSq(), 200, allowableError);
	ASSERT_NEAR(bbe::Vector2(-20, 10).getLengthSq(), 500, allowableError);
	ASSERT_NEAR(bbe::Vector2(10, -10).getLengthSq(), 200, allowableError);
	ASSERT_NEAR(bbe::Vector2(20, -10).getLengthSq(), 500, allowableError);
	ASSERT_NEAR(bbe::Vector2(-10, -10).getLengthSq(), 200, allowableError);
	ASSERT_NEAR(bbe::Vector2(-20, -10).getLengthSq(), 500, allowableError);
}

TEST(Vector2, TestGetDistanceTo)
{
	{
		const bbe::Vector2 a(100, 200);
		const bbe::Vector2 b(100, 400);
		ASSERT_NEAR(a.getDistanceTo(b), (a - b).getLength(), allowableError);
		ASSERT_NEAR(a.getDistanceTo(100, 400), (a - b).getLength(), allowableError);
	}
	{
		const bbe::Vector2 a(-100, 200);
		const bbe::Vector2 b(10, 20);
		ASSERT_NEAR(a.getDistanceTo(b), (a - b).getLength(), allowableError);
		ASSERT_NEAR(a.getDistanceTo(10, 20), (a - b).getLength(), allowableError);
	}
}

TEST(Vector2, TestGetMax)
{
	{
		const bbe::Vector2 a(10, 10);
		ASSERT_EQ(a.getMax(), 10);
	}
	{
		const bbe::Vector2 a(10, 20);
		ASSERT_EQ(a.getMax(), 20);
	}
	{
		const bbe::Vector2 a(20, 10);
		ASSERT_EQ(a.getMax(), 20);
	}
	{
		const bbe::Vector2 a(20, 20);
		ASSERT_EQ(a.getMax(), 20);
	}
	{
		const bbe::Vector2 a(-10, -10);
		ASSERT_EQ(a.getMax(), -10);
	}
	{
		const bbe::Vector2 a(-10, -20);
		ASSERT_EQ(a.getMax(), -10);
	}
	{
		const bbe::Vector2 a(-20, -10);
		ASSERT_EQ(a.getMax(), -10);
	}
	{
		const bbe::Vector2 a(-20, -20);
		ASSERT_EQ(a.getMax(), -20);
	}
}

TEST(Vector2, TestGetMaxAbs)
{
	{
		const bbe::Vector2 a(10, 10);
		ASSERT_EQ(a.getMaxAbs(), 10);
	}
	{
		const bbe::Vector2 a(10, 20);
		ASSERT_EQ(a.getMaxAbs(), 20);
	}
	{
		const bbe::Vector2 a(20, 10);
		ASSERT_EQ(a.getMaxAbs(), 20);
	}
	{
		const bbe::Vector2 a(20, 20);
		ASSERT_EQ(a.getMaxAbs(), 20);
	}
	{
		const bbe::Vector2 a(-10, -10);
		ASSERT_EQ(a.getMaxAbs(), 10);
	}
	{
		const bbe::Vector2 a(-10, -20);
		ASSERT_EQ(a.getMaxAbs(), 20);
	}
	{
		const bbe::Vector2 a(-20, -10);
		ASSERT_EQ(a.getMaxAbs(), 20);
	}
	{
		const bbe::Vector2 a(-20, -20);
		ASSERT_EQ(a.getMaxAbs(), 20);
	}
}

TEST(Vector2, TestGetMaxAbsKeepSign)
{
	{
		const bbe::Vector2 a(10, 10);
		ASSERT_EQ(a.getMaxAbsKeepSign(), 10);
	}
	{
		const bbe::Vector2 a(10, 20);
		ASSERT_EQ(a.getMaxAbsKeepSign(), 20);
	}
	{
		const bbe::Vector2 a(20, 10);
		ASSERT_EQ(a.getMaxAbsKeepSign(), 20);
	}
	{
		const bbe::Vector2 a(20, 20);
		ASSERT_EQ(a.getMaxAbsKeepSign(), 20);
	}
	{
		const bbe::Vector2 a(-10, -10);
		ASSERT_EQ(a.getMaxAbsKeepSign(), -10);
	}
	{
		const bbe::Vector2 a(-10, -20);
		ASSERT_EQ(a.getMaxAbsKeepSign(), -20);
	}
	{
		const bbe::Vector2 a(-20, -10);
		ASSERT_EQ(a.getMaxAbsKeepSign(), -20);
	}
	{
		const bbe::Vector2 a(-20, -20);
		ASSERT_EQ(a.getMaxAbsKeepSign(), -20);
	}
}

TEST(Vector2, TestGetMin)
{
	{
		const bbe::Vector2 a(10, 10);
		ASSERT_EQ(a.getMin(), 10);
	}
	{
		const bbe::Vector2 a(10, 20);
		ASSERT_EQ(a.getMin(), 10);
	}
	{
		const bbe::Vector2 a(20, 10);
		ASSERT_EQ(a.getMin(), 10);
	}
	{
		const bbe::Vector2 a(20, 20);
		ASSERT_EQ(a.getMin(), 20);
	}
	{
		const bbe::Vector2 a(-10, -10);
		ASSERT_EQ(a.getMin(), -10);
	}
	{
		const bbe::Vector2 a(-10, -20);
		ASSERT_EQ(a.getMin(), -20);
	}
	{
		const bbe::Vector2 a(-20, -10);
		ASSERT_EQ(a.getMin(), -20);
	}
	{
		const bbe::Vector2 a(-20, -20);
		ASSERT_EQ(a.getMin(), -20);
	}
}

TEST(Vector2, TestGetMinAbs)
{
	{
		const bbe::Vector2 a(10, 10);
		ASSERT_EQ(a.getMinAbs(), 10);
	}
	{
		const bbe::Vector2 a(10, 20);
		ASSERT_EQ(a.getMinAbs(), 10);
	}
	{
		const bbe::Vector2 a(20, 10);
		ASSERT_EQ(a.getMinAbs(), 10);
	}
	{
		const bbe::Vector2 a(20, 20);
		ASSERT_EQ(a.getMinAbs(), 20);
	}
	{
		const bbe::Vector2 a(-10, -10);
		ASSERT_EQ(a.getMinAbs(), 10);
	}
	{
		const bbe::Vector2 a(-10, -20);
		ASSERT_EQ(a.getMinAbs(), 10);
	}
	{
		const bbe::Vector2 a(-20, -10);
		ASSERT_EQ(a.getMinAbs(), 10);
	}
	{
		const bbe::Vector2 a(-20, -20);
		ASSERT_EQ(a.getMinAbs(), 20);
	}
}

TEST(Vector2, TestGetMinAbsKeepSign)
{
	{
		const bbe::Vector2 a(10, 10);
		ASSERT_EQ(a.getMinAbsKeepSign(), 10);
	}
	{
		const bbe::Vector2 a(10, 20);
		ASSERT_EQ(a.getMinAbsKeepSign(), 10);
	}
	{
		const bbe::Vector2 a(20, 10);
		ASSERT_EQ(a.getMinAbsKeepSign(), 10);
	}
	{
		const bbe::Vector2 a(20, 20);
		ASSERT_EQ(a.getMinAbsKeepSign(), 20);
	}
	{
		const bbe::Vector2 a(-10, -10);
		ASSERT_EQ(a.getMinAbsKeepSign(), -10);
	}
	{
		const bbe::Vector2 a(-10, -20);
		ASSERT_EQ(a.getMinAbsKeepSign(), -10);
	}
	{
		const bbe::Vector2 a(-20, -10);
		ASSERT_EQ(a.getMinAbsKeepSign(), -10);
	}
	{
		const bbe::Vector2 a(-20, -20);
		ASSERT_EQ(a.getMinAbsKeepSign(), -20);
	}
}

TEST(Vector2, TestGetAngle)
{
	for (float rad = 0.001f; rad < bbe::Math::PI * 2; rad += 0.001f)
	{
		const bbe::Vector2 a = bbe::Vector2::createVector2OnUnitCircle(rad) * rad;
		ASSERT_NEAR(a.getAngle(), rad, 0.01f);
	}
	for (float rad = 0.001f; rad < bbe::Math::PI * 2; rad += 0.001f)
	{
		const bbe::Vector2 a = bbe::Vector2::createVector2OnUnitCircle(rad) * rad;
		ASSERT_NEAR(a.getAngle(bbe::Vector2(1, 0)), rad, 0.01f);
	}
}

TEST(Vector2, TestSwizzle)
{
	const bbe::Vector2 a(10, 20);
	const bbe::Vector2 xx = a.xx();
	ASSERT_EQ(xx.x, 10);
	ASSERT_EQ(xx.y, 10);
	const bbe::Vector2 xy = a.xy();
	ASSERT_EQ(xy.x, 10);
	ASSERT_EQ(xy.y, 20);
	const bbe::Vector2 yx = a.yx();
	ASSERT_EQ(yx.x, 20);
	ASSERT_EQ(yx.y, 10);
	const bbe::Vector2 yy = a.yy();
	ASSERT_EQ(yy.x, 20);
	ASSERT_EQ(yy.y, 20);
}
