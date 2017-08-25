#include "stdafx.h"
#include "BBE\Image.h"
#include "BBE/Exceptions.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bbe::Image::Image()
{
}

bbe::Image::Image(const char * path)
{
	load(path);
}

bbe::Image::Image(int width, int height)
{
	load(width, height);
}

bbe::Image::Image(int width, int height, const Color & c)
{
	load(width, height, c);
}

bbe::Image::~Image()
{
	destroy();
}

void bbe::Image::load(const char * path)
{
	if (m_pdata != nullptr)
	{
		throw AlreadyCreatedException();
	}

	int texChannels = 0;
	stbi_uc *pixels = stbi_load(path, &m_width, &m_height, &texChannels, STBI_rgb_alpha);

	if (pixels == nullptr)
	{
		throw LoadException();
	}

	m_pdata = new ColorByte[m_width * m_height]; //TODO use allocator
	for (int i = 0; i < m_width; i++)
	{
		for (int k = 0; k < m_height; k++)
		{
			int index = k * m_width + i;
			m_pdata[index].r = pixels[index * 4 + 0];
			m_pdata[index].g = pixels[index * 4 + 1];
			m_pdata[index].b = pixels[index * 4 + 2];
			m_pdata[index].a = pixels[index * 4 + 3];
		}
	}

	stbi_image_free(pixels);
}

void bbe::Image::load(int width, int height)
{
	load(width, height, Color());
}

void bbe::Image::load(int width, int height, const Color & c)
{
	m_width = width;
	m_height = height;

	m_pdata = new ColorByte[m_width * m_height]; //TODO use allocator
	ColorByte cb((byte)(c.r * 255), (byte)(c.g * 255), (byte)(c.b * 255), (byte)(c.a * 255));
	for (int i = 0; i < m_width * m_height; i++)
	{
		m_pdata[i] = cb;
	}
}

void bbe::Image::destroy()
{
	if (m_pdata != nullptr)
	{
		delete[] m_pdata;
		m_pdata = nullptr;
		m_width = 0;
		m_height = 0;
	}
}

int bbe::Image::getWidth() const
{
	return m_width;
}

int bbe::Image::getHeight() const
{
	return m_height;
}

bbe::Color bbe::Image::getPixel(int x, int y) const
{
	if (m_pdata == nullptr)
	{
		throw NotInitializedException();
	}

	int index = y * m_width + x;
	return Color(m_pdata[index].r / 255.f, m_pdata[index].g / 255.f, m_pdata[index].b / 255.f, m_pdata[index].a / 255.f);
}
