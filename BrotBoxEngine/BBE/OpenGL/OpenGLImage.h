#pragma once

#include "../BBE/ManuallyRefCountable.h"
#include "../BBE/Image.h"
#include "BBE/glfwWrapper.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct OpenGLImage : public ManuallyRefCountable
			{
				GLuint tex = 0;

				OpenGLImage(const bbe::Image& image);
				~OpenGLImage();

				OpenGLImage(const OpenGLImage&) = delete;
				OpenGLImage(OpenGLImage&&) = delete;
				OpenGLImage& operator =(const OpenGLImage&) = delete;
				OpenGLImage&& operator ==(const OpenGLImage&&) = delete;

				GLint internalFormat(const bbe::Image& image);
				GLenum format(const bbe::Image& image);
				GLenum type(const bbe::Image& image);
			};
		}
	}
}
