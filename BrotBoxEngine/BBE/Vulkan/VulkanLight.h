#pragma once

#include "../BBE/Vulkan/VulkanBuffer.h"
#include "../BBE/Vector3.h"
#include "../BBE/DynamicArray.h"
#include "../BBE/Stack.h"
#include "../BBE/Color.h"
#include "../BBE/LightFalloffMode.h"

namespace bbe
{
	class PointLight;
	namespace INTERNAL
	{
		namespace vulkan
		{
			namespace VulkanLight
			{
				class PointLightVertexData
				{
				public:
					Vector3 m_position;
					float   m_used = -1;
				};

				class PointLightFragmentData
				{
				public:
					float            m_lightStrength;
					LightFalloffMode m_lightFallOffMode;
					float            pad1;
					float            pad2;
					Color            m_lightColor;
					Color            m_specularColor;
				};

				void s_init(VkDevice device, VkPhysicalDevice physicalDevice);
				void beginDraw();
				void addLight(const bbe::Vector3& pos, float lightStrenght, const bbe::Color &lightColor, const bbe::Color &specularColor, LightFalloffMode falloffMode);
				void s_destroy();

				VulkanBuffer& getVertexBuffer();
				VulkanBuffer& getFragmentBuffer();
			}
		}
	}
}