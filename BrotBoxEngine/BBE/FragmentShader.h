#pragma once
// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include "../BBE/Vulkan/VulkanShader.h"
#include "../BBE/Vulkan/VulkanPipeline.h"

namespace bbe
{
	class PrimitiveBrush2D;

	class FragmentShader
	{
	private:
		constexpr static size_t PUSHCONST_START_ADDR = 80;
		bbe::Array<char, 48> pushConstants;

		INTERNAL::vulkan::VulkanShader   m_shader;
		INTERNAL::vulkan::VulkanPipeline m_pipeline;

		bool isLoaded = false;

	public:
		FragmentShader();
		FragmentShader(const char* path);
		~FragmentShader();

		FragmentShader(const FragmentShader& other) = delete;
		FragmentShader(FragmentShader&& other) = delete;
		FragmentShader& operator=(const FragmentShader& other) = delete;
		FragmentShader& operator=(FragmentShader&& other) = delete;

		void load(const char* path);

		INTERNAL::vulkan::VulkanPipeline& INTERNAL_getPipeline();

		const char* getPushConstants() const;

		void setPushConstant(uint32_t offset, uint32_t length, const void* data);
	};
}
#endif