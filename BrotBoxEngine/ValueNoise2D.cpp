#include "stdafx.h"
#include "BBE/ValueNoise2D.h"
#include "BBE/Exceptions.h"
#include "BBE/DynamicArray.h"
#include "BBE/Random.h"
#include "BBE/Math.h"
#include "BBE/ValueNoise2D.h"
#include "BBE/TimeHelper.h"

static int m_octaves = 1;
static int m_startFrequencyX = 4 * 8;
static int m_startFrequencyY = 4 * 8;
static float m_startAlpha = 1;
static float m_alphaChange = 0.5f;
static int m_frequencyChange = 2;

void bbe::ValueNoise2D::standardize()
{
	if (m_pdata == nullptr)
	{
		throw IllegalStateException();
	}
	min = 100000000.0f;
	max = -100000000.0f;
	for (int i = 0; i < m_width * m_height; i++)
	{
		float val = m_pdata[i];
		if (val < min) min = val;
		if (val > max) max = val;
	}

	float maxMin = max - min;
	for (int i = 0; i < m_width * m_height; i++)
	{
		m_pdata[i] = (m_pdata[i] - min) / maxMin;
	}

	wasStandardized = true;
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
	create(width, height, (int)TimeHelper::getTimeStamp());
}

void bbe::ValueNoise2D::create(int width, int height, int seed)
{
	if (m_wasCreated)
	{
		throw AlreadyCreatedException();
	}

	//m_pdata = new float[width * height];
	//memset(m_pdata, 0, sizeof(float) * width * height);
	m_width = width;
	m_height = height;
	m_wasCreated = true;


	float alpha = m_startAlpha;
	int frequencyX = m_startFrequencyX;
	int frequencyY = m_startFrequencyY;
	Random rand;
	rand.setSeed(seed);

	for (int octave = 0; octave < m_octaves; octave++)
	{
		DynamicArray<float> nodes((frequencyX + 2) * (frequencyY + 3));
		for (size_t i = 0; i < nodes.getLength(); i++)
		{
			nodes[i] = rand.randomFloat() * alpha;
		}

		this->nodes.add(nodes);

		alpha *= m_alphaChange;
		frequencyX *= m_frequencyChange;
		frequencyY *= m_frequencyChange;
	}

	//standardize();

}

void bbe::ValueNoise2D::destroy()
{
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}

	unload();
	m_width = 0;
	m_height = 0;


	m_wasCreated = false;
}

void bbe::ValueNoise2D::unload()
{
	if (m_pdata != nullptr)
	{
		delete[] m_pdata;
		m_pdata = nullptr;
	}
}

float bbe::ValueNoise2D::get(int x, int y) const
{
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= m_width) x = m_width - 1;
	if (y >= m_height) y = m_height - 1;

	if (m_pdata != nullptr)
	{
		return m_pdata[x * m_height + y];
	}


	float val = 0;
	float alpha = m_startAlpha;
	int frequencyX = m_startFrequencyX;
	int frequencyY = m_startFrequencyY;

	for (int octave = 0; octave < m_octaves; octave++)
	{
		int i = x;
		int k = y;
		float percentX = (float)i / (float)m_width;
		float percentY = (float)k / (float)m_height;

		float currentX = percentX * frequencyX;
		float currentY = percentY * frequencyY;

		int indexX = (int)currentX;
		int indexY = (int)currentY;


		float preA = Math::interpolateCubic(nodes[octave][indexX + 0 + (indexY + 0) * (frequencyX + 1)], nodes[octave][indexX + 1 + (indexY + 0) * (frequencyX + 1)], nodes[octave][indexX + 2 + (indexY + 0) * (frequencyX + 1)], nodes[octave][indexX + 3 + (indexY + 0) * (frequencyX + 1)], currentX - indexX);
		float a = Math::interpolateCubic(nodes[octave][indexX + 0 + (indexY + 1) * (frequencyX + 1)], nodes[octave][indexX + 1 + (indexY + 1) * (frequencyX + 1)], nodes[octave][indexX + 2 + (indexY + 1) * (frequencyX + 1)], nodes[octave][indexX + 3 + (indexY + 1) * (frequencyX + 1)], currentX - indexX);
		float b = Math::interpolateCubic(nodes[octave][indexX + 0 + (indexY + 2) * (frequencyX + 1)], nodes[octave][indexX + 1 + (indexY + 2) * (frequencyX + 1)], nodes[octave][indexX + 2 + (indexY + 2) * (frequencyX + 1)], nodes[octave][indexX + 3 + (indexY + 2) * (frequencyX + 1)], currentX - indexX);
		float postB = Math::interpolateCubic(nodes[octave][indexX + 0 + (indexY + 3) * (frequencyX + 1)], nodes[octave][indexX + 1 + (indexY + 3) * (frequencyX + 1)], nodes[octave][indexX + 2 + (indexY + 3) * (frequencyX + 1)], nodes[octave][indexX + 3 + (indexY + 3) * (frequencyX + 1)], currentX - indexX);
		float w = Math::interpolateCubic(preA, a, b, postB, currentY - indexY);

		val += w;

		alpha *= m_alphaChange;
		frequencyX *= m_frequencyChange;
		frequencyY *= m_frequencyChange;
	}

	
	if (wasStandardized)
	{
		float maxMin = max - min;
		val = (val - min) / maxMin;
	}

	return val;
}

void bbe::ValueNoise2D::preCalculate()
{
	if (m_pdata != nullptr)
	{
		throw IllegalStateException();
	}

	float *data = new float[m_width * m_height];

	for (int i = 0; i < m_width; i++)
	{
		for (int k = 0; k < m_height; k++)
		{
			data[k * m_width + i] = get(i, k);
		}
	}

	m_pdata = data;
}

void bbe::ValueNoise2D::set(int x, int y, float val)
{
	if (m_pdata == nullptr)
	{
		throw IllegalStateException();
	}
	if (!m_wasCreated)
	{
		throw NotInitializedException();
	}
	m_pdata[x + y * m_width] = val;
}

float * bbe::ValueNoise2D::getRaw()
{
	if (m_pdata == nullptr)
	{
		throw IllegalStateException();
	}
	return m_pdata;
}
