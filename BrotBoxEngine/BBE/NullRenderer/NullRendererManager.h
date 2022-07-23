#pragma once

#include "../BBE/PrimitiveBrush2D.h"
#include "../BBE/PrimitiveBrush3D.h"
#include "../BBE/RenderManager.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace nullRenderer
		{
			class NullRendererManager 
				: public RenderManager {

				PrimitiveBrush2D m_primitiveBrush2D;
				PrimitiveBrush3D m_primitiveBrush3D;
			public:
				NullRendererManager();

				NullRendererManager(const NullRendererManager& other) = delete;
				NullRendererManager(NullRendererManager&& other) = delete;
				NullRendererManager& operator=(const NullRendererManager& other) = delete;
				NullRendererManager& operator=(NullRendererManager&& other) = delete;

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow *window, uint32_t initialWindowWidth, uint32_t initialWindowHeight) override;

				void destroy() override;
				void preDraw2D() override;
				void preDraw3D() override;
				void preDraw() override;
				void postDraw() override;
				void waitEndDraw() override;
				void waitTillIdle() override;

				bool isReadyToDraw() const override;

				bbe::PrimitiveBrush2D &getBrush2D() override;
				bbe::PrimitiveBrush3D &getBrush3D() override;

				void resize(uint32_t width, uint32_t height) override;

				void screenshot(const bbe::String& path) override;
				void setVideoRenderingMode(const char* path) override;
			};
		}
	}
}
