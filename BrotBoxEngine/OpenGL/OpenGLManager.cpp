#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/OpenGL/OpenGLImage.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "BBE/FatalErrors.h"
#include "BBE/Rectangle.h"
#include "BBE/Circle.h"
#include "BBE/Matrix4.h"
#include "BBE/OpenGL/OpenGLRectangle.h"
#include "BBE/OpenGL/OpenGLCircle.h"
#include "BBE/OpenGL/OpenGLCube.h"
#include "BBE/OpenGL/OpenGLSphere.h"
#include <iostream>

// TODO: "ExampleRotatingCubeIntersections" has visual artifacts
//       The effect gets worse with objects further away from (0/0/0). Floating Point Issue?
// TODO: "ExampleSandGame" performs much worse than on Vulkan - Why?
// TODO: Is every OpenGL Resource properly freed? How can we find that out?

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
		GLint length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

		bbe::List<char> log;
		log.resizeCapacityAndLength(length);
		glGetProgramInfoLog(program, length, &length, log.getRaw());
		std::cout << log.getRaw() << std::endl;

		bbe::INTERNAL::triggerFatalError("Failed to link program");
	}
	glUseProgram(program);
}

GLuint bbe::INTERNAL::openGl::Program::getShader(GLenum shaderType, const char* src)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

		bbe::List<char> log;
		log.resizeCapacityAndLength(length);
		glGetShaderInfoLog(shader, length, &length, log.getRaw());
		std::cout << log.getRaw() << std::endl;

		bbe::INTERNAL::triggerFatalError("Failed to compile shader");
	}
	return shader;
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

GLuint bbe::INTERNAL::openGl::Program::getUniformLocation(const char* name)
{
	const GLint pos = glGetUniformLocation(program, name);
	if (pos == -1)
	{
		// If this happens: Was the variable used? If not, the GLSL compiler could have optimized it away.
		bbe::INTERNAL::triggerFatalError("Could not find uniform!");
	}
	return pos;
}

void bbe::INTERNAL::openGl::Program::addVertexShader(const char* src)
{
	vertex = getShader(GL_VERTEX_SHADER, src);
}

void bbe::INTERNAL::openGl::Program::addFragmentShader(const char* src)
{
	fragment = getShader(GL_FRAGMENT_SHADER, src);
}

void bbe::INTERNAL::openGl::Program::uniform1f(GLint pos, GLfloat a)
{
	use();
	glUniform1f(pos, a);
}

void bbe::INTERNAL::openGl::Program::uniform2f(GLint pos, GLfloat a, GLfloat b)
{
	use();
	glUniform2f(pos, a, b);
}

void bbe::INTERNAL::openGl::Program::uniform3f(GLint pos, GLfloat a, GLfloat b, GLfloat c)
{
	use();
	glUniform3f(pos, a, b, c);
}

void bbe::INTERNAL::openGl::Program::uniform3f(GLint pos, const bbe::Vector3& vec)
{
	uniform3f(pos, vec.x, vec.y, vec.z);
}

void bbe::INTERNAL::openGl::Program::uniform4f(GLint pos, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
	use();
	glUniform4f(pos, a, b, c, d);
}

void bbe::INTERNAL::openGl::Program::uniform4f(GLint pos, const bbe::Color& color)
{
	uniform4f(pos, color.r, color.g, color.b, color.a);
}

void bbe::INTERNAL::openGl::Program::uniform1i(GLint pos, GLint a)
{
	use();
	glUniform1i(pos, a);
}

void bbe::INTERNAL::openGl::Program::uniformMatrix4fv(GLint pos, GLboolean transpose, const bbe::Matrix4& val)
{
	use();
	glUniformMatrix4fv(pos, 1, transpose, &val[0]);
}

bbe::INTERNAL::openGl::Framebuffer::Framebuffer()
{
}

bbe::INTERNAL::openGl::Framebuffer::Framebuffer(GLsizei width, GLsizei height)
	: width(width), height(height)
{
	glGenFramebuffers(1, &framebuffer);
}

void bbe::INTERNAL::openGl::Framebuffer::destroy()
{
	if (!framebuffer) return;

	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(textures.getLength(), textures.getRaw());
	glDeleteTextures(1, &depthBuffer);

	framebuffer = 0;
	textures.clear();
	depthBuffer = 0;
	width = 0;
	height = 0;
}

