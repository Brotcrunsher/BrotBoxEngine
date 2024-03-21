#pragma once

#include "../BBE/glfwWrapper.h"
#include "../BBE/Matrix4.h"
#include "../BBE/Cube.h"
#include "../BBE/IcoSphere.h"
#include "../BBE/FillMode.h"
#include "../BBE/ViewFrustum.h"
#include "../BBE/RenderManager.h"
#include "../BBE/FragmentShader.h"
#include "../BBE/Color.h"
#include "../BBE/Model.h"
#include "../BBE/Future.h"
#include "../BBE/RenderMode.h"
#include "../BBE/LightBaker.h"

namespace bbe
{
	template<typename T, T maxValue>
	class Color_t;
	using Color = Color_t<float, 1.0f>;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
		namespace nullRenderer
		{
			class NullRendererManager;
		}
		namespace openGl
		{
			class OpenGLManager;
		}
	}

	class PrimitiveBrush3D
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class INTERNAL::nullRenderer::NullRendererManager;
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

		bbe::Model m_rectangle;
		bbe::Model m_cube;
	public:
		PrimitiveBrush3D();

		void fillCube(const bbe::Vector3& pos);
		void fillCube(const Cube &cube);
		void fillIcoSphere(const IcoSphere &sphere);
		void fillLine(const bbe::Vector3& a, const bbe::Vector3& b, float lineWidth = 0.1f);

		void addLight(const bbe::Vector3& pos, float lightStrength, const bbe::Color& lightColor, const bbe::Color& specularColor, LightFalloffMode falloffMode);
		void addLight(const bbe::PointLight& light);

#ifdef BBE_RENDERER_OPENGL
		bbe::Future<bool> isCubeVisible(const Cube& cube);
		void fillRectangle(const bbe::Matrix4& transform, const Image* albedo = nullptr, const Image* normals = nullptr, const Image* emissions = nullptr, const bbe::FragmentShader* shader = nullptr);
		void fillCube(const Cube& cube, const Image* albedo, const Image* normals = nullptr, const Image* emissions = nullptr, const bbe::FragmentShader* shader = nullptr);
		void fillModel(const bbe::Matrix4& transform, const bbe::Model& model, const Image* albedo = nullptr, const Image* normals = nullptr, const Image* emissions = nullptr, const bbe::FragmentShader* shader = nullptr);
		void setRenderMode(bbe::RenderMode renderMode);

		void bakeLightMrt(bbe::LightBaker& lightBaker);
		void bakeLight(bbe::LightBaker& lightBaker, const bbe::PointLight& light);
		void bakeLightGammaCorrect(bbe::LightBaker& lightBaker);
		bbe::Image bakeLightDetach(bbe::LightBaker& lightBaker);
#endif

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);

		void setColorHSV(float h, float s, float v);

		void setCamera(const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUpVector = Vector3(0, 0, 1.0f));

		void setFillMode(FillMode fm);
		FillMode getFillMode();
	};
}