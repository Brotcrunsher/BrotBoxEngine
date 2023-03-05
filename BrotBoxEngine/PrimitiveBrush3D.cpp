#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Math.h"
#include "BBE/Vector2.h"
#include "BBE/Matrix4.h"
#include "BBE/Rectangle.h"
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif
#ifdef BBE_RENDERER_OPENGL
#include "BBE/OpenGL/OpenGLManager.h"
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
	m_rectangle = bbe::Model(
		{ 
			bbe::PosNormalPair{bbe::Vector3(-0.5, -0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(0, 0)},
			bbe::PosNormalPair{bbe::Vector3(-0.5,  0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(0, 1)},
			bbe::PosNormalPair{bbe::Vector3( 0.5, -0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(1, 0)},
			bbe::PosNormalPair{bbe::Vector3( 0.5,  0.5, 0), bbe::Vector3(0, 0, 1), bbe::Vector2(1, 1)},
		},
		{0, 1, 2, 2, 1, 3}
	);

	m_cube = bbe::Model(
		{
			PosNormalPair{Vector3( 0.5, -0.5, -0.5), Vector3(0, 0, -1), bbe::Vector2(1, 0)},
			PosNormalPair{Vector3( 0.5,  0.5, -0.5), Vector3(0, 0, -1), bbe::Vector2(1, 1)},
			PosNormalPair{Vector3(-0.5,  0.5, -0.5), Vector3(0, 0, -1), bbe::Vector2(0, 1)},
			PosNormalPair{Vector3(-0.5, -0.5, -0.5), Vector3(0, 0, -1), bbe::Vector2(0, 0)},

			PosNormalPair{Vector3( 0.5, -0.5,  0.5), Vector3(0, 0,  1), bbe::Vector2(1, 0)},
			PosNormalPair{Vector3( 0.5,  0.5,  0.5), Vector3(0, 0,  1), bbe::Vector2(1, 1)},
			PosNormalPair{Vector3(-0.5,  0.5,  0.5), Vector3(0, 0,  1), bbe::Vector2(0, 1)},
			PosNormalPair{Vector3(-0.5, -0.5,  0.5), Vector3(0, 0,  1), bbe::Vector2(0, 0)},

			PosNormalPair{Vector3( 0.5, -0.5, -0.5), Vector3(0, -1, 0), bbe::Vector2(1, 0)},
			PosNormalPair{Vector3( 0.5, -0.5,  0.5), Vector3(0, -1, 0), bbe::Vector2(1, 1)},
			PosNormalPair{Vector3(-0.5, -0.5,  0.5), Vector3(0, -1, 0), bbe::Vector2(0, 1)},
			PosNormalPair{Vector3(-0.5, -0.5, -0.5), Vector3(0, -1, 0), bbe::Vector2(0, 0)},

			PosNormalPair{Vector3( 0.5,  0.5, -0.5), Vector3(0,  1, 0), bbe::Vector2(1, 0)},
			PosNormalPair{Vector3( 0.5,  0.5,  0.5), Vector3(0,  1, 0), bbe::Vector2(1, 1)},
			PosNormalPair{Vector3(-0.5,  0.5,  0.5), Vector3(0,  1, 0), bbe::Vector2(0, 1)},
			PosNormalPair{Vector3(-0.5,  0.5, -0.5), Vector3(0,  1, 0), bbe::Vector2(0, 0)},

			PosNormalPair{Vector3(-0.5,  0.5, -0.5), Vector3(-1, 0, 0), bbe::Vector2(1, 0)},
			PosNormalPair{Vector3(-0.5,  0.5,  0.5), Vector3(-1, 0, 0), bbe::Vector2(1, 1)},
			PosNormalPair{Vector3(-0.5, -0.5,  0.5), Vector3(-1, 0, 0), bbe::Vector2(0, 1)},
			PosNormalPair{Vector3(-0.5, -0.5, -0.5), Vector3(-1, 0, 0), bbe::Vector2(0, 0)},

			PosNormalPair{Vector3( 0.5,  0.5, -0.5), Vector3( 1, 0, 0), bbe::Vector2(1, 0)},
			PosNormalPair{Vector3( 0.5,  0.5,  0.5), Vector3( 1, 0, 0), bbe::Vector2(1, 1)},
			PosNormalPair{Vector3( 0.5, -0.5,  0.5), Vector3( 1, 0, 0), bbe::Vector2(0, 1)},
			PosNormalPair{Vector3( 0.5, -0.5, -0.5), Vector3( 1, 0, 0), bbe::Vector2(0, 0)},
		},
		{
		     0,  1,  3,	//Bottom
			 1,  2,  3,
			 5,  4,  7,	//Top
			 6,  5,  7,
			 9,  8, 11,	//Left
			10,  9, 11,
			12, 13, 15,	//Right
			13, 14, 15,
			16, 17, 19,	//Front
			17, 18, 19,
			21, 20, 23,	//Back
			22, 21, 23,
		}
	);
}

void bbe::PrimitiveBrush3D::fillCube(const Cube & cube)
{
	m_prenderManager->fillCube3D(cube);
}

void bbe::PrimitiveBrush3D::fillIcoSphere(const IcoSphere & sphere)
{
	m_prenderManager->fillSphere3D(sphere);
}

void bbe::PrimitiveBrush3D::fillLine(const bbe::Vector3& a, const bbe::Vector3& b, float lineWidth)
{
	bbe::Vector3 aToB = b - a;
	const bbe::Vector3 mid = (a + b) / 2;
	const bbe::Vector3 scale = bbe::Vector3(lineWidth, lineWidth, aToB.getLength());
	const bbe::Matrix4 rot = bbe::Matrix4::createRotationMatrix(bbe::Vector3(0, 0, 1), aToB);

	bbe::Cube cube(mid, scale, rot);
	fillCube(cube);
}

void bbe::PrimitiveBrush3D::addLight(const bbe::Vector3& pos, float lightStrength, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode)
{
	m_prenderManager->addLight(pos, lightStrength, lightColor, specularColor, falloffMode);
}

void bbe::PrimitiveBrush3D::addLight(const bbe::PointLight& light)
{
	addLight(light.pos, light.lightStrength, light.lightColor, light.specularColor, light.falloffMode);
}

#ifdef BBE_RENDERER_OPENGL
void bbe::PrimitiveBrush3D::fillRectangle(const bbe::Matrix4& transform, const Image* albedo, const Image* normals, const bbe::FragmentShader* shader)
{
	fillModel(transform, m_rectangle, albedo, normals, shader);
}

void bbe::PrimitiveBrush3D::fillCube(const Cube& cube, const Image* albedo, const Image* normals, const bbe::FragmentShader* shader)
{
	fillModel(cube.getTransform(), m_cube, albedo, normals, shader);
}

void bbe::PrimitiveBrush3D::fillModel(const bbe::Matrix4& transform, const bbe::Model& model, const Image* albedo, const Image* normals, const bbe::FragmentShader* shader)
{
	((bbe::INTERNAL::openGl::OpenGLManager*)m_prenderManager)->fillModel(transform, model, albedo, normals, shader);
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

void bbe::PrimitiveBrush3D::setColorHSV(float h, float s, float v)
{
	auto rgb = bbe::Color::HSVtoRGB(h, s, v);
	setColor(rgb.r, rgb.g, rgb.b, 1.0);
}

void bbe::PrimitiveBrush3D::setCamera(const Vector3 & cameraPos, const Vector3 & cameraTarget, const Vector3 & cameraUpVector)
{
	m_view = Matrix4::createViewMatrix(cameraPos, cameraTarget, cameraUpVector);
	m_projection = Matrix4::createPerspectiveMatrix(Math::toRadians(60.0f), (float)m_screenWidth / (float)m_screenHeight, 0.01f, 20000.0f);

	m_prenderManager->setCamera3D(cameraPos, m_view, m_projection);

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
