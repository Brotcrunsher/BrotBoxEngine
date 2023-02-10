#include "BBE/OpenGL/OpenGLImage.h"

bbe::INTERNAL::openGl::OpenGLImage::OpenGLImage(const bbe::Image& image)
{
	if (image.m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	if (image.m_prendererData != nullptr)
	{
		throw IllegalStateException();
	}

	image.m_prendererData = this;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
	// TODO This might break if the image decoder has a different row alignment than 1.
	//      Check if this could ever be the case with stb image.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(image), image.getWidth(), image.getHeight(), 0, format(image), type(image), image.m_pdata);

	glGenerateMipmap(GL_TEXTURE_2D);
}

bbe::INTERNAL::openGl::OpenGLImage::~OpenGLImage()
{
	if (tex)
	{
		glDeleteTextures(1, &tex);
		tex = 0;
	}
}

GLint bbe::INTERNAL::openGl::OpenGLImage::internalFormat(const bbe::Image& image)
{
	switch (image.m_format)
	{
	case (ImageFormat::R8G8B8A8   ): return GL_RGBA8;
	case (ImageFormat::R8         ): return GL_R8;
	case (ImageFormat::R32FLOAT   ): return GL_R32F;
	}

	throw bbe::IllegalArgumentException();
}

GLenum bbe::INTERNAL::openGl::OpenGLImage::format(const bbe::Image& image)
{
	switch (image.m_format)
	{
	case (ImageFormat::R8G8B8A8   ): return GL_RGBA;
	case (ImageFormat::R8         ): return GL_RED;
	case (ImageFormat::R32FLOAT   ): return GL_RED;
	}

	throw bbe::IllegalArgumentException();
}

GLenum bbe::INTERNAL::openGl::OpenGLImage::type(const bbe::Image& image)
{
	switch (image.m_format)
	{
	case (ImageFormat::R8G8B8A8   ): return GL_UNSIGNED_BYTE;
	case (ImageFormat::R8         ): return GL_UNSIGNED_BYTE;
	case (ImageFormat::R32FLOAT   ): return GL_FLOAT;
	}

	throw bbe::IllegalArgumentException();
}
