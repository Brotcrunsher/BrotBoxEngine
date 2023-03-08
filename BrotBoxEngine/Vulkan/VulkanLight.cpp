#include "BBE/Vulkan/VulkanLight.h"
#include "BBE/EngineSettings.h"
#include "BBE/Exceptions.h"

static size_t lightAmount = 0;
static bbe::INTERNAL::vulkan::VulkanBuffer s_bufferVertexData;
static bbe::INTERNAL::vulkan::VulkanLight::PointLightVertexData* s_dataVertex;
static bbe::INTERNAL::vulkan::VulkanBuffer s_bufferFragmentData;
static bbe::INTERNAL::vulkan::VulkanLight::PointLightFragmentData* s_dataFragment;

void bbe::INTERNAL::vulkan::VulkanLight::s_init(VkDevice device, VkPhysicalDevice physicalDevice)
{
	s_bufferVertexData.create(device, physicalDevice, sizeof(PointLightVertexData) * Settings::getAmountOfLightSources(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	s_dataVertex = (PointLightVertexData*)s_bufferVertexData.map();

	s_bufferFragmentData.create(device, physicalDevice, sizeof(PointLightFragmentData) * Settings::getAmountOfLightSources(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	s_dataFragment = (PointLightFragmentData*)s_bufferFragmentData.map();

	memset(s_dataVertex, 0, sizeof(PointLightVertexData) * Settings::getAmountOfLightSources());

	for (int i = 0; i < Settings::getAmountOfLightSources(); i++)
	{
		s_dataFragment[i].m_lightStrength = 10.0f;
		s_dataFragment[i].m_lightColor = Color(1, 1, 1, 1);
		s_dataFragment[i].m_specularColor = Color(.35f, .35f, .35f, 1.0f);
		s_dataFragment[i].m_lightFallOffMode = LightFalloffMode::LIGHT_FALLOFF_LINEAR;
	}
}

void bbe::INTERNAL::vulkan::VulkanLight::beginDraw()
{
	lightAmount = 0;
	for (size_t i = 0; i < Settings::getAmountOfLightSources(); i++)
	{
		s_dataVertex[0].m_used = -1.0f;
	}
}

void bbe::INTERNAL::vulkan::VulkanLight::addLight(const bbe::Vector3& pos, float lightStrenght, const bbe::Color &lightColor, const bbe::Color &specularColor, LightFalloffMode falloffMode)
{
	lightAmount++;
	if (lightAmount >= Settings::getAmountOfLightSources())
	{
		throw bbe::OutOfLightResourcesException();
	}

	s_dataVertex[lightAmount].m_position = pos;
	s_dataVertex[lightAmount].m_used = 1.0f;


	s_dataFragment[lightAmount].m_lightStrength = lightStrenght;
	s_dataFragment[lightAmount].m_lightFallOffMode = falloffMode;
	s_dataFragment[lightAmount].pad1;
	s_dataFragment[lightAmount].pad2;
	s_dataFragment[lightAmount].m_lightColor = lightColor;
	s_dataFragment[lightAmount].m_specularColor = specularColor;
}

void bbe::INTERNAL::vulkan::VulkanLight::s_destroy()
{
	s_bufferVertexData.unmap();
	s_bufferFragmentData.unmap();

	s_bufferVertexData.destroy();
	s_bufferFragmentData.destroy();
}

bbe::INTERNAL::vulkan::VulkanBuffer& bbe::INTERNAL::vulkan::VulkanLight::getVertexBuffer()
{
	return s_bufferVertexData;
}

bbe::INTERNAL::vulkan::VulkanBuffer& bbe::INTERNAL::vulkan::VulkanLight::getFragmentBuffer()
{
	return s_bufferFragmentData;
}
