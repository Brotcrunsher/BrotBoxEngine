#pragma once

#include "../BBE/glfwWrapper.h"
#include "../BBE/Matrix4.h"
#include "../BBE/Cube.h"
#include "../BBE/IcoSphere.h"
#include "../BBE/Terrain.h"
#include "../BBE/FillMode.h"
#include "../BBE/ViewFrustum.h"
#include "../BBE/RenderManager.h"
#include "../BBE/Color.h"

namespace bbe
{
	class Color;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
		namespace openGl
		{
			class OpenGLManager;
		}
	}

	class PrimitiveBrush3D
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class INTERNAL::openGl::OpenGLManager;
	private:
		int                                          m_screenWidth = -1;
		int                                          m_screenHeight = -1;

		Matrix4 m_modelMatrix;
		Matrix4 m_viewProjectionMatrix;

		Vector3 m_cameraPos;

		void INTERNAL_setColor(float r, float g, float b, float a, bool force);
		void INTERNAL_beginDraw(
			int screenWidth, int screenHeight,
			bbe::RenderManager* renderManager);

		FillMode m_fillMode = FillMode::SOLID;

		Matrix4 m_view;
		Matrix4 m_projection;

		Color m_color = Color(-1000, -1000, -1000);

		bbe::RenderManager* m_prenderManager = nullptr;
	public:
		PrimitiveBrush3D();

		void fillCube(const Cube &cube);
		void fillIcoSphere(const IcoSphere &sphere);

		void addLight(const bbe::Vector3& pos, float lightStrength, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode);
		void addLight(const bbe::PointLight& light);

#ifdef BBE_RENDERER_VULKAN
		void drawTerrain(const Terrain &terrain);
#endif

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);

		void setCamera(const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUpVector = Vector3(0, 0, 1.0f));


		void setFillMode(FillMode fm);
		FillMode getFillMode();
	};
}