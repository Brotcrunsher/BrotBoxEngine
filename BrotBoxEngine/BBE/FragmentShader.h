#pragma once

#include "../BBE/VulkanShader.h"
#include "../BBE/VulkanPipeline.h"

namespace bbe
{
	class PrimitiveBrush2D;

	class FragmentShader
	{
	private:
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

		void setPushConstant(PrimitiveBrush2D& brush, size_t offset, size_t length, const void* data);
	};
}