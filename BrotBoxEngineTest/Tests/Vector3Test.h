#pragma once

#include "BBE/Vector3.h"
#include "BBE/UtilTest.h"

namespace bbe
{
	namespace test
	{
		void testVector3()
		{
			{
				Vector3 point(500, 0, 0);
				Vector3 rotated = point.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1));
				assertEqualsFloat(rotated.x, 0);
				assertEqualsFloat(rotated.y, 500);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1));
				assertEqualsFloat(rotated.x, -500);
				assertEqualsFloat(rotated.y, 0);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1));
				assertEqualsFloat(rotated.x, 0);
				assertEqualsFloat(rotated.y, -500);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1));
				assertEqualsFloat(rotated.x, 500);
				assertEqualsFloat(rotated.y, 0);
			}

			{
				Vector3 point(500, 0, 0);
				Vector3 rotated = point.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3());
				assertEqualsFloat(rotated.x, 0);
				assertEqualsFloat(rotated.y, 500);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3());
				assertEqualsFloat(rotated.x, -500);
				assertEqualsFloat(rotated.y, 0);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3());
				assertEqualsFloat(rotated.x, 0);
				assertEqualsFloat(rotated.y, -500);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3());
				assertEqualsFloat(rotated.x, 500);
				assertEqualsFloat(rotated.y, 0);
			}

			{
				Vector3 point(600, 0, 0);
				Vector3 rotated = point.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3(100, 0, 0));
				assertEqualsFloat(rotated.x, 100);
				assertEqualsFloat(rotated.y, 500);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3(100, 0, 0));
				assertEqualsFloat(rotated.x, -400);
				assertEqualsFloat(rotated.y, 0);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3(100, 0, 0));
				assertEqualsFloat(rotated.x, 100);
				assertEqualsFloat(rotated.y, -500);

				rotated = rotated.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), Vector3(100, 0, 0));
				assertEqualsFloat(rotated.x, 600);
				assertEqualsFloat(rotated.y, 0);
			}

			{
				Vector3 point(1000, 500, 0);
				Vector3 center(500, 500, 0);
				Vector3 rotatedPoint = point.rotate(bbe::Math::PI / 2, Vector3(0, 0, 1), center);
				assertEqualsFloat(rotatedPoint.x, 500);
				assertEqualsFloat(rotatedPoint.y, 1000);
			}
		}
	}
}