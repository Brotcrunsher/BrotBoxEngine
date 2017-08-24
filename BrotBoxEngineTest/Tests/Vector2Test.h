#pragma once

#include "BBE/Vector2.h"
#include "BBE/UtilTest.h"


namespace bbe
{
	namespace test
	{
		void testVector2()
		{
			{
				Vector2 vec;
				assertEquals(vec.x, 0);
				assertEquals(vec.y, 0);
			}

			{
				Vector2 vec(1.0f, 2.0f);
				assertEquals(vec.x, 1.0f);
				assertEquals(vec.y, 2.0f);
			}

			{
				Vector2 vec = Vector2::createVector2OnUnitCircle(0);
				assertEquals(vec.x, 1.0f);
				assertEquals(vec.y, 0.0f);

				vec = Vector2::createVector2OnUnitCircle(1);
				assertEqualsFloat(vec.getLength(), 1.0);
				assertEquals(vec.isUnit(), true);
			}

			{
				Vector2 vec1(1, 2);
				Vector2 vec2(3, 4);

				assertEqualsFloat(vec1 * vec2, 11);

				Vector2 vec3 = vec1 + vec2;
				assertEquals(vec3.x, 4);
				assertEquals(vec3.y, 6);
				assertEquals(vec3[0], 4);
				assertEquals(vec3[1], 6);

				const Vector2 vec5 = vec3;
				assertEquals(vec5[0], 4);
				assertEquals(vec5[1], 6);

				Vector2 vec4 = vec1 - vec2;
				assertEquals(vec4.x, -2);
				assertEquals(vec4.y, -2);

				vec1 = vec1 * 5;
				assertEquals(vec1.x, 5);
				assertEquals(vec1.y, 10);

				vec2 = vec2 / 2;
				assertEquals(vec2.x, 1.5f);
				assertEquals(vec2.y, 2.0f);
			}

			{
				Vector2 vec1(1, 1);
				Vector2 vec2(1, 1);
				Vector2 vec3(1, 2);
				Vector2 vec4(2, 1);

				assertEquals(vec1 == vec2, true);
				assertEquals(vec2 == vec1, true);
				assertEquals(vec1 == vec3, false);
				assertEquals(vec3 == vec1, false);
				assertEquals(vec1 == vec4, false);
				assertEquals(vec4 == vec1, false);
				assertEquals(vec3 == vec4, false);
				assertEquals(vec4 == vec3, false);

				assertEquals(vec1 != vec2, false);
				assertEquals(vec2 != vec1, false);
				assertEquals(vec1 != vec3, true);
				assertEquals(vec3 != vec1, true);
				assertEquals(vec1 != vec4, true);
				assertEquals(vec4 != vec1, true);
				assertEquals(vec3 != vec4, true);
				assertEquals(vec4 != vec3, true);

				assertEquals(vec1 > vec2, false);
				assertEquals(vec1 < vec2, false);
				assertEquals(vec1 > vec3, false);
				assertEquals(vec1 < vec3, true);

				assertEquals(vec1 >= vec2, true);
				assertEquals(vec1 <= vec2, true);
				assertEquals(vec1 >= vec3, false);
				assertEquals(vec1 <= vec3, true);
			}

			{
				Vector2 vec1(1, 2);
				Vector2 vec2(1.0001f, 2.000002f);
				Vector2 vec3(3, 5);
				Vector2 vec4(-1, -2);

				assertEquals(vec1.equals(vec2), true);
				assertEquals(vec1.equals(vec3), false);
				assertEquals(vec1.equals(vec4), false);

				assertEquals(vec1.isSameLength(vec2), true);
				assertEquals(vec1.isSameLength(vec3), false);
				assertEquals(vec1.isSameLength(vec4), true);
			}

			{
				Vector2 forward(1, 0);
				Vector2 up(0, -1);
				Vector2 down(0, 1);
				Vector2 backward(-1, 0);
				Vector2 forward2(100, 0);
				Vector2 up2(0, -100);
				Vector2 down2(0, 100);
				Vector2 backward2(-100, 0);

				assertEquals(forward.isSameDirection(forward2),  true);
				assertEquals(forward.isSameDirection(up),        false);
				assertEquals(forward.isSameDirection(down),      false);
				assertEquals(forward.isSameDirection(backward),  false);
				assertEquals(forward.isSameDirection(up2),       false);
				assertEquals(forward.isSameDirection(down2),     false);
				assertEquals(forward.isSameDirection(backward2), false);

				assertEquals(forward.isOppositeDirection(forward2),  false);
				assertEquals(forward.isOppositeDirection(up),        false);
				assertEquals(forward.isOppositeDirection(down),      false);
				assertEquals(forward.isOppositeDirection(backward),  true);
				assertEquals(forward.isOppositeDirection(up2),       false);
				assertEquals(forward.isOppositeDirection(down2),     false);
				assertEquals(forward.isOppositeDirection(backward2), true);

				assertEquals(forward.isLeft(forward2),  false);
				assertEquals(forward.isLeft(up),        true);
				assertEquals(forward.isLeft(down),      false);
				assertEquals(forward.isLeft(backward),  false);
				assertEquals(forward.isLeft(up2),       true);
				assertEquals(forward.isLeft(down2),     false);
				assertEquals(forward.isLeft(backward2), false);

				assertEquals(forward.isRight(forward2),  false);
				assertEquals(forward.isRight(up),        false);
				assertEquals(forward.isRight(down),      true);
				assertEquals(forward.isRight(backward),  false);
				assertEquals(forward.isRight(up2),       false);
				assertEquals(forward.isRight(down2),     true);
				assertEquals(forward.isRight(backward2), false);
			}

			{
				Vector2 vec1(1, 0);
				Vector2 vec2(Math::NaN, 0);
				Vector2 vec3(0, Math::NaN);
				Vector2 vec4(Math::NaN, Math::NaN);

				assertEquals(vec1.isContainingNaN(), false);
				assertEquals(vec2.isContainingNaN(), true);
				assertEquals(vec3.isContainingNaN(), true);
				assertEquals(vec4.isContainingNaN(), true);
			}

			{
				Vector2 vec1(1, 0);
				Vector2 vec2(Math::INFINITY_POSITIVE, 0);
				Vector2 vec3(Math::INFINITY_NEGATIVE, 0);
				Vector2 vec4(0, Math::INFINITY_POSITIVE);
				Vector2 vec5(Math::INFINITY_POSITIVE, Math::INFINITY_POSITIVE);
				Vector2 vec6(Math::INFINITY_NEGATIVE, Math::INFINITY_POSITIVE);
				Vector2 vec7(0, Math::INFINITY_NEGATIVE);
				Vector2 vec8(Math::INFINITY_POSITIVE, Math::INFINITY_NEGATIVE);
				Vector2 vec9(Math::INFINITY_NEGATIVE, Math::INFINITY_NEGATIVE);

				assertEquals(vec1.isContainingInfinity(), false);
				assertEquals(vec2.isContainingInfinity(), true);
				assertEquals(vec3.isContainingInfinity(), true);
				assertEquals(vec4.isContainingInfinity(), true);
				assertEquals(vec5.isContainingInfinity(), true);
				assertEquals(vec6.isContainingInfinity(), true);
				assertEquals(vec7.isContainingInfinity(), true);
				assertEquals(vec8.isContainingInfinity(), true);
				assertEquals(vec9.isContainingInfinity(), true);
			}

			{
				for (int i = 0; i < 1000; i++)
				{
					Vector2 vec = Vector2::createVector2OnUnitCircle((float)i);
					assertEquals(vec.isUnit(), true);
				}

				Vector2 vec1(2, 0);
				Vector2 vec2(-2, 0);
				Vector2 vec3(0, 2);
				Vector2 vec4(0, -2);

				assertEquals(vec1.isUnit(), false);
				assertEquals(vec2.isUnit(), false);
				assertEquals(vec3.isUnit(), false);
				assertEquals(vec4.isUnit(), false);
			}

			{
				Vector2 vec1(1, 0);
				Vector2 vec2(2, 0);
				Vector2 vec3(3, 0);

				assertEquals(vec1.isCloseTo(vec2, 1), true);
				assertEquals(vec1.isCloseTo(vec3, 1), false);
				assertEquals(vec2.isCloseTo(vec3, 1), true);
			}

			{
				Vector2 vec1(0, 0);
				Vector2 vec2(0.0001f, 0);
				Vector2 vec3(0, 0.00001f);
				assertEquals(vec1.isZero(), true);
				assertEquals(vec2.isZero(), false);
				assertEquals(vec3.isZero(), false);
			}

			{
				Vector2 vec1(1, 0);
				Vector2 vec2 = vec1.rotate(0);
				Vector2 vec3 = vec1.rotate(Math::PI / 2);
				Vector2 vec4 = vec1.rotate(Math::PI);
				Vector2 vec5 = vec1.rotate(Math::PI * 3 / 2);
				Vector2 vec6 = vec1.rotate(Math::PI * 2);
				Vector2 vec7 = vec1.rotate(-Math::PI / 2);
				Vector2 vec8 = vec1.rotate(-Math::PI);
				Vector2 vec9 = vec1.rotate(-Math::PI * 3 / 2);
				Vector2 vec10 = vec1.rotate(-Math::PI * 2);

				assertEqualsFloat(vec2.x, 1);
				assertEqualsFloat(vec2.y, 0);
				assertEqualsFloat(vec3.x, 0);
				assertEqualsFloat(vec3.y, 1);
				assertEqualsFloat(vec4.x, -1);
				assertEqualsFloat(vec4.y, 0);
				assertEqualsFloat(vec5.x, 0);
				assertEqualsFloat(vec5.y, -1);
				assertEqualsFloat(vec6.x, 1);
				assertEqualsFloat(vec6.y, 0);
				assertEqualsFloat(vec7.x, 0);
				assertEqualsFloat(vec7.y, -1);
				assertEqualsFloat(vec8.x, -1);
				assertEqualsFloat(vec8.y, 0);
				assertEqualsFloat(vec9.x, 0);
				assertEqualsFloat(vec9.y, 1);
				assertEqualsFloat(vec10.x, 1);
				assertEqualsFloat(vec10.y, 0);
			}

			{
				Vector2 vec1(9, 9);
				Vector2 center(8, 9);
				Vector2 vec2 = vec1.rotate(Math::PI / 2, center);

				assertEqualsFloat(vec2.x, 8);
				assertEqualsFloat(vec2.y, 10);
			}

			{
				Vector2 vec1(1, 0);
				Vector2 vec2 = vec1.rotate90Clockwise();
				Vector2 vec3 = vec1.rotate90CounterClockwise();

				assertEquals(vec2.x, 0);
				assertEquals(vec2.y, 1);

				assertEquals(vec3.x, 0);
				assertEquals(vec3.y, -1);
			}

			{
				Vector2 vec1(2, 2);
				Vector2 vec2 = vec1.setLenght(16);

				assertEqualsFloat(vec2.getLength(), 16);
				assertEqualsFloat(vec2.x, 11.3137f);
				assertEqualsFloat(vec2.y, 11.3137f);
			}

			{
				Vector2 vec1(19, 2);
				Vector2 vec2 = vec1.normalize();

				assertEquals(vec2.isUnit(), true);
				assertEqualsFloat(vec2.x, 0.9945);
				assertEqualsFloat(vec2.y, 0.1046);
			}

			{
				Vector2 vec1(1, 1);
				Vector2 vec2(-1, 1);
				Vector2 vec3(1, -1);
				Vector2 vec4(-1, -1);

				vec1 = vec1.abs();
				vec2 = vec2.abs();
				vec3 = vec3.abs();
				vec4 = vec4.abs();

				assertEquals(vec1.x, 1);
				assertEquals(vec1.y, 1);
				assertEquals(vec2.x, 1);
				assertEquals(vec2.y, 1);
				assertEquals(vec3.x, 1);
				assertEquals(vec3.y, 1);
				assertEquals(vec4.x, 1);
				assertEquals(vec4.y, 1);
			}

			{
				Vector2 vec1(3, 4);
				Vector2 vec2(0, 4);
				Vector2 vec3(100, 4);
				Vector2 vec4(3, 0);
				Vector2 vec5(0, 0);
				Vector2 vec6(100, 0);
				Vector2 vec7(3, 100);
				Vector2 vec8(0, 100);
				Vector2 vec9(100, 100);

				vec1 = vec1.clampComponents(1, 10);
				vec2 = vec2.clampComponents(1, 10);
				vec3 = vec3.clampComponents(1, 10);
				vec4 = vec4.clampComponents(1, 10);
				vec5 = vec5.clampComponents(1, 10);
				vec6 = vec6.clampComponents(1, 10);
				vec7 = vec7.clampComponents(1, 10);
				vec8 = vec8.clampComponents(1, 10);
				vec9 = vec9.clampComponents(1, 10);

				assertEquals(vec1.x, 3);
				assertEquals(vec1.y, 4);
				assertEquals(vec2.x, 1);
				assertEquals(vec2.y, 4);
				assertEquals(vec3.x, 10);
				assertEquals(vec3.y, 4);

				assertEquals(vec4.x, 3);
				assertEquals(vec4.y, 1);
				assertEquals(vec5.x, 1);
				assertEquals(vec5.y, 1);
				assertEquals(vec6.x, 10);
				assertEquals(vec6.y, 1);

				assertEquals(vec7.x, 3);
				assertEquals(vec7.y, 10);
				assertEquals(vec8.x, 1);
				assertEquals(vec8.y, 10);
				assertEquals(vec9.x, 10);
				assertEquals(vec9.y, 10);
			}

			{
				Vector2 vec1(4, 2);
				Vector2 vec2(5, 8);

				assertEqualsFloat(vec1.getLength(), 4.47213);
				assertEquals(vec1.getLengthSq(), 20);
				assertEqualsFloat(vec1.getDistanceTo(vec2), 6.0827);
			}

			{
				Vector2 vec1(-3, 8);
				Vector2 vec2(8, -3);

				assertEquals(vec1.getMax(), 8);
				assertEquals(vec2.getMax(), 8);
				assertEquals(vec1.getMin(), -3);
				assertEquals(vec2.getMin(), -3);
			}

			{
				Vector2 vec1(-3, 8);
				Vector2 vec2(3, -8);

				assertEquals(vec1.getMaxAbs(), 8);
				assertEquals(vec2.getMaxAbs(), 8);
				assertEquals(vec1.getMinAbs(), 3);
				assertEquals(vec2.getMinAbs(), 3);
			}

			{
				Vector2 vec1(1, 1);
				Vector2 vec2(-1, -1);

				assertEqualsFloat(vec1.getAngle(), 0.785398);
				assertEqualsFloat(vec2.getAngle(), 3.926);
				assertEqualsFloat(vec1.getAngle(Vector2(1, 0)), 0.785398);
				assertEqualsFloat(vec2.getAngle(Vector2(1, 0)), 3.926);
				assertEqualsFloat(vec1.getAngle(vec2), Math::PI);
				assertEqualsFloat(vec2.getAngle(vec1), Math::PI);
				assertEqualsFloat(vec1.getAngle(Vector2(-1, 1)), 4.7123);
				assertEqualsFloat(vec2.getAngle(Vector2(-1, 1)), Math::PI / 2);

			}

			{
				Vector2 vec(5, 5);
				Vector2 proj(1, 0);

				Vector2 result = vec.project(proj);
				assertEqualsFloat(result.x, 5);
				assertEqualsFloat(result.y, 0);
			}

			{
				Vector2 vec(-1, -1);
				Vector2 normal(1, 0);
				Vector2 result = vec.reflect(normal);

				assertEqualsFloat(result.x, 1);
				assertEqualsFloat(result.y, -1);
			}

			//TODO test other stuff
		}
	}
}
