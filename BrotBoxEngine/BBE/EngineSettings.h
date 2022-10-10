#pragma once


namespace bbe
{
	namespace Settings
	{
		void INTERNAL_start();

		int getAmountOfLightSources();
		void setAmountOfLightSources(int amount);

		int getTerrainAdditionalTextures();

		void setShaderDoublesAllowed(bool allowed);
		bool getShaderDoublesAllowed();
	}

}