#include "BBE/OpenGL/OpenGLManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "BBE/FatalErrors.h"
#include "BBE/Rectangle.h"
#include "BBE/Circle.h"

bbe::INTERNAL::openGl::OpenGLManager::OpenGLManager()
{
}

void bbe::INTERNAL::openGl::OpenGLManager::init(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow* window, uint32_t initialWindowWidth, uint32_t initialWindowHeight)
{
	m_pwindow = window;

	glfwMakeContextCurrent(window);

	GLenum resp = glewInit();
	if (resp != GLEW_OK)
	{
		bbe::String errorMessage = "An error occurred while initializing GLEW: ";
		errorMessage += (const char*)glewGetErrorString(resp);
		bbe::INTERNAL::triggerFatalError(errorMessage);
	}

	{
		char const* vertexShaderSrc =
			"#version 300 es\n"
			"precision mediump float;\n"
			"in vec2 position;\n"
			"uniform vec2 screenSize;"
			"uniform vec2 scale;"
			"void main()\n"
			"{\n"
			"	vec2 pos = (position / screenSize * vec2(2, -2) * scale) + vec2(-1, +1);\n"
			"	gl_Position = vec4(pos, 0.0, 1.0);\n"
			"}";

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
		glCompileShader(vertexShader);
		GLint success = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			bbe::INTERNAL::triggerFatalError("Failed to compile shader");
		}

		char const* fragmentShaderSource =
			"#version 300 es\n"
			"precision mediump float;\n"
			"out vec4 outColor;\n"
			"uniform vec4 inColor;\n"
			"void main()\n"
			"{\n"
			"	outColor = inColor;\n"
			"}";

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		success = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			bbe::INTERNAL::triggerFatalError("Failed to compile shader");
		}

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		glUseProgram(shaderProgram);

		GLint screenSizePos = glGetUniformLocation(shaderProgram, "screenSize");
		glUniform2f(screenSizePos, (float)initialWindowWidth, (float)initialWindowHeight);

		GLint scalePos = glGetUniformLocation(shaderProgram, "scale");
		glUniform2f(scalePos, (float)1, (float)1);

		GLint inColorPos = glGetUniformLocation(shaderProgram, "inColor");
		glUniform4f(inColorPos, (float)1, (float)1, (float)1, (float)1);
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	imguiStart();
}

void bbe::INTERNAL::openGl::OpenGLManager::destroy()
{
	imguiStop();
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw2D()
{
	m_primitiveBrush2D.INTERNAL_beginDraw(m_pwindow, 0, 0, this);
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw3D()
{
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imguiStartFrame();
}

void bbe::INTERNAL::openGl::OpenGLManager::postDraw()
{
	imguiEndFrame();
	glfwSwapBuffers(m_pwindow);
	glfwPollEvents();
}

void bbe::INTERNAL::openGl::OpenGLManager::waitEndDraw()
{
}

void bbe::INTERNAL::openGl::OpenGLManager::waitTillIdle()
{
}

bbe::PrimitiveBrush2D& bbe::INTERNAL::openGl::OpenGLManager::getBrush2D()
{
	return m_primitiveBrush2D;
}

bbe::PrimitiveBrush3D& bbe::INTERNAL::openGl::OpenGLManager::getBrush3D()
{
	return m_primitiveBrush3D;
}

void bbe::INTERNAL::openGl::OpenGLManager::resize(uint32_t width, uint32_t height)
{
	GLint screenSizePos = glGetUniformLocation(shaderProgram, "screenSize");
	glUniform2f(screenSizePos, (float)width, (float)height);
}

void bbe::INTERNAL::openGl::OpenGLManager::screenshot(const bbe::String& path)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::setVideoRenderingMode(const char* path)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::setColor2D(const bbe::Color& color)
{
	GLint inColorPos = glGetUniformLocation(shaderProgram, "inColor");
	glUniform4f(inColorPos, color.r, color.g, color.b, color.a);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader)
{
	// TODO make proper implementation
	bbe::List<bbe::Vector2> vertices;
	rect.getVertices(vertices);
	for (bbe::Vector2& v : vertices)
	{
		v.rotate(rotation, rect.getCenter());
	}
	uint32_t a[] = {0, 1, 3, 1, 2, 3};
	fillVertexIndexList2D(a, 6, vertices.getRaw(), vertices.getLength(), { 0, 0 }, { 1, 1 });
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCircle2D(const Circle& circle)
{
	// TODO make proper implementation
	bbe::List<bbe::Vector2> vertices;
	circle.getVertices(vertices);
	bbe::List<uint32_t> indices;
	for (size_t i = 2; i<vertices.getLength(); i++)
	{
		indices.add(0);
		indices.add(i - 1);
		indices.add(i);
	}
	fillVertexIndexList2D(indices.getRaw(), indices.getLength(), vertices.getRaw(), vertices.getLength(), {0, 0}, {1, 1});
}

void bbe::INTERNAL::openGl::OpenGLManager::drawImage2D(const Rectangle& rect, const Image& image, float rotation)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::fillVertexIndexList2D(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale)
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bbe::Vector2) * amountOfVertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * amountOfIndices, indices, GL_STATIC_DRAW);

	GLint scalePos = glGetUniformLocation(shaderProgram, "scale");
	glUniform2f(scalePos, (float)scale.x, (float)scale.y);

	GLint positionAttribute = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawElements(GL_TRIANGLES, amountOfIndices, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

void bbe::INTERNAL::openGl::OpenGLManager::setColor3D(const bbe::Color& color)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::setCamera3D(const bbe::Matrix4& m_view, const bbe::Matrix4& m_projection)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCube3D(const Cube& cube)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::fillSphere3D(const IcoSphere& sphere)
{
}

void bbe::INTERNAL::openGl::OpenGLManager::imguiStart()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_pwindow, false);

	m_imguiInitSuccessful = ImGui_ImplOpenGL3_Init();
	if (!m_imguiInitSuccessful)
	{
		throw IllegalStateException();
	}

	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig fontConfig;
	imguiFontSmall = io.Fonts->AddFontDefault(&fontConfig);
	fontConfig.SizePixels = 26;
	imguiFontBig = io.Fonts->AddFontDefault(&fontConfig);

	ImGui_ImplOpenGL3_CreateFontsTexture();
}

void bbe::INTERNAL::openGl::OpenGLManager::imguiStop()
{
	if (m_imguiInitSuccessful)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}
}

void bbe::INTERNAL::openGl::OpenGLManager::imguiStartFrame()
{
	// TODO: Still not ideal - what if the scale is anything else than 1 or 2 (e.g. on 8k)
	float scale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, &scale, nullptr);
	ImGuiIO& io = ImGui::GetIO();
	if (scale < 1.5f)
	{
		io.FontDefault = imguiFontSmall;
	}
	else
	{
		io.FontDefault = imguiFontBig;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void bbe::INTERNAL::openGl::OpenGLManager::imguiEndFrame()
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplOpenGL3_RenderDrawData(drawData);
}

bool bbe::INTERNAL::openGl::OpenGLManager::isReadyToDraw() const
{
	return true;
}
