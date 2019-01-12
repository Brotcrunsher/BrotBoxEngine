#include "BBE/Math.h"
#include "BBE/UtilTest.h"

namespace bbe
{
	namespace test
	{
		void testMath()
		{
			bbe::Math::INTERNAL::startMath();

			assertEqualsFloat(Math::cos(0               ), 1);
			assertEqualsFloat(Math::cos(Math::PI / 2    ), 0);
			assertEqualsFloat(Math::cos(Math::PI        ), -1);
			assertEqualsFloat(Math::cos(Math::PI * 3 / 2), 0);
			assertEqualsFloat(Math::cos(-Math::PI / 2    ), 0);
			assertEqualsFloat(Math::cos(-Math::PI        ), -1);
			assertEqualsFloat(Math::cos(-Math::PI * 3 / 2), 0);

			assertEqualsFloat(Math::acos(-1), Math::PI);
			assertEqualsFloat(Math::acos(0), Math::PI / 2);
			assertEqualsFloat(Math::acos(1), 0);

			assertEqualsFloat(Math::sin(0               ), 0);
			assertEqualsFloat(Math::sin(Math::PI / 2    ), 1);
			assertEqualsFloat(Math::sin(Math::PI        ), 0);
			assertEqualsFloat(Math::sin(Math::PI * 3 / 2), -1);
			assertEqualsFloat(Math::sin(-Math::PI / 2    ), -1);
			assertEqualsFloat(Math::sin(-Math::PI        ), 0);
			assertEqualsFloat(Math::sin(-Math::PI * 3 / 2), 1);

			assertEqualsFloat(Math::asin(-1), -Math::PI / 2);
			assertEqualsFloat(Math::asin(0), 0);
			assertEqualsFloat(Math::asin(1), Math::PI / 2);

			assertEqualsFloat(Math::sqrt(0), 0);
			assertEqualsFloat(Math::sqrt(1), 1);
			assertEqualsFloat(Math::sqrt(100), 10);
			assertEqualsFloat(Math::sqrt(10000), 100);
			assertEqualsFloat(Math::sqrt(9), 3);
			assertEqualsFloat(Math::sqrt(2), Math::SQRT2);

			assertEqualsFloat(Math::mod(3.5f, 1.7f), 0.1f);
			assertEqualsFloat(Math::mod(2.f, 1.f), 0.f);
			
			assertEqualsFloat(Math::pingpong(1.0f, 1.0f), 1.0f);
			assertEqualsFloat(Math::pingpong(1.5f, 1.0f), 0.5f);
			assertEqualsFloat(Math::pingpong(2.0f, 1.0f), 0.0f);
			assertEqualsFloat(Math::pingpong(1.0f, 1.5f), 1.0f);
			assertEqualsFloat(Math::pingpong(1.5f, 1.5f), 1.5f);
			assertEqualsFloat(Math::pingpong(1.6f, 1.5f), 1.4f);

			assertEqualsFloat(Math::floor(0.0f), 0.0f);
			assertEqualsFloat(Math::floor(0.001f), 0.0f);
			assertEqualsFloat(Math::floor(0.5f), 0.0f);
			assertEqualsFloat(Math::floor(0.999f), 0.0f);
			assertEqualsFloat(Math::floor(1.0f), 1.0f);
			assertEqualsFloat(Math::floor(1.7f), 1.0f);
			assertEqualsFloat(Math::floor(35094.2f), 35094.0f);
			assertEqualsFloat(Math::floor(-0.1f), -1.0f);
			assertEqualsFloat(Math::floor(-0.999f), -1.0f);
			assertEqualsFloat(Math::floor(-1.0f), -1.0f);
			assertEqualsFloat(Math::floor(-1.001f), -2.0f);

			assertEqualsFloat(Math::ceil(0.0f), 0.0f);
			assertEqualsFloat(Math::ceil(0.001f), 1.0f);
			assertEqualsFloat(Math::ceil(0.5f), 1.0f);
			assertEqualsFloat(Math::ceil(0.999f), 1.0f);
			assertEqualsFloat(Math::ceil(1.0f), 1.0f);
			assertEqualsFloat(Math::ceil(1.7f), 2.0f);
			assertEqualsFloat(Math::ceil(35094.2f), 35095.0f);
			assertEqualsFloat(Math::ceil(-0.1f), 0.0f);
			assertEqualsFloat(Math::ceil(-0.999f), 0.0f);
			assertEqualsFloat(Math::ceil(-1.0f), -1.0f);
			assertEqualsFloat(Math::ceil(-1.001f), -1.0f);

			assertEqualsFloat(Math::round(0.0f), 0.0f);
			assertEqualsFloat(Math::round(0.001f), 0.0f);
			assertEqualsFloat(Math::round(0.5f), 1.0f);
			assertEqualsFloat(Math::round(0.999f), 1.0f);
			assertEqualsFloat(Math::round(1.0f), 1.0f);
			assertEqualsFloat(Math::round(1.7f), 2.0f);
			assertEqualsFloat(Math::round(35094.2f), 35094.0f);
			assertEqualsFloat(Math::round(-0.1f), 0.0f);
			assertEqualsFloat(Math::round(-0.999f), -1.0f);
			assertEqualsFloat(Math::round(-1.0f), -1.0f);
			assertEqualsFloat(Math::round(-1.001f), -1.0f);

			assertEqualsFloat(Math::square(0), 0);
			assertEqualsFloat(Math::square(1), 1);
			assertEqualsFloat(Math::square(2), 4);
			assertEqualsFloat(Math::square(3), 9);
			assertEqualsFloat(Math::square(-1), 1);
			assertEqualsFloat(Math::square(-2), 4);
			assertEqualsFloat(Math::square(-3), 9);

			assertEqualsFloat(Math::clamp(3.0f, 1.0f, 10.0f), 3.0f);
			assertEqualsFloat(Math::clamp(30.0f, 1.0f, 10.0f), 10.0f);
			assertEqualsFloat(Math::clamp(1.0f, 1.0f, 10.0f), 1.0f);
			assertEqualsFloat(Math::clamp(10.0f, 1.0f, 10.0f), 10.0f);
			assertEqualsFloat(Math::clamp(9.0f, 1.0f, 10.0f), 9.0f);
			assertEqualsFloat(Math::clamp(0.0f, 1.0f, 10.0f), 1.0f);
			assertEqualsFloat(Math::clamp(-1.0f, 1.0f, 10.0f), 1.0f);
			assertEqualsFloat(Math::clamp(Math::INFINITY_POSITIVE, 1.0f, 10.0f), 10.0f);
			assertEqualsFloat(Math::clamp(Math::INFINITY_NEGATIVE, 1.0f, 10.0f), 1.0f);
			assertEquals(Math::isNaN(Math::clamp(Math::NaN, 1.0f, 10.0f)), true);

			assertEqualsFloat(Math::clamp01(0.5f), 0.5f);
			assertEqualsFloat(Math::clamp01(1.0f), 1.0f);
			assertEqualsFloat(Math::clamp01(0.9f), 0.9f);
			assertEqualsFloat(Math::clamp01(1.5f), 1.0f);
			assertEqualsFloat(Math::clamp01(-0.5f), 0.0f);
			assertEqualsFloat(Math::clamp01(-1.5f), 0.0f);
			assertEqualsFloat(Math::clamp01(Math::INFINITY_POSITIVE), 1.0f);
			assertEqualsFloat(Math::clamp01(Math::INFINITY_NEGATIVE), 0.0f);

			assertEquals(Math::isInRange(0.5, 1.0, 2.0), false);
			assertEquals(Math::isInRange(1.0, 1.0, 2.0), true);
			assertEquals(Math::isInRange(1.5, 1.0, 2.0), true);
			assertEquals(Math::isInRange(2.0, 1.0, 2.0), true);
			assertEquals(Math::isInRange(2.5, 1.0, 2.0), false);
			assertEquals(Math::isInRange(3.5, 1.0, 2.0), false);
			assertEquals(Math::isInRange(4.5, 1.0, 2.0), false);

			assertEquals(Math::isInRangeStrict(0.5, 1.0, 2.0), false);
			assertEquals(Math::isInRangeStrict(1.0, 1.0, 2.0), false);
			assertEquals(Math::isInRangeStrict(1.5, 1.0, 2.0), true);
			assertEquals(Math::isInRangeStrict(2.0, 1.0, 2.0), false);
			assertEquals(Math::isInRangeStrict(2.5, 1.0, 2.0), false);
			assertEquals(Math::isInRangeStrict(3.5, 1.0, 2.0), false);
			assertEquals(Math::isInRangeStrict(4.5, 1.0, 2.0), false);

			assertEquals(Math::isInRange01(-0.1f), false);
			assertEquals(Math::isInRange01(0.0f), true);
			assertEquals(Math::isInRange01(0.5f), true);
			assertEquals(Math::isInRange01(1.0f), true);
			assertEquals(Math::isInRange01(1.5f), false);
			assertEquals(Math::isInRange01(2.0f), false);
			assertEquals(Math::isInRange01(2.5f), false);
			assertEquals(Math::isInRange01(3.5f), false);
			assertEquals(Math::isInRange01(4.5f), false);

			assertEquals(Math::isInRange01Strict(-0.1f), false);
			assertEquals(Math::isInRange01Strict(0.0f), false);
			assertEquals(Math::isInRange01Strict(0.5f), true);
			assertEquals(Math::isInRange01Strict(1.0f), false);
			assertEquals(Math::isInRange01Strict(1.5f), false);
			assertEquals(Math::isInRange01Strict(2.0f), false);
			assertEquals(Math::isInRange01Strict(2.5f), false);
			assertEquals(Math::isInRange01Strict(3.5f), false);
			assertEquals(Math::isInRange01Strict(4.5f), false);

			assertEqualsFloat(Math::abs(1.0), 1.0);
			assertEqualsFloat(Math::abs(0.0), 0.0);
			assertEqualsFloat(Math::abs(-1.0), 1.0);
			assertEquals(Math::abs(Math::INFINITY_POSITIVE) == Math::INFINITY_POSITIVE, true);
			assertEquals(Math::abs(Math::INFINITY_NEGATIVE) == Math::INFINITY_POSITIVE, true);

			assertEqualsFloat(Math::max(1.0, 0.0), 1.0);
			assertEqualsFloat(Math::max(0.0, 4.0), 4.0);

			assertEqualsFloat(Math::min(1.0, 0.0), 0.0);
			assertEqualsFloat(Math::min(1.0, 4.0), 1.0);

			assertEqualsFloat(Math::maxAbs(-3.0, 2.0), 3.0);
			assertEqualsFloat(Math::maxAbs(-2.0, 3.0), 3.0);

			assertEqualsFloat(Math::minAbs(-3.0, 2.0), 2.0);
			assertEqualsFloat(Math::minAbs(-2.0, 3.0), 2.0);

			assertEqualsFloat(Math::maxAbsKeepSign(-3.0, 2.0), -3.0);
			assertEqualsFloat(Math::maxAbsKeepSign(-2.0, 3.0), 3.0);

			assertEqualsFloat(Math::minAbsKeepSign(-3.0, 2.0), 2.0);
			assertEqualsFloat(Math::minAbsKeepSign(-2.0, 3.0), -2.0);

			assertEquals(Math::floatEquals(3, 3.0001f, 0.1f), true);
			assertEquals(Math::floatEquals(3, 3.1001f, 0.1f), false);

			assertEquals(Math::floatEquals(3.0001f, 3, 0.1f), true);
			assertEquals(Math::floatEquals(3.1001f, 3, 0.1f), false);

			assertEquals(Math::isNaN(0), false);
			assertEquals(Math::isNaN(10000), false);
			assertEquals(Math::isNaN(-10000), false);
			assertEquals(Math::isNaN(Math::PI), false);
			assertEquals(Math::isNaN(Math::INFINITY_POSITIVE), false);
			assertEquals(Math::isNaN(Math::INFINITY_NEGATIVE), false);
			assertEquals(Math::isNaN(Math::NaN), true);

			assertEquals(Math::isInfinity(0), false);
			assertEquals(Math::isInfinity(10000), false);
			assertEquals(Math::isInfinity(-10000), false);
			assertEquals(Math::isInfinity(Math::PI), false);
			assertEquals(Math::isInfinity(Math::INFINITY_POSITIVE), true);
			assertEquals(Math::isInfinity(Math::INFINITY_NEGATIVE), true);
			assertEquals(Math::isInfinity(Math::NaN), false);

			assertEquals(Math::isPositiveInfinity(0), false);
			assertEquals(Math::isPositiveInfinity(10000), false);
			assertEquals(Math::isPositiveInfinity(-10000), false);
			assertEquals(Math::isPositiveInfinity(Math::PI), false);
			assertEquals(Math::isPositiveInfinity(Math::INFINITY_POSITIVE), true);
			assertEquals(Math::isPositiveInfinity(Math::INFINITY_NEGATIVE), false);
			assertEquals(Math::isPositiveInfinity(Math::NaN), false);

			assertEquals(Math::isNegativeInfinity(0), false);
			assertEquals(Math::isNegativeInfinity(10000), false);
			assertEquals(Math::isNegativeInfinity(-10000), false);
			assertEquals(Math::isNegativeInfinity(Math::PI), false);
			assertEquals(Math::isNegativeInfinity(Math::INFINITY_POSITIVE), false);
			assertEquals(Math::isNegativeInfinity(Math::INFINITY_NEGATIVE), true);
			assertEquals(Math::isNegativeInfinity(Math::NaN), false);

			assertEquals(Math::nextMultiple(16, 31), 32);
			assertEquals(Math::nextMultiple(16, 32), 32);
			assertEquals(Math::nextMultiple(16, 33), 48);

			assertEquals(Math::isOdd(0 ), false);
			assertEquals(Math::isOdd(-1), true);
			assertEquals(Math::isOdd(1 ), true);
			assertEquals(Math::isOdd(-2), false);
			assertEquals(Math::isOdd(2 ), false);
			assertEquals(Math::isOdd(-3), true);
			assertEquals(Math::isOdd(3 ), true);

			assertEquals(Math::isEven(0 ), true);
			assertEquals(Math::isEven(-1), false);
			assertEquals(Math::isEven(1 ), false);
			assertEquals(Math::isEven(-2), true);
			assertEquals(Math::isEven(2 ), true);
			assertEquals(Math::isEven(-3), false);
			assertEquals(Math::isEven(3 ), false);
		}
	}
}