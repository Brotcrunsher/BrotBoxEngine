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
#include "BBE/OpenGL/OpenGLModel.h"
#include "BBE/FragmentShader.h"
#include "BBE/OpenGL/OpenGLFragmentShader.h"
#include <iostream>

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

GLuint bbe::INTERNAL::openGl::Program::getShader(GLenum shaderType, const bbe::String& src)
{
	GLuint shader = glCreateShader(shaderType);
	const char* csrc = src.getRaw();
	glShaderSource(shader, 1, &csrc, NULL);
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

void bbe::INTERNAL::openGl::Program::addShaders(const char* vertexSrc, const char* fragmentSrc, const bbe::List<UniformVariable>& uniformVariables)
{
	const bbe::String header = getHeader(uniformVariables);
	addVertexShader(header + vertexSrc);
	addFragmentShader(header + fragmentSrc);
	compile();
	for (const UniformVariable& uv : uniformVariables)
	{
		*(uv.cppHandle) = glGetUniformLocation(program, uv.name);
	}
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

void bbe::INTERNAL::openGl::Program::addVertexShader(const bbe::String& src)
{
	vertex = getShader(GL_VERTEX_SHADER, src);
}

void bbe::INTERNAL::openGl::Program::addFragmentShader(const bbe::String& src)
{
	fragment = getShader(GL_FRAGMENT_SHADER, src);
}

bbe::String bbe::INTERNAL::openGl::Program::getHeader(const bbe::List<UniformVariable>& uniformVariables)
{
	bbe::String retVal =
		"#version 300 es\n"
		"precision highp float;\n";
	for (const UniformVariable& uv : uniformVariables)
	{
		retVal += uv.toString();
	}
	retVal += "\n"; // This is necessary in case the shader code starts with a pre processor directive.
	return retVal;
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
	glDeleteTextures((GLsizei)textures.getLength(), textures.getRaw());
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
	for (GLenum i = 0 ; i<(GLenum)textures.getLength(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
	}
}

void bbe::INTERNAL::openGl::Framebuffer::finalize()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	for (GLenum i = 0; i<(GLenum)textures.getLength(); i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
	}

	bbe::List<GLenum> attachements;
	for (GLenum i = 0; i < (GLenum)textures.getLength(); i++)
	{
		attachements.add(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers((GLsizei)attachements.getLength(), attachements.getRaw());

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
		"in vec2 position;"
		"void main()"
		"{"
		"   float s = sin(rotation);"
		"   float c = cos(rotation);"
		"   vec2 scaledPos = position * scalePosOffset.xy;"
		"   vec2 firstTranslatedPos = scaledPos - scalePosOffset.xy * 0.5;"
		"   vec2 rotatedPos = vec2(c * firstTranslatedPos.x - s * firstTranslatedPos.y, s * firstTranslatedPos.x + c * firstTranslatedPos.y);"
		"   vec2 secondTranslatedPos = rotatedPos + scalePosOffset.xy * 0.5;"
		"	vec2 pos = (((secondTranslatedPos + scalePosOffset.zw) / screenSize * vec2(2, -2)) + vec2(-1, +1));\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);"
		"}";
	
	char const* fragmentShaderSource =
		"out vec4 outColor;"
		"void main()"
		"{"
		"	outColor = inColor;"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource,
		{
			{ UT::UT_vec2 , "screenSize"	, &screenSizePos2d},
			{ UT::UT_vec4 , "scalePosOffset", &scalePosOffsetPos2d},
			{ UT::UT_float, "rotation"		, &rotationPos2d},
			{ UT::UT_vec4 , "inColor"		, &inColorPos2d},
		});

	program.uniform2f(screenSizePos2d, (float)m_windowWidth, (float)m_windowHeight);
	program.uniform4f(scalePosOffsetPos2d, 1.f, 1.f, 0.f, 0.f);
	program.uniform4f(inColorPos2d, 1.f, 1.f, 1.f, 1.f);

	return program;
}

static GLint screenSizePos2dTex = 0;
static GLint scalePos2dTex = 0;
static GLint inColorPos2dTex = 0;
static GLint texPos2dTex = 0;
static GLint swizzleModePos = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init2dTexShaders()
{
	Program program;
	char const* vertexShaderSrc =
		"in vec2 position;\n"
		"in vec2 uv;"
		"out vec2 uvPassOn;"
		"void main()\n"
		"{\n"
		"   uvPassOn = uv;"
		"	vec2 pos = (position / screenSize * vec2(2, -2) * scale) + vec2(-1, +1);\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}";

	char const* fragmentShaderSource =
		// WebGL does not support swizzles, so we
		// have to implement them on our own.
		"#define SWIZZL_MODE_RGBA 0\n"
		"#define SWIZZL_MODE_RRRR 1\n"
		"in vec2 uvPassOn;"
		"out vec4 outColor;"
		"void main()"
		"{"
		"   vec4 texColor = texture(tex, uvPassOn);"
		"   if(swizzleMode == SWIZZL_MODE_RRRR)"
		"   {"
		"      texColor = vec4(texColor[0], texColor[0], texColor[0], texColor[0]);"
		"   }"
		"	outColor = inColor * texColor;\n"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource,
		{
			{UT::UT_int      , "swizzleMode", &swizzleModePos    },
			{UT::UT_vec2     , "screenSize" , &screenSizePos2dTex},
			{UT::UT_vec2     , "scale"      , &scalePos2dTex     },
			{UT::UT_vec4     , "inColor"    , &inColorPos2dTex   },
			{UT::UT_sampler2D, "tex"        , &texPos2dTex       }
		});

	program.uniform1i(swizzleModePos, 0);
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
static GLint modelPos3dMrt = 0;
static GLint albedoTexMrt = 0;
static GLint normalsTexMrt = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init3dShadersMrt()
{
	Program program;
	char const* vertexShaderSrc =
		"in vec3 inPos;"
		"in vec3 inNormal;"
		"in vec2 inUvCoord;"
		"out vec4 passPos;"
		"out vec4 passNormal;"
		"out vec2 passUvCoord;"
		"void main()"
		"{"
		"   vec4 worldPos = model * vec4(inPos, 1.0);"
		"   gl_Position = projection * view * worldPos * vec4(1.0, -1.0, 1.0, 1.0);"
		"   passPos = view * worldPos;"
		"   passNormal = view * model * vec4(inNormal, 0.0);"
		"   passUvCoord = vec2(inUvCoord.x, 1.0 - inUvCoord.y);" //TODO: wtf? Where does that y flip come from?
		"}";

	char const* fragmentShaderSource =
		"in vec4 passPos;"
		"in vec4 passNormal;"
		"in vec2 passUvCoord;"
		"layout (location = 0) out vec4 outPos;"
		"layout (location = 1) out vec4 outNormal;"
		"layout (location = 2) out vec4 outAlbedo;"
		"void main()"
		"{"
		"   outPos    = passPos;"
		"   outNormal = vec4(normalize(passNormal.xyz + (view * model * vec4(texture(normals, passUvCoord).xyz, 0.0)).xyz), 1.0);" // TODO HACK: Setting the alpha component to 1 to avoid it being discarded from the Texture. Can we do better?
		"   outAlbedo = inColor * texture(albedo, passUvCoord);"
		//"   outAlbedo = inColor * vec4(passUvCoord, 0.0, 1.0);"
		"}";

	program.addShaders(vertexShaderSrc, fragmentShaderSource,
		{
			{UT::UT_vec4,      "inColor"   , &inColorPos3dMrt   },
			{UT::UT_mat4,      "view"      , &viewPos3dMrt	    },
			{UT::UT_mat4,      "projection", &projectionPos3dMrt},
			{UT::UT_mat4,      "model"	   , &modelPos3dMrt	    },
			{UT::UT_sampler2D, "albedo"    , &albedoTexMrt      },
			{UT::UT_sampler2D, "normals"   , &normalsTexMrt     }
		});

	return program;
}

static GLint gPositionPos3dLight = 0;
static GLint gNormalPos3dLight = 0;
static GLint gAlbedoSpecPos3dLight = 0;
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
		"#define FALLOFF_NONE    0\n"
		"#define FALLOFF_LINEAR  1\n"
		"#define FALLOFF_SQUARED 2\n"
		"#define FALLOFF_CUBIC   3\n"
		"#define FALLOFF_SQRT    4\n"
		"in vec2 uvCoord;"
		"out vec4 outColor;"
		"void main()"
		"{"
		"   vec3 normal = texture(gNormal, uvCoord).xyz;"
		"   if(length(normal) == 0.0) { discard; }"
		"   vec3 pos = texture(gPosition, uvCoord).xyz;"
		"   vec3 albedo = texture(gAlbedoSpec, uvCoord).xyz;"
		"   vec3 toLight = lightPos - pos;"
		"   vec3 toCamera = -pos;"
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
		"   vec3 specular = pow(max(dot(R, V), 0.0), 10.0) * specularColor.xyz * lightPower;"
		"   outColor = vec4(ambient + diffuse + specular, 1.0);"
		"}";
	program.addShaders(vertexShaderSrc, fragmentShaderSource,
		{
			{UT::UT_sampler2D, "gPosition"	  , &gPositionPos3dLight     },
			{UT::UT_sampler2D, "gNormal"	  , &gNormalPos3dLight       },
			{UT::UT_sampler2D, "gAlbedoSpec"  , &gAlbedoSpecPos3dLight   },
			{UT::UT_float    , "lightStrength", &lightStrengthPos3dLight },
			{UT::UT_int      , "falloffMode"  , &falloffModePos3dLight   },
			{UT::UT_vec4     , "lightColor"	  , &lightColorPos3dLight    },
			{UT::UT_vec4     , "specularColor", &specularColorPos3dLight },
			{UT::UT_vec3     , "lightPos"	  , &lightPosPos3dLight      },
			{UT::UT_float    , "ambientFactor", &ambientFactorPos3dLight },
		});

	program.uniform1i(gPositionPos3dLight, 0);
	program.uniform1i(gNormalPos3dLight, 1);
	program.uniform1i(gAlbedoSpecPos3dLight, 2);

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

void bbe::INTERNAL::openGl::OpenGLManager::fillInternalMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, size_t amountOfIndices, const Image* albedo, const Image* normals)
{
	m_program3dMrt.use();
	glBindFramebuffer(GL_FRAMEBUFFER, mrtFb.framebuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glUniformMatrix4fv(modelPos3dMrt, 1, GL_FALSE, modelMatrix);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLint positionAttribute = glGetAttribLocation(m_program3dMrt.program, "inPos");
	glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(positionAttribute);

	GLint normalPosition = glGetAttribLocation(m_program3dMrt.program, "inNormal");
	glVertexAttribPointer(normalPosition, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(normalPosition);

	GLint uvPosition = glGetAttribLocation(m_program3dMrt.program, "inUvCoord");
	glVertexAttribPointer(uvPosition, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(uvPosition);

	if (!albedo) albedo = &white;
	if (!normals) normals = &black;

	glUniform1i(albedoTexMrt, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, toRendererData(*albedo)->tex);

	glUniform1i(normalsTexMrt, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, toRendererData(*normals)->tex);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawElements(GL_TRIANGLES, (GLsizei)amountOfIndices, GL_UNSIGNED_INT, 0);
}

bbe::INTERNAL::openGl::OpenGLImage* bbe::INTERNAL::openGl::OpenGLManager::toRendererData(const bbe::Image& image) const
{
	if (image.m_prendererData == nullptr)
	{
		return new bbe::INTERNAL::openGl::OpenGLImage(image);
	}
	else
	{
		return (bbe::INTERNAL::openGl::OpenGLImage*)image.m_prendererData;
	}
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

	{
		byte pixel[] = { 255, 255, 255, 255 };
		white.load(1, 1, pixel, bbe::ImageFormat::R8G8B8A8);
	}
	{
		byte pixel[] = { 0, 0, 0, 0 };
		black.load(1, 1, pixel, bbe::ImageFormat::R8G8B8A8);
	}
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
		bbe::Vector4 p(l.pos, 1.0f);
		p = m_view * p;
		m_program3dLight.uniform3f(lightPosPos3dLight, p.xyz());
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
	GLuint program = 0;
	GLint scalePosOffsetPos = 0;
	GLint rotationPos = 0;
	GLint screenSizePos = 0;
	if (shader)
	{
		bbe::INTERNAL::openGl::OpenGLFragmentShader* fs = nullptr;
		if (shader->m_prendererData)
		{
			fs = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)shader->m_prendererData;
		}
		else
		{
			fs = new bbe::INTERNAL::openGl::OpenGLFragmentShader(*shader);
		}
		program = fs->program;

		scalePosOffsetPos = fs->scalePosOffsetPos;
		rotationPos = fs->rotationPos;
		screenSizePos = fs->screenSizePos;
	}
	else
	{
		program = m_program2d.program;

		scalePosOffsetPos = scalePosOffsetPos2d;
		rotationPos = rotationPos2d;
	}

	glUseProgram(program);
	
	if (previousDrawCall2d != PreviousDrawCall2D::RECT)
	{
		previousDrawCall2d = PreviousDrawCall2D::RECT;
		glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLRectangle::getIbo());
	
		GLint positionAttribute = glGetAttribLocation(program, "position");
		glEnableVertexAttribArray(positionAttribute);
	
		glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
		glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if(shader) glUniform2f(screenSizePos, (float)m_windowWidth, (float)m_windowHeight);
	glUniform4f(scalePosOffsetPos, rect.getWidth(), rect.getHeight(), rect.getX(), rect.getY());
	glUniform1f(rotationPos, rotation);
	glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)OpenGLRectangle::getAmountOfIndices(), GL_UNSIGNED_INT, 0);
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
		m_program2d.uniform1f(rotationPos2d, 0);
	}

	m_program2d.uniform4f(scalePosOffsetPos2d, circle.getWidth(), circle.getHeight(), circle.getX(), circle.getY());
	glDrawElements(GL_TRIANGLE_FAN, (GLsizei)OpenGLCircle::getAmountOfIndices(), GL_UNSIGNED_INT, 0);
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

	if (image.m_format == ImageFormat::R8)
	{
		m_program2dTex.uniform1i(swizzleModePos, 1);
	}
	else
	{
		m_program2dTex.uniform1i(swizzleModePos, 0);
	}

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bbe::Vector2) * vertices.getLength(), vertices.getRaw(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);

	glUniform2f(scalePos2dTex, 1.0f, 1.0f);

	GLint positionAttribute = glGetAttribLocation(m_program2dTex.program, "position");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uvPosition = glGetAttribLocation(m_program2dTex.program, "uv");
	glEnableVertexAttribArray(uvPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_imageUvBuffer);
	glVertexAttribPointer(uvPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUniform1i(texPos2dTex, 0);
	glActiveTexture(GL_TEXTURE0);
	bbe::INTERNAL::openGl::OpenGLImage* ogi = toRendererData(image);
	glBindTexture(GL_TEXTURE_2D, ogi->tex);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillVertexIndexList2D(const uint32_t* indices, size_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale)
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

	glDrawElements(GL_TRIANGLES, (GLsizei)amountOfIndices, GL_UNSIGNED_INT, 0);

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
	m_view = view;
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCube3D(const Cube& cube)
{
	fillInternalMesh(&cube.getTransform()[0], OpenGLCube::getIbo(), OpenGLCube::getVbo(), OpenGLCube::getAmountOfIndices(), nullptr, nullptr);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillSphere3D(const IcoSphere& sphere)
{
	fillInternalMesh(&sphere.getTransform()[0], OpenGLSphere::getIbo(), OpenGLSphere::getVbo(), OpenGLSphere::getAmountOfIndices(), nullptr, nullptr);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillModel(const bbe::Matrix4& transform, const Model& model, const Image* albedo, const Image* normals)
{
	bbe::INTERNAL::openGl::OpenGLModel* ogm = nullptr;
	if (model.m_prendererData == nullptr)
	{
		ogm = new bbe::INTERNAL::openGl::OpenGLModel(model);
	}
	else
	{
		ogm = (bbe::INTERNAL::openGl::OpenGLModel*)model.m_prendererData;
	}
	fillInternalMesh(&(transform[0]), ogm->getIbo(), ogm->getVbo(), ogm->getAmountOfIndices(), albedo, normals);
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

bbe::String bbe::INTERNAL::openGl::UniformVariable::toString() const
{
	bbe::String retVal = "uniform ";
	switch (type)
	{
	case(UT::UT_int):
		retVal += "int ";
		break;
	case(UT::UT_float):
		retVal += "float ";
		break;
	case(UT::UT_vec2):
		retVal += "vec2 ";
		break;
	case(UT::UT_vec3):
		retVal += "vec3 ";
		break;
	case(UT::UT_vec4):
		retVal += "vec4 ";
		break;
	case(UT::UT_mat4):
		retVal += "mat4 ";
		break;
	case(UT::UT_sampler2D):
		retVal += "sampler2D ";
		break;
	default:
		throw bbe::IllegalStateException();
	}

	retVal += name;
	retVal += ";";

	return retVal;
}
