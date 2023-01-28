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
			struct PosNormalPair
			{
				bbe::Vector3 pos;
				bbe::Vector3 normal;
			};
			static_assert(alignof(PosNormalPair) == alignof(float));
			static_assert(sizeof(PosNormalPair) == (2 * 3 * sizeof(float)));

			struct Program
			{
			private:
				void compile();
				GLuint getShader(GLenum shaderType, const char* src);
				void addVertexShader(const char* src);
				void addFragmentShader(const char* src);
			public:
				GLuint vertex = 0;
				GLuint fragment = 0;
				GLuint program = 0;

				void addShaders(const char* vertexSrc, const char* fragmentSrc);
				void destroy();

				void use();

				GLuint getUniformLocation(const char* name);

				void uniform1f(GLint pos, GLfloat a);
				void uniform2f(GLint pos, GLfloat a, GLfloat b);
				void uniform3f(GLint pos, GLfloat a, GLfloat b, GLfloat c);
				void uniform3f(GLint pos, const bbe::Vector3& vec);
				void uniform4f(GLint pos, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
				void uniform4f(GLint pos, const bbe::Color& color);
				void uniform1i(GLint pos, GLint a);

				void uniformMatrix4fv(GLint pos, GLboolean transpose, const bbe::Matrix4& val);
			};

			struct Framebuffer
			{
				GLuint framebuffer = 0;
				bbe::List<GLuint> textures;
				GLsizei width = 0;
				GLsizei height = 0;
				GLuint depthBuffer = 0;

				Framebuffer();
				Framebuffer(GLsizei width, GLsizei height);

				void destroy();
				GLuint addTexture();
				void addDepthBuffer();
				void clearTextures();
				void bind();
				void finalize();
			};

			class OpenGLManager 
				: public RenderManager {
			private:
				PrimitiveBrush2D m_primitiveBrush2D;
				PrimitiveBrush3D m_primitiveBrush3D;

				GLFWwindow* m_pwindow = nullptr;

				Program m_program2d;
				Program m_program2dTex;
				Program m_program3dMrt;
				Program m_program3dLight;

				Framebuffer mrtFb;

				GLuint m_imageUvBuffer = 0;

				uint32_t m_windowWidth = 0;
				uint32_t m_windowHeight = 0;

				constexpr static uint32_t m_imguiMinImageCount = 2;
				bool m_imguiInitSuccessful = false;
				ImFont* m_pimguiFontSmall = nullptr;
				ImFont* m_pimguiFontBig = nullptr;

				Program init2dShaders();
				Program init2dTexShaders();
				Program init3dShadersMrt();
				Program init3dShadersLight();
				void initGeometryBuffer();

				void fillMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, size_t amountOfIndices);

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
				virtual void setCamera3D(const Vector3& cameraPos, const bbe::Matrix4& view, const bbe::Matrix4& projection) override;
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
