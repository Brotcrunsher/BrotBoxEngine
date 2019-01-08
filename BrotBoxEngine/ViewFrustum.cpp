#include "stdafx.h"
#include "BBE/ViewFrustum.h"

void bbe::INTERNAL::vulkan::ViewFrustum::updatePlanes(const bbe::Matrix4 &mvpMatrix)
{
	planes[0].x = mvpMatrix.get(3, 0) + mvpMatrix.get(0, 0);
	planes[0].y = mvpMatrix.get(3, 1) + mvpMatrix.get(0, 1);
	planes[0].z = mvpMatrix.get(3, 2) + mvpMatrix.get(0, 2);
	planes[0].w = mvpMatrix.get(3, 3) + mvpMatrix.get(0, 3);

	planes[1].x = mvpMatrix.get(3, 0) - mvpMatrix.get(0, 0);
	planes[1].y = mvpMatrix.get(3, 1) - mvpMatrix.get(0, 1);
	planes[1].z = mvpMatrix.get(3, 2) - mvpMatrix.get(0, 2);
	planes[1].w = mvpMatrix.get(3, 3) - mvpMatrix.get(0, 3);

	planes[2].x = mvpMatrix.get(3, 0) - mvpMatrix.get(1, 0);
	planes[2].y = mvpMatrix.get(3, 1) - mvpMatrix.get(1, 1);
	planes[2].z = mvpMatrix.get(3, 2) - mvpMatrix.get(1, 2);
	planes[2].w = mvpMatrix.get(3, 3) - mvpMatrix.get(1, 3);

	planes[3].x = mvpMatrix.get(3, 0) + mvpMatrix.get(1, 0);
	planes[3].y = mvpMatrix.get(3, 1) + mvpMatrix.get(1, 1);
	planes[3].z = mvpMatrix.get(3, 2) + mvpMatrix.get(1, 2);
	planes[3].w = mvpMatrix.get(3, 3) + mvpMatrix.get(1, 3);

	planes[4].x = mvpMatrix.get(3, 0) + mvpMatrix.get(2, 0);
	planes[4].y = mvpMatrix.get(3, 1) + mvpMatrix.get(2, 1);
	planes[4].z = mvpMatrix.get(3, 2) + mvpMatrix.get(2, 2);
	planes[4].w = mvpMatrix.get(3, 3) + mvpMatrix.get(2, 3);

	for (int i = 0; i < 5; i++)
	{
		planes[i] = planes[i].normalizeXYZ();
	}
}

bbe::Vector4 * bbe::INTERNAL::vulkan::ViewFrustum::getPlanes()
{
	return planes;
}
