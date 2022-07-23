#include "bbe/NullRenderer/NullRendererManager.h"

bbe::INTERNAL::nullRenderer::NullRendererManager::NullRendererManager()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::init(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow* window, uint32_t initialWindowWidth, uint32_t initialWindowHeight)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::destroy()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::preDraw2D()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::preDraw3D()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::preDraw()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::postDraw()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::waitEndDraw()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::waitTillIdle()
{
}

bbe::PrimitiveBrush2D& bbe::INTERNAL::nullRenderer::NullRendererManager::getBrush2D()
{
	return m_primitiveBrush2D;
}

bbe::PrimitiveBrush3D& bbe::INTERNAL::nullRenderer::NullRendererManager::getBrush3D()
{
	return m_primitiveBrush3D;
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::resize(uint32_t width, uint32_t height)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::screenshot(const bbe::String& path)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::setVideoRenderingMode(const char* path)
{
}

bool bbe::INTERNAL::nullRenderer::NullRendererManager::isReadyToDraw() const
{
	return true;
}
