#pragma once

#include "../BBE/glfwWrapper.h"
#include "../BBE/PrimitiveBrush2D.h"
#include "../BBE/PrimitiveBrush3D.h"
#include "../BBE/RenderManager.h"

struct ImFont;

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct Program
			{
			private:
				void compile();
				void addVertexShader(const char* src);
				void addFragmentShader(const char* src);
			public:
				GLuint vertex = 0;
				GLuint fragment = 0;
				GLuint program = 0;

				void addShaders(const char* vertexSrc, const char* fragmentSrc);
				void destroy();

				void use();

				void uniform1f(const char* name, GLfloat a);
				void uniform2f(const char* name, GLfloat a, GLfloat b);
				void uniform3f(const char* name, GLfloat a, GLfloat b, GLfloat c);
				void uniform3f(const char* name, const bbe::Vector3& vec);
				void uniform4f(const char* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
				void uniform4f(const char* name, const bbe::Color& color);
				void uniform1i(const char* name, GLint a);

				void uniformMatrix4fv(const char* name, GLboolean transpose, const bbe::Matrix4& val);
			};

			class OpenGLManager 
				: public RenderManager {
			private:
				PrimitiveBrush2D m_primitiveBrush2D;
				PrimitiveBrush3D m_primitiveBrush3D;

				GLFWwindow* m_pwindow = nullptr;

				Program m_program2d;
				Program m_program2dTex;
				Program m_program3d;

				GLuint m_imageUvBuffer = 0;

				uint32_t m_windowWidth = 0;
				uint32_t m_windowHeight = 0;

				constexpr static uint32_t m_imguiMinImageCount = 2;
				bool m_imguiInitSuccessful = false;
				ImFont* m_pimguiFontSmall = nullptr;
				ImFont* m_pimguiFontBig = nullptr;

				Program init2dShaders();
				Program init2dTexShaders();
				Program init3dShaders();

				void fillMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, GLuint nbo, size_t amountOfIndices);

			public:
				OpenGLManager();

				OpenGLManager(const OpenGLManager& other) = delete;
				OpenGLManager(OpenGLManager&& other) = delete;
				OpenGLManager& operator=(const OpenGLManager& other) = delete;
				OpenGLManager& operator=(OpenGLManager&& other) = delete;

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

				virtual void setColor2D(const bbe::Color& color) override;
				virtual void fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader) override;
				virtual void fillCircle2D(const Circle& circle) override;
				virtual void drawImage2D(const Rectangle& rect, const Image& image, float rotation) override;
				virtual void fillVertexIndexList2D(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale) override;

				virtual void setColor3D(const bbe::Color& color) override;
				virtual void setCamera3D(const bbe::Matrix4& view, const bbe::Matrix4& projection) override;
				virtual void fillCube3D(const Cube& cube) override;
				virtual void fillSphere3D(const IcoSphere& sphere) override;
				virtual void addLight(const bbe::Vector3& pos, float lightStrengh, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode) override;

				virtual void imguiStart() override;
				virtual void imguiStop() override;
				virtual void imguiStartFrame() override;
				virtual void imguiEndFrame() override;

			};
		}
	}
}
