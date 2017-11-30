#pragma once

namespace bbe
{
	namespace Profiler
	{
		float getRenderTime();
		float getCPUTime();

		namespace INTERNAL
		{
			void setRenderTime(float time);
			void setCPUTime(float time);
		}
	}
}