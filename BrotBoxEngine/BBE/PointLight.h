#pragma once

#include "../BBE/VulkanBuffer.h"
#include "../BBE/Vector3.h"
#include "../BBE/DynamicArray.h"
#include "../BBE/Stack.h"

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
		static bbe::INTERNAL::vulkan::VulkanBuffer s_buffer;
		static INTERNAL::PointLightVertexData *s_data;
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
	};
}