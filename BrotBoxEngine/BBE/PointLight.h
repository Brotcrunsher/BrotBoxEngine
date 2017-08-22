#pragma once

#include "../BBE/VulkanBuffer.h"
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
			Vector3 position;
			float used = -1;
		};

		class PointLightFragmentData
		{
		public:
			float lightStrength;
			LightFalloffMode lightFallOffMode;
			float pad1;
			float pad2;
			Color lightColor;
			Color specularColor;
		};

		class PointLightWithPos
		{
		public:
			PointLightWithPos(PointLight* pointLight, const Vector3 &pos);
			PointLight* m_light;
			Vector3 m_pos;

			bool operator==(const PointLightWithPos &other) const
			{
				return m_light == other.m_light;
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
		PointLight(const Vector3 &pos);
		~PointLight();

		PointLight(const PointLight&) = delete;
		PointLight(PointLight&&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		PointLight& operator=(PointLight&&) = delete;

		Vector3 getPosition();
		void setPosition(Vector3 pos);

		void destroy();
		void turnOn(bool on);
		bool isOn();

		void setLightStrength(float lightStrength);
		float getLightStrength();
		void setLightColor(const Color &color);
		Color getLightColor();
		void setSpecularColor(const Color &color);
		Color getSpecularColor();
		void setFalloffMode(LightFalloffMode falloffMode);
		LightFalloffMode getFalloffMode();
	};
}