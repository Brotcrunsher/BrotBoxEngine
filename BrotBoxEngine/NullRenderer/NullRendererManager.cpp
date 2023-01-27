#include "BBE/NullRenderer/NullRendererManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"

bbe::INTERNAL::nullRenderer::NullRendererManager::NullRendererManager()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::init(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow* window, uint32_t initialWindowWidth, uint32_t initialWindowHeight)
{
	m_pwindow = window;
	imguiStart();
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::destroy()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::preDraw2D()
{
	m_primitiveBrush2D.INTERNAL_beginDraw(m_pwindow, 0, 0, this);
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::preDraw3D()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::preDraw()
{
	imguiStartFrame();
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::postDraw()
{
	imguiEndFrame();
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

void bbe::INTERNAL::nullRenderer::NullRendererManager::setColor2D(const bbe::Color& color)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::fillCircle2D(const Circle& circle)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::drawImage2D(const Rectangle& rect, const Image& image, float rotation)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::fillVertexIndexList2D(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::setColor3D(const bbe::Color& color)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::setCamera3D(const bbe::Vector3& pos, const bbe::Matrix4& m_view, const bbe::Matrix4& m_projection)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::fillCube3D(const Cube& cube)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::fillSphere3D(const IcoSphere& sphere)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::addLight(const bbe::Vector3& pos, float lightStrenght, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode)
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::imguiStart()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig fontConfig;
	io.Fonts->AddFontDefault(&fontConfig);

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::imguiStop()
{
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::imguiStartFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)1280, (float)720);
	ImGui::NewFrame();
}

void bbe::INTERNAL::nullRenderer::NullRendererManager::imguiEndFrame()
{
	ImGui::Render();
}

bool bbe::INTERNAL::nullRenderer::NullRendererManager::isReadyToDraw() const
{
	return true;
}
