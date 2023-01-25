#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/OpenGL/OpenGLImage.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "BBE/FatalErrors.h"
#include "BBE/Rectangle.h"
#include "BBE/Circle.h"
#include "BBE/Matrix4.h"
#include "BBE/OpenGL/OpenGLCube.h"
#include "BBE/OpenGL/OpenGLSphere.h"

// TODO: Fix switching between fullHD and 4k Screen

void bbe::INTERNAL::openGl::Program::compile()
{
	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		bbe::INTERNAL::triggerFatalError("Failed to link program");
	}
	glUseProgram(program);
}

void bbe::INTERNAL::openGl::Program::addShaders(const char* vertexSrc, const char* fragmentSrc)
{
	addVertexShader(vertexSrc);
	addFragmentShader(fragmentSrc);
	compile();
}

void bbe::INTERNAL::openGl::Program::destroy()
{
	glDeleteProgram(program);
	glDeleteShader(fragment);
	glDeleteShader(vertex);
}

void bbe::INTERNAL::openGl::Program::use()
{
	glUseProgram(program);
}

void bbe::INTERNAL::openGl::Program::addVertexShader(const char* src)
{
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &src, NULL);
	glCompileShader(vertex);
	GLint success = 0;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		bbe::INTERNAL::triggerFatalError("Failed to compile shader");
	}
}

void bbe::INTERNAL::openGl::Program::addFragmentShader(const char* src)
{
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &src, NULL);
	glCompileShader(fragment);
	GLint success = 0;
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		bbe::INTERNAL::triggerFatalError("Failed to compile shader");
	}
}

void bbe::INTERNAL::openGl::Program::uniform1f(const char* name, GLfloat a)
{
	use();
	const GLint pos = glGetUniformLocation(program, name);
	glUniform1f(pos, a);
}

void bbe::INTERNAL::openGl::Program::uniform2f(const char* name, GLfloat a, GLfloat b)
{
	use();
	const GLint pos = glGetUniformLocation(program, name);
	glUniform2f(pos, a, b);
}

void bbe::INTERNAL::openGl::Program::uniform3f(const char* name, GLfloat a, GLfloat b, GLfloat c)
{
	use();
	const GLint pos = glGetUniformLocation(program, name);
	glUniform3f(pos, a, b, c);
}

void bbe::INTERNAL::openGl::Program::uniform3f(const char* name, const bbe::Vector3& vec)
{
	uniform3f(name, vec.x, vec.y, vec.z);
}

void bbe::INTERNAL::openGl::Program::uniform4f(const char* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
	use();
	const GLint pos = glGetUniformLocation(program, name);
	glUniform4f(pos, a, b, c, d);
}

void bbe::INTERNAL::openGl::Program::uniform4f(const char* name, const bbe::Color& color)
{
	uniform4f(name, color.r, color.g, color.b, color.a);
}

void bbe::INTERNAL::openGl::Program::uniform1i(const char* name, GLint a)
{
	use();
	const GLint pos = glGetUniformLocation(program, name);
	glUniform1i(pos, a);
}

void bbe::INTERNAL::openGl::Program::uniformMatrix4fv(const char* name, GLboolean transpose, const bbe::Matrix4& val)
{
	use();
	const GLint pos = glGetUniformLocation(program, name);
	glUniformMatrix4fv(pos, 1, transpose, &val[0]);
}

bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init2dShaders()
{
	Program program;
	char const* vertexShaderSrc =
		"#version 300 es\n"
		"precision highp float;\n"
		"in vec2 position;\n"
		"uniform vec2 screenSize;"
		"uniform vec2 scale;"
		"void main()\n"
		"{\n"
		"	vec2 pos = (position / screenSize * vec2(2, -2) * scale) + vec2(-1, +1);\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}";
	
	char const* fragmentShaderSource =
		"#version 300 es\n"
		"precision highp float;\n"
		"out vec4 outColor;\n"
		"uniform vec4 inColor;\n"
		"void main()\n"
		"{\n"
		"	outColor = inColor;\n"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource);

	program.uniform2f("screenSize", (float)m_windowWidth, (float)m_windowHeight);
	program.uniform2f("scale", 1.f, 1.f);
	program.uniform4f("inColor", 1.f, 1.f, 1.f, 1.f);

	return program;
}

bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init2dTexShaders()
{
	Program program;
	char const* vertexShaderSrc =
		"#version 300 es\n"
		"precision highp float;\n"
		"in vec2 position;\n"
		"in vec2 uv;"
		"out vec2 uvPassOn;"
		"uniform vec2 screenSize;"
		"uniform vec2 scale;"
		"void main()\n"
		"{\n"
		"   uvPassOn = uv;"
		"	vec2 pos = (position / screenSize * vec2(2, -2) * scale) + vec2(-1, +1);\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}";

	char const* fragmentShaderSource =
		"#version 300 es\n"
		"precision highp float;\n"
		"in vec2 uvPassOn;"
		"out vec4 outColor;\n"
		"uniform vec4 inColor;\n"
		"uniform sampler2D tex;"
		"void main()\n"
		"{\n"
		"	outColor = inColor * texture(tex, uvPassOn);\n"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource);

	program.uniform2f("screenSize", (float)m_windowWidth, (float)m_windowHeight);
	program.uniform2f("scale", 1.f, 1.f);
	program.uniform4f("inColor", 1.f, 1.f, 1.f, 1.f);

	bbe::List<float> uvCoordinates = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};
	glGenBuffers(1, &m_imageUvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_imageUvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvCoordinates.getLength(), uvCoordinates.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return program;
}

bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init3dShaders()
{
	Program program;
	char const* vertexShaderSrc =
		"#version 300 es\n"
		"precision highp float;"
		"in vec3 inPos;"
		"in vec3 inNormal;"
		"out vec3 outNormal;"
		"out vec3 outViewVec;"
		"out vec3 outLightVec;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"uniform mat4 model;"
		"uniform vec3 lightPos;"
		"void main()"
		"{"
		"   vec4 worldPos = model * vec4(inPos, 1.0);"
		"   gl_Position = projection * view * worldPos * vec4(1.0, -1.0, 1.0, 1.0);"
		"   outNormal = mat3(view) * mat3(model) * inNormal;"
		"   outViewVec = -(view * worldPos).xyz;"
		"   outLightVec = mat3(view) * (lightPos - vec3(worldPos));"
		"}";

	char const* fragmentShaderSource =
		"#version 300 es\n"
		"precision highp float;\n"
		"#define FALLOFF_NONE    0\n"
		"#define FALLOFF_LINEAR  1\n"
		"#define FALLOFF_SQUARED 2\n"
		"#define FALLOFF_CUBIC   3\n"
		"#define FALLOFF_SQRT    4\n"
		"out vec4 outColor;"
		"in vec3 outNormal;"
		"in vec3 outViewVec;"
		"in vec3 outLightVec;"
		"uniform vec4 inColor;"
		"uniform float lightStrength;"
		"uniform int falloffMode;"
		"uniform vec4 lightColor;"
		"uniform vec4 specularColor;"
		"void main()"
		"{"
		"   vec3 texColor = inColor.xyz;"
		"   vec3 N = normalize(outNormal);"
		"   vec3 V = normalize(outViewVec);"
		"   vec3 ambient = texColor * 0.1;"
		"   vec3 diffuse = vec3(0);"
		"   vec3 specular = vec3(0);"
		"   float distToLight = length(outLightVec);"
		"   float lightPower = lightStrength;"
		"   if(distToLight > 0.f)"
		"   {"
		"       switch (falloffMode)														 "
		"       {																							 "
		"       case FALLOFF_NONE:																			 "
		"       																				 "
		"       	break;																					 "
		"       case FALLOFF_LINEAR:																		 "
		"       	lightPower = lightPower / distToLight;							 "
		"       	break;																					 "
		"       case FALLOFF_SQUARED:																		 "
		"       	lightPower = lightPower / distToLight / distToLight;				 "
		"       	break;																					 "
		"       case FALLOFF_CUBIC:																			 "
		"       	lightPower = lightPower / distToLight / distToLight / distToLight; "
		"       	break;																					 "
		"       case FALLOFF_SQRT:																			 "
		"       	lightPower = lightPower / sqrt(distToLight);						 "
		"       	break;																					 "
		"       }																							 "
		"   }"
		"   vec3 L = normalize(outLightVec);"
		"   vec3 R = reflect(-L, N);"
		"   diffuse += max(dot(N, L), 0.0) * (texColor * lightColor.xyz) * lightPower;"
		"   specular += pow(max(dot(R, V), 0.0), 4.0) * specularColor.xyz * lightPower;"
		"	outColor = vec4(ambient + diffuse + specular, 1.0);"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource);

	program.uniform3f("lightPos", 0.f, 0.f, 0.f);
	program.uniform1f("lightStrength", 0.f);
	program.uniform1i("falloffMode", 0);
	program.uniform4f("lightColor", 0.f, 0.f, 0.f, 0.f);
	program.uniform4f("specularColor", 0.f, 0.f, 0.f, 0.f);

	return program;
}

