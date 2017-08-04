#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"
#include "String.h"
#include "List.h"


namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;

			class VulkanShader
			{
			private:
				VkShaderModule m_shaderModule = VK_NULL_HANDLE;
				VkDevice m_device;

			public:
				VulkanShader();


				void init(const VulkanDevice &device, const bbe::String &path);

				void init(const VulkanDevice &device, const bbe::List<char> &code);

				void destroy();

				VkShaderModule getModule();
			};
		}
	}
}