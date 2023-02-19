#include "BBE/FragmentShader.h"
#include "BBE/SimpleFile.h"
#include "BBE/Window.h"
#include "BBE/PrimitiveBrush2D.h"
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif
#ifdef BBE_RENDERER_OPENGL
#include "BBE/OpenGL/OpenGLFragmentShader.h"
#endif

bbe::FragmentShader::FragmentShader()
{
}

bbe::FragmentShader::FragmentShader(const char* path)
{
	load(path);
}

bbe::FragmentShader::FragmentShader(const bbe::List<unsigned char>& rawData)
{
	load(rawData);
}

void bbe::FragmentShader::load(const char* path)
{
	load(bbe::simpleFile::readBinaryFile(path));
}

void bbe::FragmentShader::load(const bbe::List<unsigned char>& rawData)
{
	if (bbe::Window::INTERNAL_firstInstance == nullptr)
	{
		throw NullPointerException();
	}
#ifdef BBE_RENDERER_VULKAN
	if (bbe::INTERNAL::vulkan::VulkanManager::s_pinstance == nullptr)
	{
		throw NullPointerException();
	}
#endif
	if (isLoaded)
	{
		throw AlreadyCreatedException();
	}

	m_rawData = rawData;
#ifdef BBE_RENDERER_VULKAN
	memset(pushConstants.getRaw(), 0, pushConstants.getLength());
#endif

	isLoaded = true;
}

#ifdef BBE_RENDERER_VULKAN
void bbe::FragmentShader::setPushConstant(uint32_t offset, uint32_t length, const void* data)
{
	if (offset < PUSHCONST_START_ADDR || offset + length > PUSHCONST_START_ADDR + pushConstants.getLength())
	{
		//Only in the range of [80..128) the push constants are guaranteed to be present.
		throw IllegalArgumentException();
	}
	memcpy(pushConstants.getRaw() + offset - PUSHCONST_START_ADDR, data, length);
}
#endif

#ifdef BBE_RENDERER_OPENGL
void bbe::FragmentShader::setUniform2fv(const char* name, GLsizei size, const bbe::Vector2* values)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)m_prendererData.get();
	glUseProgram(s->program2d);
	glUniform2fv(glGetUniformLocation(s->program2d, name), size * 2, (float*)values);
	glUseProgram(s->program3d);
	glUniform2fv(glGetUniformLocation(s->program3d, name), size * 2, (float*)values);
}
void bbe::FragmentShader::setUniform1d(const char* name, double value)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader *)m_prendererData.get();
	glUseProgram(s->program2d);
	glUniform1d(glGetUniformLocation(s->program2d, name), value);
	glUseProgram(s->program3d);
	glUniform1d(glGetUniformLocation(s->program3d, name), value);
}

void bbe::FragmentShader::setUniform1i(const char* name, GLint value)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)m_prendererData.get();
	glUseProgram(s->program2d);
	glUniform1i(glGetUniformLocation(s->program2d, name), value);
	glUseProgram(s->program3d);
	glUniform1i(glGetUniformLocation(s->program3d, name), value);
}

void bbe::FragmentShader::setUniform1f(const char* name, float value)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)m_prendererData.get();
	glUseProgram(s->program2d);
	glUniform1f(glGetUniformLocation(s->program2d, name), value);
	glUseProgram(s->program3d);
	glUniform1f(glGetUniformLocation(s->program3d, name), value);
}
#endif
