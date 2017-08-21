#include "stdafx.h"
#include "BBE/PointLight.h"
#include "BBE/Exceptions.h"
#include "BBE/EngineSettings.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::PointLight::s_buffer;
bbe::INTERNAL::PointLightVertexData *bbe::PointLight::s_data;
bbe::Stack<int> bbe::PointLight::s_indexStack;
bool bbe::PointLight::s_staticIniCalled = false;
bbe::List<bbe::INTERNAL::PointLightWithPos> bbe::PointLight::s_earlyPointLights;

bbe::Vector3 bbe::PointLight::getPosition()
{
	return s_data[m_index].position;
}

void bbe::PointLight::setPosition(Vector3 pos)
{
	s_data[m_index].position = pos;
}

void bbe::PointLight::destroy()
{
	if (s_data[m_index].used == VK_TRUE)
	{
		s_data[m_index].used = VK_FALSE;
		s_indexStack.push(m_index);
	}
}

void bbe::PointLight::s_init(VkDevice device, VkPhysicalDevice physicalDevice)
{
	s_buffer.create(device, physicalDevice, sizeof(INTERNAL::PointLightVertexData) * Settings::getAmountOfLightSources(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	s_data = (INTERNAL::PointLightVertexData*)s_buffer.map();

	memset(s_data, 0, sizeof(INTERNAL::PointLightVertexData) * Settings::getAmountOfLightSources());

	for (int i = Settings::getAmountOfLightSources() - 1; i>=0; i--)
	{
		s_indexStack.push(i);
	}

	s_staticIniCalled = true;
	
	for (int i = 0; i < s_earlyPointLights.getLength(); i++)
	{
		s_earlyPointLights[i].m_light->init(s_earlyPointLights[i].m_pos);
	}

	s_earlyPointLights.clear();

}

void bbe::PointLight::s_destroy()
{
	s_buffer.destroy();
}

void bbe::PointLight::init(const Vector3 &pos)
{
	if (!s_staticIniCalled)
	{
		s_earlyPointLights.add(INTERNAL::PointLightWithPos(this, pos));
		return;
	}
	if (!s_indexStack.hasDataLeft())
	{
		throw OutOfLightResourcesException();
	}
	m_index = s_indexStack.pop();
	s_data[m_index].used = VK_TRUE;
	s_data[m_index].position = pos;
}

bbe::PointLight::PointLight()
{
	init(Vector3());
}

bbe::PointLight::PointLight(const Vector3 & pos)
{
	init(pos);
}

bbe::PointLight::~PointLight()
{
	if (!s_staticIniCalled)
	{
		s_earlyPointLights.removeSingle(INTERNAL::PointLightWithPos(this, Vector3()));
	}
	destroy();
}

bbe::INTERNAL::PointLightWithPos::PointLightWithPos(PointLight *pointLight, const Vector3 & pos)
{
	m_light = pointLight;
	m_pos = pos;
}
