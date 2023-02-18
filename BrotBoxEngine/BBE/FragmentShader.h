#pragma once
#include "../BBE/AutoRefCountable.h"
#include "../BBE/List.h"
#include "../BBE/Vector2.h"

#ifdef BBE_RENDERER_OPENGL
#include "../BBE/glfwWrapper.h"
#endif

namespace bbe
{
	class PrimitiveBrush2D;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
			class VulkanFragmentShader;
		}
		namespace openGl
		{
			class OpenGLManager;
			struct OpenGLFragmentShader;
		}
	}


	class FragmentShader
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class INTERNAL::vulkan::VulkanFragmentShader;
		friend class INTERNAL::openGl::OpenGLManager;
		friend struct INTERNAL::openGl::OpenGLFragmentShader;
	private:
#ifdef BBE_RENDERER_VULKAN
		constexpr static size_t PUSHCONST_START_ADDR = 80;
		bbe::Array<char, 48> pushConstants;
#endif

		bool isLoaded = false;

		mutable bbe::AutoRef m_prendererData = nullptr;
		bbe::List<unsigned char> m_rawData;
	public:
		FragmentShader();
		explicit FragmentShader(const char* path);
		explicit FragmentShader(const bbe::List<unsigned char>& rawData);

		void load(const char* path);
		void load(const bbe::List<unsigned char>& rawData);

#ifdef BBE_RENDERER_VULKAN
		void setPushConstant(uint32_t offset, uint32_t length, const void* data);
#elif defined(BBE_RENDERER_OPENGL)
		void setUniform2fv(const char* name, GLsizei size, const bbe::Vector2* values);
		void setUniform1d(const char* name, double value);
		void setUniform1i(const char* name, GLint value);
		void setUniform1f(const char* name, float value);
#endif
	};
}