void bbe::INTERNAL::openGl::OpenGLManager::fillMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, GLuint nbo, size_t amountOfIndices)
{
	m_program3d.use();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLint modelPos = glGetUniformLocation(m_program3d.program, "model");
	glUniformMatrix4fv(modelPos, 1, GL_FALSE, modelMatrix);

	GLint positionAttribute = glGetAttribLocation(m_program3d.program, "inPos");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLint normalPosition = glGetAttribLocation(m_program3d.program, "inNormal");
	glEnableVertexAttribArray(normalPosition);
	glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glVertexAttribPointer(normalPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawElements(GL_TRIANGLES, amountOfIndices, GL_UNSIGNED_INT, 0);
}

bbe::INTERNAL::openGl::OpenGLManager::OpenGLManager()
{
}

void bbe::INTERNAL::openGl::OpenGLManager::init(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow* window, uint32_t initialWindowWidth, uint32_t initialWindowHeight)
{
	m_pwindow = window;

	m_windowWidth = initialWindowWidth;
	m_windowHeight = initialWindowHeight;

	glfwMakeContextCurrent(window);

	GLenum resp = glewInit();
	if (resp != GLEW_OK)
	{
		bbe::String errorMessage = "An error occurred while initializing GLEW: ";
		errorMessage += (const char*)glewGetErrorString(resp);
		bbe::INTERNAL::triggerFatalError(errorMessage);
	}

	m_program2d = init2dShaders();
	m_program2dTex = init2dTexShaders();
	m_program3d = init3dShaders();

	OpenGLCube::init();
	OpenGLSphere::init();

	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	imguiStart();
}

void bbe::INTERNAL::openGl::OpenGLManager::destroy()
{
	imguiStop();

	OpenGLSphere::destroy();
	OpenGLCube::destroy();

	m_program3d.destroy();
	glDeleteBuffers(1, &m_imageUvBuffer);
	m_program2dTex.destroy();
	m_program2d.destroy();
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw2D()
{
	glDisable(GL_DEPTH_TEST);
	m_primitiveBrush2D.INTERNAL_beginDraw(m_pwindow, m_windowWidth, m_windowHeight, this);
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw3D()
{
	glEnable(GL_DEPTH_TEST);
	m_primitiveBrush3D.INTERNAL_beginDraw(m_windowWidth, m_windowHeight, this);
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
	m_program2d.uniform2f("screenSize", (float)width, (float)height);

	m_windowWidth = width;
	m_windowHeight = height;
}

void bbe::INTERNAL::openGl::OpenGLManager::screenshot(const bbe::String& path)
{
	// TODO
}

void bbe::INTERNAL::openGl::OpenGLManager::setVideoRenderingMode(const char* path)
{
	// TODO
}

void bbe::INTERNAL::openGl::OpenGLManager::setColor2D(const bbe::Color& color)
{
	m_program2d   .uniform4f("inColor", color);
	m_program2dTex.uniform4f("inColor", color);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader)
{
	// TODO make proper implementation
	m_program2d.use();
	bbe::List<bbe::Vector2> vertices;
	rect.getVertices(vertices);
	for (bbe::Vector2& v : vertices)
	{
		v = v.rotate(rotation, rect.getCenter());
	}
	uint32_t indices[] = {0, 1, 3, 1, 2, 3};
	fillVertexIndexList2D(indices, 6, vertices.getRaw(), vertices.getLength(), { 0, 0 }, { 1, 1 });
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCircle2D(const Circle& circle)
{
	// TODO make proper implementation
	m_program2d.use();
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
	// TODO make proper implementation
	m_program2dTex.use();
	bbe::List<bbe::Vector2> vertices;
	rect.getVertices(vertices);
	for (bbe::Vector2& v : vertices)
	{
		v = v.rotate(rotation, rect.getCenter());
	}
	uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bbe::Vector2) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);

	GLint scalePos = glGetUniformLocation(m_program2dTex.program, "scale");
	glUniform2f(scalePos, 1.0f, 1.0f);

	GLint positionAttribute = glGetAttribLocation(m_program2dTex.program, "position");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uvPosition = glGetAttribLocation(m_program2dTex.program, "uv");
	glEnableVertexAttribArray(uvPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_imageUvBuffer);
	glVertexAttribPointer(uvPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint texPosition = glGetUniformLocation(m_program2dTex.program, "tex");
	glUniform1i(texPosition, 0);
	glActiveTexture(GL_TEXTURE0);

	bbe::INTERNAL::openGl::OpenGLImage* ogi = nullptr;
	if (image.m_prendererData == nullptr)
	{
		ogi = new bbe::INTERNAL::openGl::OpenGLImage(image);
	}
	else
	{
		ogi = (bbe::INTERNAL::openGl::OpenGLImage*)image.m_prendererData;
	}

	glBindTexture(GL_TEXTURE_2D, ogi->tex);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillVertexIndexList2D(const uint32_t* indices, uint32_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale)
{
	m_program2d.use();

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bbe::Vector2) * amountOfVertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * amountOfIndices, indices, GL_STATIC_DRAW);

	GLint scalePos = glGetUniformLocation(m_program2d.program, "scale");
	glUniform2f(scalePos, (float)scale.x, (float)scale.y);

	GLint positionAttribute = glGetAttribLocation(m_program2d.program, "position");
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
	m_program3d.uniform4f("inColor", color);
}

void bbe::INTERNAL::openGl::OpenGLManager::setCamera3D(const bbe::Matrix4& view, const bbe::Matrix4& projection)
{
	m_program3d.uniformMatrix4fv("view", GL_FALSE, view);
	m_program3d.uniformMatrix4fv("projection", GL_FALSE, projection);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCube3D(const Cube& cube)
{
	fillMesh(&cube.getTransform()[0], OpenGLCube::getIbo(), OpenGLCube::getVbo(), OpenGLCube::getNbo(), OpenGLCube::getAmountOfIndices());
}

void bbe::INTERNAL::openGl::OpenGLManager::fillSphere3D(const IcoSphere& sphere)
{
	fillMesh(&sphere.getTransform()[0], OpenGLSphere::getIbo(), OpenGLSphere::getVbo(), OpenGLSphere::getNbo(), OpenGLSphere::getAmountOfIndices());
}

void bbe::INTERNAL::openGl::OpenGLManager::addLight(const bbe::Vector3& pos, float lightStrength, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode)
{
	m_program3d.uniform3f("lightPos", pos);
	m_program3d.uniform1f("lightStrength", lightStrength);
	m_program3d.uniform1i("falloffMode", (int)falloffMode);
	m_program3d.uniform4f("lightColor", lightColor);
	m_program3d.uniform4f("specularColor", specularColor);
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
	m_pimguiFontSmall = io.Fonts->AddFontDefault(&fontConfig);
	fontConfig.SizePixels = 26;
	m_pimguiFontBig = io.Fonts->AddFontDefault(&fontConfig);

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
		io.FontDefault = m_pimguiFontSmall;
	}
	else
	{
		io.FontDefault = m_pimguiFontBig;
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
