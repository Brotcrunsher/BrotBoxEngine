#pragma once

#include "BBE/Matrix4.h"
#include "BBE/UtilTest.h"

namespace bbe
{
	namespace test
	{
		void testMatrix4()
		{
			{
				Matrix4 mat;
				assertEquals(mat[0], 1);
				assertEquals(mat[1], 0);
				assertEquals(mat[2], 0);
				assertEquals(mat[3], 0);

				assertEquals(mat[4], 0);
				assertEquals(mat[5], 1);
				assertEquals(mat[6], 0);
				assertEquals(mat[7], 0);

				assertEquals(mat[8], 0);
				assertEquals(mat[9], 0);
				assertEquals(mat[10], 1);
				assertEquals(mat[11], 0);

				assertEquals(mat[12], 0);
				assertEquals(mat[13], 0);
				assertEquals(mat[14], 0);
				assertEquals(mat[15], 1);

				for (int i = 0; i < 4; i++)
				{
					for (int k = 0; k < 4; k++)
					{
						if (i == k)
						{
							assertEquals(mat.get(i, k), 1);
						}
						else
						{
							assertEquals(mat.get(i, k), 0);
						}
					}
				}


				float* data = reinterpret_cast<float*>(&mat);

				assertEquals(data[0], 1);
				assertEquals(data[1], 0);
				assertEquals(data[2], 0);
				assertEquals(data[3], 0);

				assertEquals(data[4], 0);
				assertEquals(data[5], 1);
				assertEquals(data[6], 0);
				assertEquals(data[7], 0);

				assertEquals(data[8], 0);
				assertEquals(data[9], 0);
				assertEquals(data[10], 1);
				assertEquals(data[11], 0);

				assertEquals(data[12], 0);
				assertEquals(data[13], 0);
				assertEquals(data[14], 0);
				assertEquals(data[15], 1);
			}

			{
				Matrix4 mat;
				mat[3] = 17;
				mat[6] = 42;
				mat[10] = 100;
				mat[12] = 2928;

				assertEquals(mat[0], 1);
				assertEquals(mat[1], 0);
				assertEquals(mat[2], 0);
				assertEquals(mat[3], 17);

				assertEquals(mat[4], 0);
				assertEquals(mat[5], 1);
				assertEquals(mat[6], 42);
				assertEquals(mat[7], 0);

				assertEquals(mat[8], 0);
				assertEquals(mat[9], 0);
				assertEquals(mat[10], 100);
				assertEquals(mat[11], 0);

				assertEquals(mat[12], 2928);
				assertEquals(mat[13], 0);
				assertEquals(mat[14], 0);
				assertEquals(mat[15], 1);
			}

			{
				Matrix4 mat;
				mat.set(3, 0, 17);
				mat.set(2, 1, 42);
				mat.set(2, 2, 100);
				mat.set(0, 3, 2928);

				assertEquals(mat[0], 1);
				assertEquals(mat[1], 0);
				assertEquals(mat[2], 0);
				assertEquals(mat[3], 17);

				assertEquals(mat[4], 0);
				assertEquals(mat[5], 1);
				assertEquals(mat[6], 42);
				assertEquals(mat[7], 0);

				assertEquals(mat[8], 0);
				assertEquals(mat[9], 0);
				assertEquals(mat[10], 100);
				assertEquals(mat[11], 0);

				assertEquals(mat[12], 2928);
				assertEquals(mat[13], 0);
				assertEquals(mat[14], 0);
				assertEquals(mat[15], 1);

				assertEquals(mat.get(0, 0), 1);
				assertEquals(mat.get(1, 0), 0);
				assertEquals(mat.get(2, 0), 0);
				assertEquals(mat.get(3, 0), 17);

				assertEquals(mat.get(0, 1), 0);
				assertEquals(mat.get(1, 1), 1);
				assertEquals(mat.get(2, 1), 42);
				assertEquals(mat.get(3, 1), 0);

				assertEquals(mat.get(0, 2), 0);
				assertEquals(mat.get(1, 2), 0);
				assertEquals(mat.get(2, 2), 100);
				assertEquals(mat.get(3, 2), 0);

				assertEquals(mat.get(0, 3), 2928);
				assertEquals(mat.get(1, 3), 0);
				assertEquals(mat.get(2, 3), 0);
				assertEquals(mat.get(3, 3), 1);
			}

			{
				const Matrix4 mat;
				assertEquals(mat[0], 1);
				assertEquals(mat[1], 0);
				assertEquals(mat[2], 0);
				assertEquals(mat[3], 0);

				assertEquals(mat[4], 0);
				assertEquals(mat[5], 1);
				assertEquals(mat[6], 0);
				assertEquals(mat[7], 0);

				assertEquals(mat[8], 0);
				assertEquals(mat[9], 0);
				assertEquals(mat[10], 1);
				assertEquals(mat[11], 0);

				assertEquals(mat[12], 0);
				assertEquals(mat[13], 0);
				assertEquals(mat[14], 0);
				assertEquals(mat[15], 1);
			}

			{
				Matrix4 m1;
				m1.set(0, 0, 5);
				m1.set(1, 1, 6);
				m1.set(2, 2, 7);

				Matrix4 m2;
				m2.set(0, 3, 2);
				m2.set(1, 3, 3);
				m2.set(2, 3, 4);

				Matrix4 m3 = m1 * m2;
				assertEquals(m3.get(0, 0), 5);
				assertEquals(m3.get(1, 0), 0);
				assertEquals(m3.get(2, 0), 0);
				assertEquals(m3.get(3, 0), 0);

				assertEquals(m3.get(0, 1), 0);
				assertEquals(m3.get(1, 1), 6);
				assertEquals(m3.get(2, 1), 0);
				assertEquals(m3.get(3, 1), 0);

				assertEquals(m3.get(0, 2), 0);
				assertEquals(m3.get(1, 2), 0);
				assertEquals(m3.get(2, 2), 7);
				assertEquals(m3.get(3, 2), 0);

				assertEquals(m3.get(0, 3), 10);
				assertEquals(m3.get(1, 3), 18);
				assertEquals(m3.get(2, 3), 28);
				assertEquals(m3.get(3, 3), 1);

				Matrix4 m4 = m2 * m1;
				assertEquals(m4.get(0, 0), 5);
				assertEquals(m4.get(1, 0), 0);
				assertEquals(m4.get(2, 0), 0);
				assertEquals(m4.get(3, 0), 0);
							  
				assertEquals(m4.get(0, 1), 0);
				assertEquals(m4.get(1, 1), 6);
				assertEquals(m4.get(2, 1), 0);
				assertEquals(m4.get(3, 1), 0);
							  
				assertEquals(m4.get(0, 2), 0);
				assertEquals(m4.get(1, 2), 0);
				assertEquals(m4.get(2, 2), 7);
				assertEquals(m4.get(3, 2), 0);
							  
				assertEquals(m4.get(0, 3), 2);
				assertEquals(m4.get(1, 3), 3);
				assertEquals(m4.get(2, 3), 4);
				assertEquals(m4.get(3, 3), 1);
			}
		}
	}
}