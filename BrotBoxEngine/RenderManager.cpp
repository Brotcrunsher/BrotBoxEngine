#include "BBE/RenderManager.h"

void bbe::RenderManager::setFillMode2D(bbe::FillMode fm)
{
	m_fillMode2D = fm;
}

bbe::FillMode bbe::RenderManager::getFillMode2D()
{
	return m_fillMode2D;
}

void bbe::RenderManager::setFillMode3D(bbe::FillMode fm)
{
	m_fillMode3D = fm;
}

bbe::FillMode bbe::RenderManager::getFillMode3D()
{
	return m_fillMode3D;
}
