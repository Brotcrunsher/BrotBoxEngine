#include "BBE/OpenGL/OpenGLImage.h"

bbe::INTERNAL::openGl::OpenGLImage::OpenGLImage(const bbe::Image& image)
{
	if (!image.isLoadedCpu())
	{
		bbe::Crash(bbe::Error::NotInitialized);
	}

	if (image.m_prendererData != nullptr)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	image.m_prendererData = this;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	// TODO: These wrap modes have to respect the wish of the image! When done so make sure that the light baking uses GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	if (image.getFilterMode() == bbe::ImageFilterMode::LINEAR)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (image.getFilterMode() == bbe::ImageFilterMode::NEAREST)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	
	// TODO This might break if the image decoder has a different row alignment than 1.
	//      Check if this could ever be the case with stb image.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(image), image.getWidth(), image.getHeight(), 0, format(image), type(image), image.m_pdata.getRaw());

	glGenerateMipmap(GL_TEXTURE_2D);

	if (!image.keep)
	{
		image.m_pdata = {};
	}
}

bbe::INTERNAL::openGl::OpenGLImage::OpenGLImage(const bbe::Image& image, GLuint tex)
{
	if (image.m_prendererData != nullptr)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	image.m_prendererData = this;
	this->tex = tex;
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
	case (ImageFormat::R8G8B8A8         ): return GL_RGBA8;
	case (ImageFormat::R8               ): return GL_R8;
	case (ImageFormat::R32FLOAT         ): return GL_R32F;
	case (ImageFormat::R32G32B32A32FLOAT): return GL_RGBA32F;
	}

	bbe::Crash(bbe::Error::IllegalArgument);
}

GLenum bbe::INTERNAL::openGl::OpenGLImage::format(const bbe::Image& image)
{
	switch (image.m_format)
	{
	case (ImageFormat::R8G8B8A8         ): return GL_RGBA;
	case (ImageFormat::R8               ): return GL_RED;
	case (ImageFormat::R32FLOAT         ): return GL_RED;
	case (ImageFormat::R32G32B32A32FLOAT): return GL_RGBA;
	}

	bbe::Crash(bbe::Error::IllegalArgument);
}

GLenum bbe::INTERNAL::openGl::OpenGLImage::type(const bbe::Image& image)
{
	switch (image.m_format)
	{
	case (ImageFormat::R8G8B8A8         ): return GL_UNSIGNED_BYTE;
	case (ImageFormat::R8               ): return GL_UNSIGNED_BYTE;
	case (ImageFormat::R32FLOAT         ): return GL_FLOAT;
	case (ImageFormat::R32G32B32A32FLOAT): return GL_FLOAT;
	}

	bbe::Crash(bbe::Error::IllegalArgument);
}
