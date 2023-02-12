#pragma once

#include "../BBE/FragmentShader.h"
#include "../BBE/ManuallyRefCountable.h"
#include "../BBE/Vulkan/VulkanShader.h"
#include "../BBE/Vulkan/VulkanPipeline.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanCommandPool;
			class VulkanManager;
			class VulkanDescriptorSet;

			struct VulkanFragmentShader : public ManuallyRefCountable
			{
				INTERNAL::vulkan::VulkanShader   m_shader;
				INTERNAL::vulkan::VulkanPipeline m_pipeline;

				VulkanFragmentShader(const bbe::FragmentShader& shader);
				~VulkanFragmentShader();

				VulkanFragmentShader(const VulkanFragmentShader&) = delete;
				VulkanFragmentShader(VulkanFragmentShader&&) = delete;
				VulkanFragmentShader& operator =(const VulkanFragmentShader&) = delete;
				VulkanFragmentShader&& operator ==(const VulkanFragmentShader&&) = delete;

				void init(const bbe::List<unsigned char>& code);
			};
		}
	}
}