GLuint bbe::INTERNAL::openGl::Framebuffer::addTexture()
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	textures.add(texture);
	return texture;
}

void bbe::INTERNAL::openGl::Framebuffer::addDepthBuffer()
{
	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bbe::INTERNAL::openGl::Framebuffer::clearTextures()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bbe::INTERNAL::openGl::Framebuffer::bind()
{
	for (size_t i = 0 ; i<textures.getLength(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
	}
}

void bbe::INTERNAL::openGl::Framebuffer::finalize()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	for (size_t i = 0; i<textures.getLength(); i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
	}

	bbe::List<GLenum> attachements;
	for (size_t i = 0; i < textures.getLength(); i++)
	{
		attachements.add(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers(attachements.getLength(), attachements.getRaw());

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		triggerFatalError("Frame Buffer was not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static GLint screenSizePos2d = 0;
static GLint scalePosOffsetPos2d = 0;
static GLint rotationPos2d = 0;
static GLint inColorPos2d = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init2dShaders()
{
	Program program;
	char const* vertexShaderSrc =
		"#version 300 es\n"
		"precision highp float;\n"
		"in vec2 position;\n"
		"uniform vec2 screenSize;"
		"uniform vec4 scalePosOffset;"
		"uniform float rotation;"
		"void main()\n"
		"{\n"
		"   float s = sin(rotation);"
		"   float c = cos(rotation);"
		"   vec2 scaledPos = position * scalePosOffset.xy;"
		"   vec2 firstTranslatedPos = scaledPos - scalePosOffset.xy * 0.5;"
		"   vec2 rotatedPos = vec2(c * firstTranslatedPos.x - s * firstTranslatedPos.y, s * firstTranslatedPos.x + c * firstTranslatedPos.y);"
		"   vec2 secondTranslatedPos = rotatedPos + scalePosOffset.xy * 0.5;"
		"	vec2 pos = (((secondTranslatedPos + scalePosOffset.zw) / screenSize * vec2(2, -2)) + vec2(-1, +1));\n"
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

	screenSizePos2d = program.getUniformLocation("screenSize");
	scalePosOffsetPos2d = program.getUniformLocation("scalePosOffset");
	rotationPos2d = program.getUniformLocation("rotation");
	inColorPos2d = program.getUniformLocation("inColor");

	program.uniform2f(screenSizePos2d, (float)m_windowWidth, (float)m_windowHeight);
	program.uniform4f(scalePosOffsetPos2d, 1.f, 1.f, 0.f, 0.f);
	program.uniform4f(inColorPos2d, 1.f, 1.f, 1.f, 1.f);

	return program;
}

static GLint screenSizePos2dTex = 0;
static GLint scalePos2dTex = 0;
static GLint inColorPos2dTex = 0;
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

	screenSizePos2dTex = program.getUniformLocation("screenSize");
	scalePos2dTex = program.getUniformLocation("scale");
	inColorPos2dTex = program.getUniformLocation("inColor");

	program.uniform2f(screenSizePos2dTex, (float)m_windowWidth, (float)m_windowHeight);
	program.uniform2f(scalePos2dTex, 1.f, 1.f);
	program.uniform4f(inColorPos2dTex, 1.f, 1.f, 1.f, 1.f);

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

static GLint inColorPos3dMrt = 0;
static GLint viewPos3dMrt = 0;
static GLint projectionPos3dMrt = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init3dShadersMrt()
{
	Program program;
	char const* vertexShaderSrc =
		"#version 300 es\n"
		"precision highp float;"
		"in vec3 inPos;"
		"in vec3 inNormal;"
		"out vec4 passPos;"
		"out vec4 passNormal;"
		"out vec4 passAlbedo;"
		"uniform vec4 inColor;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"uniform mat4 model;"
		"void main()"
		"{"
		"   vec4 worldPos = model * vec4(inPos, 1.0);"
		"   gl_Position = projection * view * worldPos * vec4(1.0, -1.0, 1.0, 1.0);"
		"   passPos = worldPos;"
		"   passNormal = model * vec4(inNormal, 0.0);"
		"   passAlbedo = vec4(inColor.xyz, 1.0);"
		"}";

	char const* fragmentShaderSource =
		"#version 300 es\n"
		"precision highp float;\n"
		"in vec4 passPos;"
		"in vec4 passNormal;"
		"in vec4 passAlbedo;"
		"layout (location = 0) out vec4 outPos;"
		"layout (location = 1) out vec4 outNormal;"
		"layout (location = 2) out vec4 outAlbedo;"
		"void main()"
		"{"
		"   outPos    = passPos;"
		"   outNormal = vec4(normalize(passNormal.xyz), 1.0);" // TODO HACK: Setting the alpha component to 1 to avoid it being discarded from the Texture. Can we do better?
		"   outAlbedo = passAlbedo;"
		"}";

	program.addShaders(vertexShaderSrc, fragmentShaderSource);

	inColorPos3dMrt = program.getUniformLocation("inColor");
	viewPos3dMrt = program.getUniformLocation("view");
	projectionPos3dMrt = program.getUniformLocation("projection");

	return program;
}

static GLint gPositionPos3dLight = 0;
static GLint gNormalPos3dLight = 0;
static GLint gAlbedoSpecPos3dLight = 0;
static GLint cameraPosPos3dLight = 0;
static GLint lightPosPos3dLight = 0;
static GLint lightStrengthPos3dLight = 0;
static GLint falloffModePos3dLight = 0;
static GLint lightColorPos3dLight = 0;
static GLint specularColorPos3dLight = 0;
static GLint ambientFactorPos3dLight = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init3dShadersLight()
{
	Program program;
	char const* vertexShaderSrc =
		"#version 300 es\n"
		"precision highp float;"
		"out vec2 uvCoord;"
		"void main()"
		"{"
		"   if(gl_VertexID == 0)"
		"   {"
		"       gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);"
		"       uvCoord = vec2(0.0, 0.0);"
		"   }"
		"   if(gl_VertexID == 1)"
		"   {"
		"       gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);"
		"       uvCoord = vec2(0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 2)"
		"   {"
		"       gl_Position = vec4(1.0, 1.0, 0.0, 1.0);"
		"       uvCoord = vec2(1.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 3)"
		"   {"
		"       gl_Position = vec4(1.0, -1.0, 0.0, 1.0);"
		"       uvCoord = vec2(1.0, 0.0);"
		"   }"
		"}";

	char const* fragmentShaderSource =
		"#version 300 es\n"
		"precision highp float;\n"
		"#define FALLOFF_NONE    0\n"
		"#define FALLOFF_LINEAR  1\n"
		"#define FALLOFF_SQUARED 2\n"
		"#define FALLOFF_CUBIC   3\n"
		"#define FALLOFF_SQRT    4\n"
		"in vec2 uvCoord;"
		"out vec4 outColor;"
		"uniform sampler2D gPosition;"
		"uniform sampler2D gNormal;"
		"uniform sampler2D gAlbedoSpec;"
		"uniform float lightStrength;"
		"uniform int falloffMode;"
		"uniform vec4 lightColor;"
		"uniform vec4 specularColor;"
		"uniform vec3 lightPos;"
		"uniform vec3 cameraPos;"
		"uniform float ambientFactor;"
		"void main()"
		"{"
		"   vec3 normal = texture(gNormal, uvCoord).xyz;"
		"   if(length(normal) == 0.0) { discard; }"
		"   vec3 pos = texture(gPosition, uvCoord).xyz;"
		"   vec3 albedo = texture(gAlbedoSpec, uvCoord).xyz;"
		"   vec3 toLight = lightPos - pos;"
		"   vec3 toCamera = cameraPos - pos;"
		"   float distToLight = length(toLight);"
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
		"   vec3 L = normalize(toLight);"
		"   vec3 ambient = albedo * ambientFactor;"
		"   vec3 diffuse = max(dot(normal, L), 0.0) * (albedo * lightColor.xyz) * lightPower;"
		"   vec3 R = reflect(-L, normal);"
		"   vec3 V = normalize(toCamera);"
		"   vec3 specular = pow(max(dot(R, V), 0.0), 4.0) * specularColor.xyz * lightPower;"
		"   outColor = vec4(ambient + diffuse + specular, 1.0);"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource);

	gPositionPos3dLight     = program.getUniformLocation("gPosition");
	gNormalPos3dLight       = program.getUniformLocation("gNormal");
	gAlbedoSpecPos3dLight   = program.getUniformLocation("gAlbedoSpec");
	cameraPosPos3dLight     = program.getUniformLocation("cameraPos");
	lightPosPos3dLight      = program.getUniformLocation("lightPos");
	lightStrengthPos3dLight = program.getUniformLocation("lightStrength");
	falloffModePos3dLight   = program.getUniformLocation("falloffMode");
	lightColorPos3dLight    = program.getUniformLocation("lightColor");
	specularColorPos3dLight = program.getUniformLocation("specularColor");
	ambientFactorPos3dLight = program.getUniformLocation("ambientFactor");

	program.uniform1i(gPositionPos3dLight, 0);
	program.uniform1i(gNormalPos3dLight, 1);
	program.uniform1i(gAlbedoSpecPos3dLight, 2);

	program.uniform3f(cameraPosPos3dLight, 0.f, 0.f, 0.f);
	program.uniform3f(lightPosPos3dLight, 0.f, 0.f, 0.f);
	program.uniform1f(lightStrengthPos3dLight, 0.f);
	program.uniform1i(falloffModePos3dLight, 0);
	program.uniform4f(lightColorPos3dLight, 0.f, 0.f, 0.f, 0.f);
	program.uniform4f(specularColorPos3dLight, 0.f, 0.f, 0.f, 0.f);
	program.uniform1f(ambientFactorPos3dLight, 0.f);
	return program;
}

void bbe::INTERNAL::openGl::OpenGLManager::initGeometryBuffer()
{
	mrtFb.destroy(); // For resizing
	mrtFb = Framebuffer(m_windowWidth, m_windowHeight);
	mrtFb.addTexture();
	mrtFb.addTexture();
	mrtFb.addTexture();
	mrtFb.addDepthBuffer();
	mrtFb.finalize();
}

void bbe::INTERNAL::openGl::OpenGLManager::fillMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, size_t amountOfIndices)
{
	m_program3dMrt.use();
	glBindFramebuffer(GL_FRAMEBUFFER, mrtFb.framebuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLint modelPos = glGetUniformLocation(m_program3dMrt.program, "model");
	glUniformMatrix4fv(modelPos, 1, GL_FALSE, modelMatrix);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLint positionAttribute = glGetAttribLocation(m_program3dMrt.program, "inPos");
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(positionAttribute);

	GLint normalPosition = glGetAttribLocation(m_program3dMrt.program, "inNormal");
	glVertexAttribPointer(normalPosition, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(normalPosition);

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
	m_program3dMrt = init3dShadersMrt();
	m_program3dLight = init3dShadersLight();
	initGeometryBuffer();

	OpenGLRectangle::init();
	OpenGLCircle::init();
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
	OpenGLCircle::destroy();
	OpenGLRectangle::destroy();

	mrtFb.destroy();
	m_program3dMrt.destroy();
	glDeleteBuffers(1, &m_imageUvBuffer);
	m_program2dTex.destroy();
	m_program2d.destroy();
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw2D()
{
	// Draw the stuff of 3D
	m_program3dLight.use();
	uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };
	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	mrtFb.bind();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	for (size_t i = 0; i < pointLights.getLength(); i++)
	{
		const bbe::PointLight& l = pointLights[i];
		m_program3dLight.uniform3f(lightPosPos3dLight, l.pos);
		m_program3dLight.uniform1f(lightStrengthPos3dLight, l.lightStrength);
		m_program3dLight.uniform1i(falloffModePos3dLight, (int)l.falloffMode);
		m_program3dLight.uniform4f(lightColorPos3dLight, l.lightColor);
		m_program3dLight.uniform4f(specularColorPos3dLight, l.specularColor);
		// TODO What if we have no light? Shouldn't we still render ambient?
		//      Maybe put the ambient calculation in a seperate program, making it
		//      completely independent of any Light?
		m_program3dLight.uniform1f(ambientFactorPos3dLight, i == 0 ? 0.1f : 0.0f);


		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	glDeleteBuffers(1, &ibo);

	// Switch to 2D
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_primitiveBrush2D.INTERNAL_beginDraw(m_pwindow, m_windowWidth, m_windowHeight, this);

	previousDrawCall2d = PreviousDrawCall2D::NONE;
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw3D()
{
	glEnable(GL_DEPTH_TEST);
	m_primitiveBrush3D.INTERNAL_beginDraw(m_windowWidth, m_windowHeight, this);
	m_program3dMrt.use();
	glBindFramebuffer(GL_FRAMEBUFFER, mrtFb.framebuffer);
	mrtFb.clearTextures();
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imguiStartFrame();
	pointLights.clear();
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
	m_program2d.uniform2f(screenSizePos2d, (float)width, (float)height);
	m_program2dTex.uniform2f(screenSizePos2dTex, (float)width, (float)height);

	m_windowWidth = width;
	m_windowHeight = height;

	initGeometryBuffer();
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
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
	m_program2d   .uniform4f(inColorPos2d, color);
	m_program2dTex.uniform4f(inColorPos2dTex, color);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader)
{
	m_program2d.use();
	
	if (previousDrawCall2d != PreviousDrawCall2D::RECT)
	{
		previousDrawCall2d = PreviousDrawCall2D::RECT;
		glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLRectangle::getIbo());
	
		GLint positionAttribute = glGetAttribLocation(m_program2d.program, "position");
		glEnableVertexAttribArray(positionAttribute);
	
		glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
		glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_program2d.uniform4f(scalePosOffsetPos2d, rect.getWidth(), rect.getHeight(), rect.getX(), rect.getY());
	m_program2d.uniform1f(rotationPos2d, rotation);
	glDrawElements(GL_TRIANGLE_STRIP, OpenGLRectangle::getAmountOfIndices(), GL_UNSIGNED_INT, 0);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCircle2D(const Circle& circle)
{
	m_program2d.use();
	
	if (previousDrawCall2d != PreviousDrawCall2D::CIRCLE)
	{
		previousDrawCall2d = PreviousDrawCall2D::CIRCLE;
		glBindBuffer(GL_ARRAY_BUFFER, OpenGLCircle::getVbo());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLCircle::getIbo());

		GLint positionAttribute = glGetAttribLocation(m_program2d.program, "position");
		glEnableVertexAttribArray(positionAttribute);

		glBindBuffer(GL_ARRAY_BUFFER, OpenGLCircle::getVbo());
		glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_program2d.uniform4f(scalePosOffsetPos2d, circle.getWidth(), circle.getHeight(), circle.getX(), circle.getY());
	glDrawElements(GL_TRIANGLE_FAN, OpenGLCircle::getAmountOfIndices(), GL_UNSIGNED_INT, 0);
}

void bbe::INTERNAL::openGl::OpenGLManager::drawImage2D(const Rectangle& rect, const Image& image, float rotation)
{
	// TODO make proper implementation
	m_program2dTex.use();
	previousDrawCall2d = PreviousDrawCall2D::IMAGE;
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

	previousDrawCall2d = PreviousDrawCall2D::VERTEX_INDEX_LIST;
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bbe::Vector2) * amountOfVertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * amountOfIndices, indices, GL_STATIC_DRAW);

	m_program2d.uniform4f(scalePosOffsetPos2d, scale.x, scale.y, pos.x, pos.y);
	m_program2d.uniform1f(rotationPos2d, 0.f);

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
	m_program3dMrt.uniform4f(inColorPos3dMrt, color);
}

void bbe::INTERNAL::openGl::OpenGLManager::setCamera3D(const Vector3& cameraPos, const bbe::Matrix4& view, const bbe::Matrix4& projection)
{
	m_program3dMrt.uniformMatrix4fv(viewPos3dMrt, GL_FALSE, view);
	m_program3dMrt.uniformMatrix4fv(projectionPos3dMrt, GL_FALSE, projection);
	m_program3dLight.uniform3f(cameraPosPos3dLight, cameraPos);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCube3D(const Cube& cube)
{
	fillMesh(&cube.getTransform()[0], OpenGLCube::getIbo(), OpenGLCube::getVbo(), OpenGLCube::getAmountOfIndices());
}

void bbe::INTERNAL::openGl::OpenGLManager::fillSphere3D(const IcoSphere& sphere)
{
	fillMesh(&sphere.getTransform()[0], OpenGLSphere::getIbo(), OpenGLSphere::getVbo(), OpenGLSphere::getAmountOfIndices());
}

void bbe::INTERNAL::openGl::OpenGLManager::addLight(const bbe::Vector3& pos, float lightStrength, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode)
{
	bbe::PointLight light(pos);
	light.lightStrength = lightStrength;
	light.lightColor = lightColor;
	light.specularColor = specularColor;
	light.falloffMode = falloffMode;
	pointLights.add(light);
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
