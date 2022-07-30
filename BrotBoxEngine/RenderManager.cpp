#include "BBE/RenderManager.h"

void bbe::RenderManager::setFillMode2D(bbe::FillMode fm)
{
	m_fillMode = fm;
}

bbe::FillMode bbe::RenderManager::getFillMode2D()
{
	return m_fillMode;
}
