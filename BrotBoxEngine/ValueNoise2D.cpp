#include "stdafx.h"
#include "BBE/ValueNoise2D.h"
#include "BBE/Exceptions.h"
#include "BBE/DynamicArray.h"
#include "BBE/Random.h"
#include "BBE/Math.h"
#include "BBE\ValueNoise2D.h"

static int m_octaves = 3;
static int m_startFrequencyX = 2;
static int m_startFrequencyY = 2;
static float m_startAlpha = 1;
static float m_alphaChange = 0.5f;
static int m_frequencyChange = 2;

void bbe::ValueNoise2D::standardize()
{
	float min = 100000000.0f;
	float max = -100000000.0f;
	for (int i = 0; i < m_width * m_height; i++)
	{
		float val = m_pdata[i];
		if (val < min) min = val;
		if (val > max) max = val;
	}

	for (int i = 0; i < m_width * m_height; i++)
	{
		m_pdata[i] = (m_pdata[i] - min) / (max - min);
	}
}

bbe::ValueNoise2D::ValueNoise2D()
{
}

bbe::ValueNoise2D::~ValueNoise2D()
{
	if (m_wasCreated)
	{
		destroy();
	}
}

void bbe::ValueNoise2D::create(int width, int height)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}

	m_pdata = new float[width * height];
	memset(m_pdata, 0, sizeof(float) * width * height);
	m_width = width;
	m_height = height;
	m_wasCreated = true;


	float alpha = m_startAlpha;
	int frequencyX = m_startFrequencyX;
	int frequencyY = m_startFrequencyY;
	Random rand;

	for (int octave = 0; octave < m_octaves; octave++)
	{
		DynamicArray<float> nodes((frequencyX + 2) * (frequencyY + 3));
		for (size_t i = 0; i < nodes.getLength(); i++)
		{
			nodes[i] = rand.randomFloat() * alpha;
		}

		for (int i = 0; i < m_width; i++)
		{
			for (int k = 0; k < m_height; k++)
			{
				float percentX = (float)i / (float)m_width;
				float percentY = (float)k / (float)m_height;

				float currentX = percentX * frequencyX;
				float currentY = percentY * frequencyY;

				int indexX = (int)currentX;
				int indexY = (int)currentY;

				/*float preA  = Math::interpolateCubic(nodes[indexX + 0 + (indexY + -1) * (frequencyX + 2)], nodes[indexX + 1 + (indexY + -1) * (frequencyX + 2)], nodes[indexX + 2 + (indexY + -1) * (frequencyX + 2)], nodes[indexX + 3 + (indexY + -1) * (frequencyX + 2)], currentX - indexX);
				float a     = Math::interpolateCubic(nodes[indexX + 0 + (indexY +  0) * (frequencyX + 2)], nodes[indexX + 1 + (indexY +  0) * (frequencyX + 2)], nodes[indexX + 2 + (indexY +  0) * (frequencyX + 2)], nodes[indexX + 3 + (indexY +  0) * (frequencyX + 2)], currentX - indexX);
				float b     = Math::interpolateCubic(nodes[indexX + 0 + (indexY +  1) * (frequencyX + 2)], nodes[indexX + 1 + (indexY +  1) * (frequencyX + 2)], nodes[indexX + 2 + (indexY +  1) * (frequencyX + 2)], nodes[indexX + 3 + (indexY +  1) * (frequencyX + 2)], currentX - indexX);
				float postB = Math::interpolateCubic(nodes[indexX + 0 + (indexY +  2) * (frequencyX + 2)], nodes[indexX + 1 + (indexY +  2) * (frequencyX + 2)], nodes[indexX + 2 + (indexY +  2) * (frequencyX + 2)], nodes[indexX + 3 + (indexY +  2) * (frequencyX + 2)], currentX - indexX);
				float w  = Math::interpolateCubic(preA, a, b, postB, currentY - indexY);*/

				float preA  = Math::interpolateCubic(nodes[indexX + 0 + (indexY + 0) * (frequencyX + 1)], nodes[indexX + 1 + (indexY + 0) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 0) * (frequencyX + 1)], nodes[indexX + 3 + (indexY + 0) * (frequencyX + 1)], currentX - indexX);
				float a     = Math::interpolateCubic(nodes[indexX + 0 + (indexY + 1) * (frequencyX + 1)], nodes[indexX + 1 + (indexY + 1) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 1) * (frequencyX + 1)], nodes[indexX + 3 + (indexY + 1) * (frequencyX + 1)], currentX - indexX);
				float b     = Math::interpolateCubic(nodes[indexX + 0 + (indexY + 2) * (frequencyX + 1)], nodes[indexX + 1 + (indexY + 2) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 2) * (frequencyX + 1)], nodes[indexX + 3 + (indexY + 2) * (frequencyX + 1)], currentX - indexX);
				float postB = Math::interpolateCubic(nodes[indexX + 0 + (indexY + 3) * (frequencyX + 1)], nodes[indexX + 1 + (indexY + 3) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 3) * (frequencyX + 1)], nodes[indexX + 3 + (indexY + 3) * (frequencyX + 1)], currentX - indexX);
				/*float a = Math::interpolateLinear(nodes[indexX + 1 + (indexY + 1) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 1) * (frequencyX + 1)], currentX - indexX);
				float b = Math::interpolateLinear(nodes[indexX + 1 + (indexY + 2) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 2) * (frequencyX + 1)], currentX - indexX);
				float b = Math::interpolateLinear(nodes[indexX + 1 + (indexY + 2) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 2) * (frequencyX + 1)], currentX - indexX);*/
				float w = Math::interpolateCubic(preA, a, b, postB, currentY - indexY);

				/*float a = Math::interpolateLinear(nodes[indexX + 1 + (indexY + 1) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 1) * (frequencyX + 1)], currentX - indexX);
				float b = Math::interpolateLinear(nodes[indexX + 1 + (indexY + 2) * (frequencyX + 1)], nodes[indexX + 2 + (indexY + 2) * (frequencyX + 1)], currentX - indexX);
				float w = Math::interpolateLinear(a, b, currentY - indexY);*/

				int index = i + k * m_width;
				m_pdata[index] += w;
			}
		}


		alpha *= m_alphaChange;
		frequencyX *= m_frequencyChange;
		frequencyY *= m_frequencyChange;
	}

	standardize();
}

void bbe::ValueNoise2D::destroy()
{
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}

	delete[] m_pdata;

	m_pdata = nullptr;
	m_width = 0;
	m_height = 0;


	m_wasCreated = false;
}

float bbe::ValueNoise2D::get(int x, int y)
{
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= m_width) x = m_width - 1;
	if (y >= m_height) y = m_height - 1;
	return m_pdata[x + y * m_width];
}

void bbe::ValueNoise2D::set(int x, int y, float val)
{
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	m_pdata[x + y * m_width] = val;
}

float * bbe::ValueNoise2D::getRaw()
{
	return m_pdata;
}
