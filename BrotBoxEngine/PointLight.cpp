#include "stdafx.h"
#include "BBE/PointLight.h"
#include "BBE/Exceptions.h"
#include "BBE/EngineSettings.h"

bbe::INTERNAL::vulkan::VulkanBuffer bbe::PointLight::s_bufferVertexData;
bbe::INTERNAL::PointLightVertexData *bbe::PointLight::s_dataVertex;
bbe::INTERNAL::vulkan::VulkanBuffer bbe::PointLight::s_bufferFragmentData;
bbe::INTERNAL::PointLightFragmentData *bbe::PointLight::s_dataFragment;
bbe::Stack<int> bbe::PointLight::s_indexStack;
bool bbe::PointLight::s_staticIniCalled = false;
bbe::List<bbe::INTERNAL::PointLightWithPos> bbe::PointLight::s_earlyPointLights;

bbe::Vector3 bbe::PointLight::getPosition()
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	return s_dataVertex[m_index].m_position;
}

void bbe::PointLight::setPosition(Vector3 pos)
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	s_dataVertex[m_index].m_position = pos;
}

void bbe::PointLight::destroy()
{
	if (s_dataVertex[m_index].m_used == VK_TRUE)
	{
		s_dataVertex[m_index].m_used = VK_FALSE;
		s_indexStack.push(m_index);
	}
}

void bbe::PointLight::turnOn(bool on)
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}

	s_dataVertex[m_index].m_used = on ? 1.0f : -1.0f;

}

bool bbe::PointLight::isOn()
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	return s_dataVertex[m_index].m_used > 0.0f;
}

void bbe::PointLight::s_init(VkDevice device, VkPhysicalDevice physicalDevice)
{
	s_bufferVertexData.create(device, physicalDevice, sizeof(INTERNAL::PointLightVertexData) * Settings::getAmountOfLightSources(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	s_dataVertex = (INTERNAL::PointLightVertexData*)s_bufferVertexData.map();

	s_bufferFragmentData.create(device, physicalDevice, sizeof(INTERNAL::PointLightFragmentData) * Settings::getAmountOfLightSources(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	s_dataFragment = (INTERNAL::PointLightFragmentData*)s_bufferFragmentData.map();

	memset(s_dataVertex, 0, sizeof(INTERNAL::PointLightVertexData) * Settings::getAmountOfLightSources());

	for (int i = 0; i < Settings::getAmountOfLightSources(); i++)
	{
		s_dataFragment[i].m_lightStrength = 10.0f;
		s_dataFragment[i].m_lightColor = Color(1, 1, 1, 1);
		s_dataFragment[i].m_specularColor = Color(.35f, .35f, .35f, 1);
		s_dataFragment[i].m_lightFallOffMode = LightFalloffMode::LIGHT_FALLOFF_LINEAR;
	}

	for (int i = Settings::getAmountOfLightSources() - 1; i>=0; i--)
	{
		s_indexStack.push(i);
	}

	s_staticIniCalled = true;
	
	for (size_t i = 0; i < s_earlyPointLights.getLength(); i++)
	{
		s_earlyPointLights[i].m_plight->init(s_earlyPointLights[i].m_pos);
	}

	s_earlyPointLights.clear();

}

void bbe::PointLight::s_destroy()
{
	s_bufferVertexData.unmap();
	s_bufferFragmentData.unmap();

	s_bufferVertexData.destroy();
	s_bufferFragmentData.destroy();
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
	s_dataVertex[m_index].m_used = VK_TRUE;
	s_dataVertex[m_index].m_position = pos;
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
	m_plight = pointLight;
	m_pos = pos;
}

void bbe::PointLight::setLightStrength(float lightStrength)
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	s_dataFragment[m_index].m_lightStrength = lightStrength;
}

float bbe::PointLight::getLightStrength()
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	return s_dataFragment[m_index].m_lightStrength;
}

void bbe::PointLight::setLightColor(const Color & color)
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	s_dataFragment[m_index].m_lightColor = color;
}

bbe::Color bbe::PointLight::getLightColor()
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	return s_dataFragment[m_index].m_lightColor;
}

void bbe::PointLight::setSpecularColor(const Color & color)
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	s_dataFragment[m_index].m_specularColor = color;
}

bbe::Color bbe::PointLight::getSpecularColor()
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	return s_dataFragment[m_index].m_specularColor;
}

void bbe::PointLight::setFalloffMode(LightFalloffMode falloffMode)
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	s_dataFragment[m_index].m_lightFallOffMode = falloffMode;
}

bbe::LightFalloffMode bbe::PointLight::getFalloffMode()
{
	if (!s_staticIniCalled)
	{
		throw IllegalStateException("Engine must have started!");
	}
	return s_dataFragment[m_index].m_lightFallOffMode;
}
