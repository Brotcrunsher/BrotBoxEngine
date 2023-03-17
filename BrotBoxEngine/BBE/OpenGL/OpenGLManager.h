#pragma once

#include "../BBE/glfwWrapper.h"
#include "../BBE/PrimitiveBrush2D.h"
#include "../BBE/PrimitiveBrush3D.h"
#include "../BBE/RenderManager.h"
#include "../BBE/PosNormalPair.h"
#include "../BBE/Model.h"
#include "../BBE/Image.h"

struct ImFont;

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct OpenGLImage;

			enum class UT // UniformType
			{
				UT_none, // Error Type
				UT_int,
				UT_float,
				UT_vec2,
				UT_vec3,
				UT_vec4,
				UT_mat4,
				UT_sampler2D,
			};

			struct UniformVariable
			{
				UT type = UT::UT_none;
				const char* name = nullptr;
				GLint* cppHandle = nullptr;

				bbe::String toString() const;
			};

			struct Program
			{
			private:
				void compile(const bbe::String& label);
				GLuint getShader(const bbe::String& label, GLenum shaderType, const bbe::String& src);
				void addVertexShader(const bbe::String& label, const bbe::String& src);
				void addFragmentShader(const bbe::String& label, const bbe::String& src);
				bbe::String getHeader(const bbe::List<UniformVariable>& uniformVariables);
			public:
				GLuint vertex = 0;
				GLuint fragment = 0;
				GLuint program = 0;

				void addShaders(const bbe::String& label, const char* vertexSrc, const char* fragmentSrc, const bbe::List<UniformVariable> &uniformVariables);
				void destroy();

				void use();

				void uniform1f(GLint pos, GLfloat a);
				void uniform2f(GLint pos, GLfloat a, GLfloat b);
				void uniform3f(GLint pos, GLfloat a, GLfloat b, GLfloat c);
				void uniform3f(GLint pos, const bbe::Vector3& vec);
				void uniform4f(GLint pos, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
				void uniform4f(GLint pos, const bbe::Color& color);
				void uniform1i(GLint pos, GLint a);

				void uniformMatrix4fv(GLint pos, GLboolean transpose, const bbe::Matrix4& val);
			};

			struct MrtProgram : public Program
			{
				GLint inColorPos3dMrt = 0;
				GLint viewPos3dMrt = 0;
				GLint projectionPos3dMrt = 0;
				GLint modelPos3dMrt = 0;
				GLint albedoTexMrt = 0;
				GLint normalsTexMrt = 0;
				GLint emissionsTexMrt = 0;
			};

			struct LightProgram : public Program
			{
				GLint gPositionPos3dLight = 0;
				GLint gNormalPos3dLight = 0;
				GLint gAlbedoSpecPos3dLight = 0;
				GLint gSpecular3dLight = 0;
				GLint lightPosPos3dLight = 0;
				GLint falloffModePos3dLight = 0;
				GLint lightColorPos3dLight = 0;
				GLint specularColorPos3dLight = 0;
				GLint projectionPos3dLight = 0;
				GLint lightRadiusPos = 0;
				GLint screenSize3dLight = 0;

				void setLightUniform(const bbe::PointLight& light, const bbe::Matrix4& view);
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
				GLuint addTexture(const char* label);
				void addDepthBuffer(const char* label);
				void clearTextures();
				void useAsInput();
				void finalize(const char* label);
			};

			struct InstanceData2D
			{
				bbe::Vector4 scalePosOffset;
				float rotation;
				bbe::Vector4 color;
			};

			class OpenGLManager 
				: public RenderManager {
			private:
				PrimitiveBrush2D m_primitiveBrush2D;
				PrimitiveBrush3D m_primitiveBrush3D;

				GLFWwindow* m_pwindow = nullptr;

				Program m_program2d;
				Program m_program2dTex;
				MrtProgram m_program3dMrt;
				Program m_program3dAmbient;
				Program m_programPostProcessing;
				LightProgram m_program3dLight;

				MrtProgram m_program3dMrtBaking;
				LightProgram m_program3dLightBaking;

				Framebuffer mrtFb;

				Framebuffer postProcessingFb;

				GLuint m_imageUvBuffer = 0;

				uint32_t m_windowWidth = 0;
				uint32_t m_windowHeight = 0;

				constexpr static uint32_t m_imguiMinImageCount = 2;
				bool m_imguiInitSuccessful = false;
				ImFont* m_pimguiFontSmall = nullptr;
				ImFont* m_pimguiFontBig = nullptr;

				Program init2dShaders();
				Program init2dTexShaders();
				MrtProgram init3dShadersMrt(bool baking);
				Program init3dShadersAmbient();
				Program init3dPostProcessing();
				LightProgram init3dShadersLight(bool baking);
				Framebuffer getGeometryBuffer(const bbe::String& label, uint32_t width, uint32_t height, bool baking) const;
				void initFrameBuffers();

				void fillModel(const bbe::Matrix4& transform, const Model& model, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader, GLuint framebuffer, bool baking, const bbe::Color& bakingColor);
				void fillInternalMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, size_t amountOfIndices, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader, GLuint framebuffer, bool baking, const bbe::Color& bakingColor);

				enum class PreviousDrawCall2D
				{
					NONE,
					RECT,
					RECT_SHADER,
					CIRCLE,
					IMAGE,
					VERTEX_INDEX_LIST,
				};
				PreviousDrawCall2D previousDrawCall2d = PreviousDrawCall2D::NONE;

				bbe::List<bbe::PointLight> pointLights;
				bbe::Matrix4 m_view;
				bbe::Matrix4 m_projection;
				bbe::Vector3 m_cameraPos;

				bbe::Color m_color2d;
				bbe::Color m_color3d;

				bbe::List<InstanceData2D> instanceDatas;
				void addInstancedData2D(PreviousDrawCall2D type, float x, float y, float width, float height, float rotation);
				void flushInstanceData2D();

				OpenGLImage* toRendererData(const bbe::Image& image) const;

				void drawLight(const bbe::PointLight& light, bool baking, GLuint ibo = 0);

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
				virtual void fillVertexIndexList2D(const uint32_t* indices, size_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale) override;

				virtual void setColor3D(const bbe::Color& color) override;
				virtual void setCamera3D(const Vector3& cameraPos, const bbe::Matrix4& view, const bbe::Matrix4& projection) override;
				virtual void fillCube3D(const Cube& cube) override;
				virtual void fillSphere3D(const IcoSphere& sphere) override;
				void fillModel(const bbe::Matrix4& transform, const Model& model, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader);
				virtual void addLight(const bbe::Vector3& pos, float lightStrengh, const bbe::Color& lightColor, const bbe::Color& specularColor, LightFalloffMode falloffMode) override;

				virtual void imguiStart() override;
				virtual void imguiStop() override;
				virtual void imguiStartFrame() override;
				virtual void imguiEndFrame() override;
				
				bbe::Image bakeLights(const bbe::Matrix4& transform, const Model& model, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader, const bbe::Color& color, const bbe::Vector2i& resolution, const bbe::List<bbe::PointLight>& lights);
				bbe::Image framebufferToImage(uint32_t width, uint32_t height) const;
			};
		}
	}
}
