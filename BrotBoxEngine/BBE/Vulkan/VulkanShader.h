#pragma once

#include "GLFW/glfw3.h"
#include "../BBE/String.h"
#include "../BBE/ByteBuffer.h"


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
				VkDevice       m_device       = VK_NULL_HANDLE;

			public:
				VulkanShader();

				void init(const bbe::String& path);
				void init(const VulkanDevice &device, const bbe::String &path);
				void init(const VulkanDevice &device, const bbe::ByteBuffer &code);

				void destroy();

				VkShaderModule getModule();
			};
		}
	}
}
