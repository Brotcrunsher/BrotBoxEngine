#include "stdafx.h"
#include "BBE/Matrix4.h"

bbe::Matrix4::Matrix4()
{
	for (int i = 0; i < 4; i++)
	{
		col0[i] = 0;
		col1[i] = 0;
		col2[i] = 0;
		col3[i] = 0;
	}

	col0[0] = 1;
	col1[1] = 1;
	col2[2] = 1;
	col3[3] = 1;
}
