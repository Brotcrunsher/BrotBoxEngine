#pragma once

#include "../BBE/AutoRefCountable.h"
#include "../BBE/Image.h"
#include "BBE/glfwWrapper.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct OpenGLImage : public AutoRefCountable
			{
				GLuint tex = 0;

				explicit OpenGLImage(const bbe::Image& image);
				OpenGLImage(const bbe::Image& image, GLuint tex);
				~OpenGLImage() override;

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
