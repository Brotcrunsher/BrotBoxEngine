#pragma once


namespace bbe
{
	namespace Settings
	{
		void INTERNAL_start();

#ifdef BBE_RENDERER_VULKAN
		int getAmountOfLightSources();
		void setAmountOfLightSources(int amount);
#endif

		void setShaderDoublesAllowed(bool allowed);
		bool getShaderDoublesAllowed();
	}

}