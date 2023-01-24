#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Math.h"
#include "BBE/Vector2.h"
#include "BBE/Matrix4.h"
#include "BBE/Rectangle.h"
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif

void bbe::PrimitiveBrush3D::INTERNAL_setColor(float r, float g, float b, float a, bool force)
{
	Color c(r, g, b, a);
	if (c.r != m_color.r || c.g != m_color.g || c.b != m_color.b || c.a != m_color.a || force)
	{
		m_prenderManager->setColor3D(c);
		m_color = c;
	}
}

void bbe::PrimitiveBrush3D::INTERNAL_beginDraw(
	int width, int height,
	bbe::RenderManager* renderManager)
{
	m_screenWidth = width;
	m_screenHeight = height;
	m_prenderManager = renderManager;

	INTERNAL_setColor(1.0f, 1.0f, 1.0f, 1.0f, true);
	setCamera(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1));
}

bbe::PrimitiveBrush3D::PrimitiveBrush3D()
{
}

void bbe::PrimitiveBrush3D::fillCube(const Cube & cube)
{
	m_prenderManager->fillCube3D(cube);
}

void bbe::PrimitiveBrush3D::fillIcoSphere(const IcoSphere & sphere)
{
	m_prenderManager->fillSphere3D(sphere);
}

void bbe::PrimitiveBrush3D::addLight(const bbe::Vector3& pos, float lightStrenght, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode)
{
	m_prenderManager->addLight(pos, lightStrenght, lightColor, specularColor, falloffMode);
}

void bbe::PrimitiveBrush3D::addLight(const bbe::PointLight& light)
{
	addLight(light.pos, light.lightStrengh, light.lightColor, light.specularColor, light.falloffMode);
}

#ifdef BBE_RENDERER_VULKAN
void bbe::PrimitiveBrush3D::drawTerrain(const Terrain& terrain)
{
	((bbe::INTERNAL::vulkan::VulkanManager*)m_prenderManager)->drawTerrain(terrain, m_color);
}
#endif

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b, float a)
{
	INTERNAL_setColor(r, g, b, a, false);
}

void bbe::PrimitiveBrush3D::setColor(float r, float g, float b)
{
	INTERNAL_setColor(r, g, b, 1.0f, false);
}

void bbe::PrimitiveBrush3D::setColor(const Color & c)
{
	INTERNAL_setColor(c.r, c.g, c.b, c.a, false);
}

void bbe::PrimitiveBrush3D::setCamera(const Vector3 & cameraPos, const Vector3 & cameraTarget, const Vector3 & cameraUpVector)
{
	m_view = Matrix4::createViewMatrix(cameraPos, cameraTarget, cameraUpVector);
	m_projection = Matrix4::createPerspectiveMatrix(Math::toRadians(60.0f), (float)m_screenWidth / (float)m_screenHeight, 0.01f, 20000.0f);

	m_prenderManager->setCamera3D(m_view, m_projection);

	m_cameraPos = cameraPos;
}

void bbe::PrimitiveBrush3D::setFillMode(FillMode fm)
{
	m_fillMode = fm;
}

bbe::FillMode bbe::PrimitiveBrush3D::getFillMode()
{
	return m_fillMode;
}
