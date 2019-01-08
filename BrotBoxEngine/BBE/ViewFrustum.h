#pragma once

#include "../BBE/Vector4.h"
#include "../BBE/Matrix4.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class ViewFrustum
			{
			private:
				bbe::Vector4 planes[5];

			public:
				void updatePlanes(const bbe::Matrix4 &mvpMatrix);
				bbe::Vector4* getPlanes();
			};
		}
	}
}