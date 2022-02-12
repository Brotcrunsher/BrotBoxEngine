#pragma once

#include "../BBE/Vulkan/VulkanBuffer.h"
#include "../BBE/Vector3.h"
#include "../BBE/DynamicArray.h"
#include "../BBE/Stack.h"
#include "../BBE/Color.h"
#include "../BBE/LightFalloffMode.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class PointLight;

	namespace INTERNAL
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

		class PointLightWithPos
		{
		public:
			PointLightWithPos(PointLight* pointLight, const Vector3 &pos);
			PointLight* m_plight;
			Vector3 m_pos;

			bool operator==(const PointLightWithPos &other) const
			{
				return m_plight == other.m_plight;
			}
		};
	}

	class PointLight
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		int m_index;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice);
		static bool s_staticIniCalled;
		static void s_destroy();
		static bbe::INTERNAL::vulkan::VulkanBuffer s_bufferVertexData;
		static INTERNAL::PointLightVertexData *s_dataVertex;
		static bbe::INTERNAL::vulkan::VulkanBuffer s_bufferFragmentData;
		static INTERNAL::PointLightFragmentData *s_dataFragment;
		static Stack<int> s_indexStack;
		static List<INTERNAL::PointLightWithPos> s_earlyPointLights;

		void init(const Vector3 &pos);


	public:


		PointLight();
		explicit PointLight(const Vector3 &pos);
		~PointLight();

		PointLight(const PointLight&) = delete;
		PointLight(PointLight&&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		PointLight& operator=(PointLight&&) = delete;

		Vector3 getPosition();
		void setPosition(const Vector3 &pos);

		void destroy();
		void turnOn(bool on);
		bool isOn();

		void setLightStrength(float lightStrength);
		float getLightStrength();
		void setLightColor(const Color &color);
		void setLightColor(float r, float g, float b, float a = 1);
		Color getLightColor();
		void setSpecularColor(const Color &color);
		Color getSpecularColor();
		void setFalloffMode(LightFalloffMode falloffMode);
		LightFalloffMode getFalloffMode();
	};
}