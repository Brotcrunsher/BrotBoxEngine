#pragma once

#include "../BBE/ManuallyRefCountable.h"
#include "../BBE/FragmentShader.h"
#include "../BBE/glfwWrapper.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct OpenGLFragmentShader : public ManuallyRefCountable
			{
				GLuint vertex = 0;
				GLuint fragment = 0;
				GLuint program = 0;
				GLint screenSizePos = 0;
				GLint scalePosOffsetPos = 0;
				GLint rotationPos = 0;

				OpenGLFragmentShader(const bbe::FragmentShader& shader);
				~OpenGLFragmentShader();

				OpenGLFragmentShader(const OpenGLFragmentShader&) = delete;
				OpenGLFragmentShader(OpenGLFragmentShader&&) = delete;
				OpenGLFragmentShader& operator =(const OpenGLFragmentShader&) = delete;
				OpenGLFragmentShader&& operator ==(const OpenGLFragmentShader&&) = delete;


				GLuint getShader(GLenum shaderType, const bbe::String& src);
			};
		}
	}
}
