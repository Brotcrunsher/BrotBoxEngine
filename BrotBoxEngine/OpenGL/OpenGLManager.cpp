#include "BBE/OpenGL/OpenGLManager.h"
#include "BBE/OpenGL/OpenGLImage.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
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
#include "BBE/OpenGL/OpenGLLightBaker.h"
#include "BBE/Logging.h"
#include <iostream>

// TODO: Is every OpenGL Resource properly freed? How can we find that out?

static void addLabel(GLenum identifier, GLuint name, const char* label)
{
#ifdef _DEBUG
	glObjectLabel(identifier, name, -1, label);
#endif
}

static GLuint genTexture(const char* label)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	addLabel(GL_TEXTURE, texture, label);
	return texture;
}


static GLuint genTextureMultisampled(const char* label)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
	//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	addLabel(GL_TEXTURE, texture, label);
	return texture;
}

enum class BufferTarget
{
	ARRAY_BUFFER = GL_ARRAY_BUFFER,
	ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
};
static GLuint genBuffer(const char* label, BufferTarget target, size_t length, const void* data)
{
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	glBindBuffer((GLenum)target, buffer);
	glBufferData((GLenum)target, length, data, GL_STATIC_DRAW);
	addLabel(GL_BUFFER, buffer, label);
	return buffer;
}

static GLuint createShader(const char* label, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	addLabel(GL_SHADER, shader, label);
	return shader;
}

static GLuint createProgram(const char* label)
{
	GLuint program = glCreateProgram();
	addLabel(GL_PROGRAM, program, label);
	return program;
}

static GLuint genFramebuffer()
{
	GLuint framebuffer = 0;
	glGenFramebuffers(1, &framebuffer);
	return framebuffer;
}

void bbe::INTERNAL::openGl::Program::compile(const bbe::String& label)
{
	program = createProgram((label + "(Program)").getRaw());
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
		BBELOGLN(log.getRaw());

		bbe::INTERNAL::triggerFatalError("Failed to link program");
	}
	glUseProgram(program);
}

GLuint bbe::INTERNAL::openGl::Program::getShader(const bbe::String& label, GLenum shaderType, const bbe::String& src)
{
	GLuint shader = createShader(label.getRaw(), shaderType);
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
		BBELOGLN(log.getRaw());

		bbe::INTERNAL::triggerFatalError("Failed to compile shader");
	}
	return shader;
}

void bbe::INTERNAL::openGl::Program::addShaders(const bbe::String& label, const char* vertexSrc, const char* fragmentSrc, const bbe::List<UniformVariable>& uniformVariables)
{
	const bbe::String header = getHeader(uniformVariables);
	addVertexShader(label, header + vertexSrc);
	addFragmentShader(label, header + fragmentSrc);
	compile(label);
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

void bbe::INTERNAL::openGl::Program::addVertexShader(const bbe::String& label, const bbe::String& src)
{
	vertex = getShader(label + "(VertexShader)", GL_VERTEX_SHADER, src);
}

void bbe::INTERNAL::openGl::Program::addFragmentShader(const bbe::String& label, const bbe::String& src)
{
	fragment = getShader(label + "(FragmentShader)", GL_FRAGMENT_SHADER, src);
}

bbe::String bbe::INTERNAL::openGl::Program::getHeader(const bbe::List<UniformVariable>& uniformVariables)
{
	bbe::String retVal;
#ifdef __APPLE__
	retVal =
		"#version 330 core\n";
#else
	retVal =
		"#version 300 es\n"
		"precision highp float;\n"
		"precision highp int;\n"; // Actually required! Intel Drivers seem to have different default precisions of int between vertex and frament shaders, leading to linker issues.
#endif

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
	: framebuffer(genFramebuffer()), width(width), height(height)
{
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

GLuint bbe::INTERNAL::openGl::Framebuffer::addTexture(const char* label, uint32_t bytes)
{
	GLuint texture = 0;
	
#ifndef __EMSCRIPTEN__
	if (samples == 1)
#endif
	{
		texture = genTexture(label);
		if (bytes == 1)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		else if (bytes == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		}
		else
		{
			bbe::Crash(bbe::Error::IllegalArgument);
		}
	}
#ifndef __EMSCRIPTEN__
	else
	{
		texture = genTextureMultisampled(label);
		if (bytes == 1)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, GL_TRUE);
		}
		else if (bytes == 4)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA32F, width, height, GL_TRUE);
		}
		else
		{
			bbe::Crash(bbe::Error::IllegalArgument);
		}
	}
#endif
	textures.add(texture);
	return texture;
}

void bbe::INTERNAL::openGl::Framebuffer::addDepthBuffer(const char* label)
{
#ifndef __EMSCRIPTEN__
	if (samples == 1)
#endif
	{
		depthBuffer = genTexture(label);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
#ifndef __EMSCRIPTEN__
	else
	{
		depthBuffer = genTextureMultisampled(label);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthBuffer, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
#endif
}

void bbe::INTERNAL::openGl::Framebuffer::clearTextures()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bbe::INTERNAL::openGl::Framebuffer::useAsInput()
{
	for (GLenum i = 0; i < (GLenum)textures.getLength(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
	}
}

void bbe::INTERNAL::openGl::Framebuffer::finalize(const char* label)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	for (GLenum i = 0; i < (GLenum)textures.getLength(); i++)
	{
		if (samples == 1) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
		else              glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, textures[i], 0);
	}

	bbe::List<GLenum> attachements;
	for (GLenum i = 0; i < (GLenum)textures.getLength(); i++)
	{
		attachements.add(GL_COLOR_ATTACHMENT0 + i);
	}
	glDrawBuffers((GLsizei)attachements.getLength(), attachements.getRaw());

	addLabel(GL_FRAMEBUFFER, framebuffer, label);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bbe::INTERNAL::openGl::Framebuffer::setSamples(GLsizei samples)
{
	this->samples = samples;
}

static GLint screenSizePos2d = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init2dShaders()
{
	Program program;
	char const* vertexShaderSrc =
		"layout (location = 0) in vec2 position;"
		"layout (location = 1) in vec4 scalePosOffset;"
		"layout (location = 2) in float rotation;"
		"layout (location = 3) in vec4 inColor;"
		"flat out vec4 passColor;"
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
		"   passColor = inColor;"
		"}";

	char const* fragmentShaderSource =
		"flat in vec4 passColor;"
		"out vec4 outColor;"
		"void main()"
		"{"
		"	outColor = passColor;"
		"}";
	program.addShaders("2d", vertexShaderSrc, fragmentShaderSource,
		{
			{ UT::UT_vec2 , "screenSize"	, &screenSizePos2d},
		});

	program.uniform2f(screenSizePos2d, (float)m_windowWidth, (float)m_windowHeight);

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
		"      texColor = vec4(1, 1, 1, texColor[0]);"
		"   }"
		"	outColor = inColor * texColor;\n"
		"}";
	program.addShaders("2dTex", vertexShaderSrc, fragmentShaderSource,
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
	m_imageUvBuffer = genBuffer("TexShadersUvBuffer", BufferTarget::ARRAY_BUFFER, sizeof(float) * uvCoordinates.getLength(), uvCoordinates.getRaw());

	return program;
}

bbe::INTERNAL::openGl::MrtProgram bbe::INTERNAL::openGl::OpenGLManager::init3dShadersMrt(bool baking)
{
	MrtProgram program;
	bbe::String vertexShaderSrc =
		"in vec3 inPos;"
		"in vec3 inNormal;"
		"in vec2 inUvCoord;"
		"out vec4 passPos;"
		"out vec4 passNormal;"
		"out vec2 passUvCoord;"
		"void main()"
		"{"
		"   vec4 worldPos = model * vec4(inPos, 1.0);";
	if (!baking)
	{
		vertexShaderSrc +=
			"   gl_Position = projection * view * worldPos * vec4(1.0, -1.0, 1.0, 1.0);"
			"   passPos = view * worldPos;"
			"   passNormal = view * model * vec4(inNormal, 0.0);"
			"   passUvCoord = inUvCoord;";
	}
	else
	{
		vertexShaderSrc +=
			"   gl_Position = vec4(inUvCoord * 2.0 - vec2(1, 1), 0.0, 1.0);"
			"   passPos = worldPos;"
			"   passNormal = model * vec4(inNormal, 0.0);"
			"   passUvCoord = inUvCoord;";
	}
	vertexShaderSrc += "}";

	bbe::String fragmentShaderSource =
		"in vec4 passPos;"
		"in vec4 passNormal;"
		"in vec2 passUvCoord;"
		"layout (location = 0) out vec4 outPos;"
		"layout (location = 1) out vec4 outNormal;"
		"layout (location = 2) out vec4 outAlbedo;"
		"layout (location = 3) out vec4 outSpecular;"
		"layout (location = 4) out vec4 outEmission;"
		"void main()"
		"{"
		"   outPos    = passPos;";
	if (!baking)
		fragmentShaderSource += "   outNormal = vec4(normalize(passNormal.xyz + (view * model * vec4(texture(normals, passUvCoord).xyz, 0.0)).xyz), 1.0);"; // TODO HACK: Setting the alpha component to 1 to avoid it being discarded from the Texture. Can we do better?
	else
		fragmentShaderSource += "   outNormal = vec4(normalize(passNormal.xyz + (       model * vec4(texture(normals, passUvCoord).xyz, 0.0)).xyz), 1.0);"; // TODO HACK: Setting the alpha component to 1 to avoid it being discarded from the Texture. Can we do better?

	fragmentShaderSource +=
		"   outAlbedo = inColor * texture(albedo, passUvCoord);"
		"   outSpecular = vec4(10.0, 1.0, 0.0, 1.0);"
		"   outEmission = texture(emissions, passUvCoord);"
		"}";

	bbe::String label;
	if (baking) label = "3dMrtBaking";
	else label = "3dMrt";

	program.addShaders(label, vertexShaderSrc.getRaw(), fragmentShaderSource.getRaw(),
		{
			{UT::UT_vec4,      "inColor"   , &program.inColorPos3dMrt   },
			{UT::UT_mat4,      "view"      , &program.viewPos3dMrt	    },
			{UT::UT_mat4,      "projection", &program.projectionPos3dMrt},
			{UT::UT_mat4,      "model"	   , &program.modelPos3dMrt	    },
			{UT::UT_sampler2D, "albedo"    , &program.albedoTexMrt      },
			{UT::UT_sampler2D, "normals"   , &program.normalsTexMrt     },
			{UT::UT_sampler2D, "emissions" , &program.emissionsTexMrt   },
		});

	bbe::Matrix4 identity;
	program.uniformMatrix4fv(program.viewPos3dMrt, false, identity);
	program.uniformMatrix4fv(program.projectionPos3dMrt, false, identity);

	return program;
}

bbe::INTERNAL::openGl::MrtProgram bbe::INTERNAL::openGl::OpenGLManager::init3dForwardNoLight()
{
	MrtProgram program;
	bbe::String vertexShaderSrc =
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
		"   passUvCoord = inUvCoord;"
		"}";

	bbe::String fragmentShaderSource =
		"in vec4 passPos;"
		"in vec4 passNormal;"
		"in vec2 passUvCoord;"
		"layout (location = 0) out vec4 outAlbedo;"
		"void main()"
		"{"
		"   outAlbedo = inColor * texture(albedo, passUvCoord) * texture(emissions, passUvCoord);"
		"}";

	program.addShaders("ForwardNoLight", vertexShaderSrc.getRaw(), fragmentShaderSource.getRaw(),
		{
			{UT::UT_vec4,      "inColor"   , &program.inColorPos3dMrt   },
			{UT::UT_mat4,      "view"      , &program.viewPos3dMrt	    },
			{UT::UT_mat4,      "projection", &program.projectionPos3dMrt},
			{UT::UT_mat4,      "model"	   , &program.modelPos3dMrt	    },
			{UT::UT_sampler2D, "albedo"    , &program.albedoTexMrt      },
			{UT::UT_sampler2D, "normals"   , &program.normalsTexMrt     },
			{UT::UT_sampler2D, "emissions" , &program.emissionsTexMrt   },
		});

	return program;
}

static GLint gAlbedoSpecPos3dAmbient = 0;
static GLint emissionsPos3dAmbient = 0;
static GLint ambientFactorPos3dAmbient = 0;
static GLint screenSizeAmbient = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init3dShadersAmbient()
{
	Program program;
	char const* vertexShaderSrc =
		"void main()"
		"{"
		"   if(gl_VertexID == 0)"
		"   {"
		"       gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 1)"
		"   {"
		"       gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 2)"
		"   {"
		"       gl_Position = vec4(1.0, 1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 3)"
		"   {"
		"       gl_Position = vec4(1.0, -1.0, 0.0, 1.0);"
		"   }"
		"}";

	char const* fragmentShaderSource =
		"out vec4 outColor;"
		"void main()"
		"{"
		"   vec3 albedo = texture(gAlbedoSpec, gl_FragCoord.xy / screenSize).xyz;"
		"   vec3 ambient = albedo * ambientFactor;"
		"   vec3 emissions = texture(emissions, gl_FragCoord.xy / screenSize).xyz;"
		"   outColor = vec4(ambient + albedo * pow(emissions, vec3(2.2)), 1.0);"
		"}";
	program.addShaders("3dAmbient", vertexShaderSrc, fragmentShaderSource,
		{
			{UT::UT_sampler2D, "gAlbedoSpec"  , &gAlbedoSpecPos3dAmbient  },
			{UT::UT_sampler2D, "emissions"    , &emissionsPos3dAmbient},
			{UT::UT_float    , "ambientFactor", &ambientFactorPos3dAmbient},
			{UT::UT_vec2     , "screenSize",    &screenSizeAmbient        },
		});

	program.uniform1i(gAlbedoSpecPos3dAmbient, 2);
	program.uniform1i(emissionsPos3dAmbient, 4);
	program.uniform1f(ambientFactorPos3dAmbient, 0.0001f);
	program.uniform2f(screenSizeAmbient, (float)m_windowWidth, (float)m_windowHeight);
	return program;
}

static GLint framePostProcessing = 0;
static GLint screenSizePostProcessing = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::init3dPostProcessing()
{
	Program program;
	char const* vertexShaderSrc =
		"void main()"
		"{"
		"   if(gl_VertexID == 0)"
		"   {"
		"       gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 1)"
		"   {"
		"       gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 2)"
		"   {"
		"       gl_Position = vec4(1.0, 1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 3)"
		"   {"
		"       gl_Position = vec4(1.0, -1.0, 0.0, 1.0);"
		"   }"
		"}";

	char const* fragmentShaderSource =
		"out vec4 outColor;"
		"void main()"
		"{"
		"   vec3 albedo = texture(frame, gl_FragCoord.xy / screenSize).xyz;"
		"   outColor = pow(vec4(albedo, 1.0), vec4(1.0 / 2.2));"
		"}";
	program.addShaders("3dPostProcessing", vertexShaderSrc, fragmentShaderSource,
		{
			{UT::UT_sampler2D, "frame"     , &framePostProcessing     },
			{UT::UT_vec2     , "screenSize", &screenSizePostProcessing},
		});

	program.uniform1i(framePostProcessing, 0);
	program.uniform2f(screenSizePostProcessing, (float)m_windowWidth, (float)m_windowHeight);
	return program;
}

static GLint frameBakingGammaCorrection = 0;
static GLint screenSizeBakingGammaCorrection = 0;
bbe::INTERNAL::openGl::Program bbe::INTERNAL::openGl::OpenGLManager::initBakingGammaCorrection()
{
	Program program;
	char const* vertexShaderSrc =
		"void main()"
		"{"
		"   if(gl_VertexID == 0)"
		"   {"
		"       gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 1)"
		"   {"
		"       gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 2)"
		"   {"
		"       gl_Position = vec4(1.0, 1.0, 0.0, 1.0);"
		"   }"
		"   if(gl_VertexID == 3)"
		"   {"
		"       gl_Position = vec4(1.0, -1.0, 0.0, 1.0);"
		"   }"
		"}";

	char const* fragmentShaderSource =
		"out vec4 outColor;"
		"void main()"
		"{"
		"   vec3 albedo = texture(frame, gl_FragCoord.xy / screenSize).xyz;"
		"   outColor = pow(vec4(albedo, 1.0), vec4(1.0 / 2.2));"
		"}";
	program.addShaders("3dPostProcessing", vertexShaderSrc, fragmentShaderSource,
		{
			{UT::UT_sampler2D, "frame"     , &frameBakingGammaCorrection     },
			{UT::UT_vec2     , "screenSize", &screenSizeBakingGammaCorrection},
		});

	program.uniform1i(frameBakingGammaCorrection, 0);
	program.uniform2f(screenSizeBakingGammaCorrection, (float)m_windowWidth, (float)m_windowHeight);
	return program;
}

bbe::INTERNAL::openGl::LightProgram bbe::INTERNAL::openGl::OpenGLManager::init3dShadersLight(bool baking)
{
	LightProgram program;
	char const* vertexShaderSrc;

	if(!baking)
	{
		vertexShaderSrc =
			"in vec3 inPos;"
			"void main()"
			"{"
			"   vec4 worldPos = vec4(lightPos.xyz, 1.0) + lightRadius * vec4(inPos, 0.0);"
			"   gl_Position = projection * worldPos * vec4(1.0, -1.0, 1.0, 1.0);"
			"}";
	}
	else
	{
		vertexShaderSrc =
			"void main()"
			"{"
			"   if(gl_VertexID == 0)"
			"   {"
			"       gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);"
			"   }"
			"   if(gl_VertexID == 1)"
			"   {"
			"       gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);"
			"   }"
			"   if(gl_VertexID == 2)"
			"   {"
			"       gl_Position = vec4(1.0, 1.0, 0.0, 1.0);"
			"   }"
			"   if(gl_VertexID == 3)"
			"   {"
			"       gl_Position = vec4(1.0, -1.0, 0.0, 1.0);"
			"   }"
			"}";
	}

	bbe::String fragmentShaderSource =
		"#define FALLOFF_NONE    0\n"
		"#define FALLOFF_LINEAR  1\n"
		"#define FALLOFF_SQUARED 2\n"
		"#define FALLOFF_CUBIC   3\n"
		"#define FALLOFF_SQRT    4\n"
		"out vec4 outColor;"
		"void main()"
		"{"
		"   vec2 uvCoord = gl_FragCoord.xy / screenSize;"
		"   vec3 normal = texture(gNormal, uvCoord).xyz;"
		"   if(length(normal) == 0.0) { discard; }"
		"   vec3 pos = texture(gPosition, uvCoord).xyz;"
		"   vec3 toLight = lightPos.xyz - pos;"
		"   float distToLight = length(toLight);"
		"   float lightPower = lightPos.w;"
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
		"   if(lightPower < 0.001) discard;"
		"   vec3 L = normalize(toLight);";
	if(!baking)
	{
		fragmentShaderSource +=
			"   vec3 albedo = texture(gAlbedoSpec, uvCoord).xyz;"
			"   vec3 diffuse = max(dot(normal, L), 0.0) * (albedo * lightColor.xyz) * lightPower;"
			"   vec3 R = reflect(-L, normal);"
			"   vec3 toCamera = -pos;"
			"   vec3 V = normalize(toCamera);"
			"   vec3 specStats = texture(gSpecular, uvCoord).xyz;"
			"   vec3 specular = pow(max(dot(R, V), 0.0), specStats.x) * specularColor.xyz * lightPower * specStats.y;"
			"   outColor = vec4(diffuse + specular * 0.1, 1.0);"
			"}";
	}
	else
	{
		fragmentShaderSource +=
			"   vec3 diffuse = max(dot(normal, L), 0.0) * (lightColor.xyz) * lightPower;"
			"   outColor = vec4(diffuse, 1.0);"
			"}";
	}

	bbe::String label;
	if (baking) label = "3dLightBaking";
	else label = "3dLight";

	program.addShaders(label, vertexShaderSrc, fragmentShaderSource.getRaw(),
		{
			{UT::UT_sampler2D, "gPosition"	  , &program.gPositionPos3dLight     },
			{UT::UT_sampler2D, "gNormal"	  , &program.gNormalPos3dLight       },
			{UT::UT_sampler2D, "gAlbedoSpec"  , &program.gAlbedoSpecPos3dLight   },
			{UT::UT_sampler2D, "gSpecular"    , &program.gSpecular3dLight        },
			{UT::UT_mat4     , "projection"   , &program.projectionPos3dLight    },
			{UT::UT_vec2     , "screenSize"   , &program.screenSize3dLight       },

			{UT::UT_vec4     , "lightColor"	  , &program.lightColorPos3dLight    },
			{UT::UT_vec4     , "specularColor", &program.specularColorPos3dLight },
			{UT::UT_vec4     , "lightPos"	  , &program.lightPosPos3dLight      },
			{UT::UT_float    , "lightRadius"  , &program.lightRadiusPos          },
			{UT::UT_int      , "falloffMode"  , &program.falloffModePos3dLight   },
		});

	program.uniform1i(program.gPositionPos3dLight, 0);
	program.uniform1i(program.gNormalPos3dLight, 1);
	program.uniform1i(program.gAlbedoSpecPos3dLight, 2);
	program.uniform1i(program.gSpecular3dLight, 3);

	program.uniform4f(program.lightPosPos3dLight, 0.f, 0.f, 0.f, 0.f);
	program.uniform1i(program.falloffModePos3dLight, 0);
	program.uniform4f(program.lightColorPos3dLight, 0.f, 0.f, 0.f, 0.f);
	program.uniform4f(program.specularColorPos3dLight, 0.f, 0.f, 0.f, 0.f);
	program.uniform2f(program.screenSize3dLight, (float)m_windowWidth, (float)m_windowHeight);

	bbe::Matrix4 identity;
	program.uniformMatrix4fv(program.projectionPos3dLight, false, identity);
	return program;
}

bbe::INTERNAL::openGl::Framebuffer bbe::INTERNAL::openGl::OpenGLManager::getGeometryBuffer(const bbe::String& label, uint32_t width, uint32_t height, bool baking) const
{
	Framebuffer fb(width, height);
	fb.addTexture((label + "(Positions)").getRaw());
	fb.addTexture((label + "(Normals)").getRaw());
	fb.addTexture((label + "(Albedo)").getRaw());
	fb.addTexture((label + "(Specular)").getRaw());
	fb.addTexture((label + "(Emission)").getRaw());
	if (!baking)
	{
		fb.addDepthBuffer((label + "(DepthBuffer)").getRaw());
	}
	fb.finalize(label.getRaw());
	return fb;
}

void bbe::INTERNAL::openGl::OpenGLManager::initFrameBuffers()
{
	// TODO Might not be necessary to create if we stay in FORWARD_NO_LIGHT mode
	mrtFb.destroy(); // For resizing
	mrtFb = getGeometryBuffer("GeometryBuffer", m_windowWidth, m_windowHeight, false);

	// TODO Might not be necessary to create if we stay in DEFERRED_SHADING mode
	forwardNoLightFb.destroy();
	forwardNoLightFb = Framebuffer(m_windowWidth, m_windowHeight);
	forwardNoLightFb.setSamples(8);
	forwardNoLightFb.addTexture("ColorBuffer ForwardNoLight");
	forwardNoLightFb.addDepthBuffer("DepthBuffer ForwardNoLight");
	forwardNoLightFb.finalize("ForwardNoLightBuffer");

	postProcessingFb.destroy();
	postProcessingFb = Framebuffer(m_windowWidth, m_windowHeight);
	postProcessingFb.addTexture("PostProcessing");
	postProcessingFb.finalize("PostProcessingBuffer");
}

void bbe::INTERNAL::openGl::OpenGLManager::fillModel(const bbe::Matrix4& transform, const Model& model, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader, GLuint framebuffer, bool baking, const bbe::Color& bakingColor)
{
	bbe::INTERNAL::openGl::OpenGLModel* ogm = nullptr;
	if (model.m_prendererData == nullptr)
	{
		ogm = new bbe::INTERNAL::openGl::OpenGLModel(model);
	}
	else
	{
		ogm = (bbe::INTERNAL::openGl::OpenGLModel*)model.m_prendererData.get();
	}
	fillInternalMesh(&(transform[0]), ogm->getIbo(), ogm->getVbo(), ogm->getAmountOfIndices(), albedo, normals, emissions, shader, framebuffer, baking, bakingColor);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillInternalMesh(const float* modelMatrix, GLuint ibo, GLuint vbo, size_t amountOfIndices, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader, GLuint framebuffer, bool baking, const bbe::Color& bakingColor)
{
#ifdef __APPLE__
	// Generate a VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	// TODO This function wants way too much. Refactor.
	GLuint program = 0;
	GLint modelPos = 0;
	GLint albedoTex = 0;
	GLint normalsTex = 0;
	GLint emissionsTex = 0;
	bbe::INTERNAL::openGl::OpenGLFragmentShader* fs = nullptr;
	bbe::INTERNAL::openGl::OpenGLFragmentShader::ThreeD* fs3d = nullptr;
	if (shader)
	{
		if (shader->m_prendererData != nullptr)
		{
			fs = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)shader->m_prendererData.get();
		}
		else
		{
			fs = new bbe::INTERNAL::openGl::OpenGLFragmentShader(*shader);
		}

		if (!baking)
		{
			     if (m_renderMode == bbe::RenderMode::DEFERRED)          fs3d = &fs->getThreeD();
			else if (m_renderMode == bbe::RenderMode::FORWARD_NO_LIGHTS) fs3d = &fs->getThreeDForwardNoLight();
			else bbe::Crash(bbe::Error::IllegalState);
		}
		else
		{
			fs3d = &fs->getThreeDBake();
		}

		program = fs3d->program;
		modelPos = fs3d->modelPos;
	}
	else
	{
		if (!baking)
		{
			if (m_renderMode == bbe::RenderMode::DEFERRED)
			{
				program = m_program3dMrt.program;
				modelPos = m_program3dMrt.modelPos3dMrt;
				albedoTex = m_program3dMrt.albedoTexMrt;
				normalsTex = m_program3dMrt.normalsTexMrt;
				emissionsTex = m_program3dMrt.emissionsTexMrt;
			}
			else if (m_renderMode == bbe::RenderMode::FORWARD_NO_LIGHTS)
			{
				program      = m_program3dForwardNoLight.program;
				modelPos     = m_program3dForwardNoLight.modelPos3dMrt;
				albedoTex    = m_program3dForwardNoLight.albedoTexMrt;
				normalsTex   = m_program3dForwardNoLight.normalsTexMrt;
				emissionsTex = m_program3dForwardNoLight.emissionsTexMrt;
			}
		}
		else
		{
			program = m_program3dMrtBaking.program;
			modelPos = m_program3dMrtBaking.modelPos3dMrt;
			albedoTex = m_program3dMrtBaking.albedoTexMrt;
			normalsTex = m_program3dMrtBaking.normalsTexMrt;
			emissionsTex = m_program3dMrtBaking.emissionsTexMrt;
		}
	}

	glUseProgram(program);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glUniformMatrix4fv(modelPos, 1, GL_FALSE, modelMatrix);
	if (fs)
	{
		if (baking)
		{
			bbe::Matrix4 identiy;
			glUniformMatrix4fv(fs->getThreeDBake().projectionPos, 1, GL_FALSE, &identiy[0]);
			glUniformMatrix4fv(fs->getThreeDBake().viewPos, 1, GL_FALSE, &identiy[0]);
			glUniform4f(fs->getThreeDBake().color3DPos, bakingColor.r, bakingColor.g, bakingColor.b, bakingColor.a);
		}
		else
		{
			glUniformMatrix4fv(fs3d->projectionPos, 1, GL_FALSE, &m_projection[0]);
			glUniformMatrix4fv(fs3d->viewPos, 1, GL_FALSE, &m_view[0]);
			glUniform4f(fs3d->color3DPos, m_color3d.r, m_color3d.g, m_color3d.b, m_color3d.a);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLint positionAttribute = glGetAttribLocation(program, "inPos");
	if (positionAttribute != -1)
	{
		glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(positionAttribute);
		glVertexAttribDivisor(positionAttribute, 0);
	}

	GLint normalPosition = glGetAttribLocation(program, "inNormal");
	if (normalPosition != -1)
	{
		glVertexAttribPointer(normalPosition, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(normalPosition);
		glVertexAttribDivisor(normalPosition, 0);
	}

	GLint uvPosition = glGetAttribLocation(program, "inUvCoord");
	if (uvPosition != -1)
	{
		glVertexAttribPointer(uvPosition, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(uvPosition);
		glVertexAttribDivisor(uvPosition, 0);
	}

	{
		if (!albedo) albedo = &bbe::Image::white();
		if (!normals) normals = &bbe::Image::black();
		if (!emissions) emissions = &bbe::Image::black();

		glUniform1i(albedoTex, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, toRendererData(*albedo)->tex);

		glUniform1i(normalsTex, 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, toRendererData(*normals)->tex);

		glUniform1i(emissionsTex, 2);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, toRendererData(*emissions)->tex);
	}

#ifdef __APPLE__
	glBindVertexArray(vao);
#else
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

	glDrawElements(GL_TRIANGLES, (GLsizei)amountOfIndices, GL_UNSIGNED_INT, 0); addDrawcallStat();
}

void bbe::INTERNAL::openGl::OpenGLManager::addInstancedData2D(PreviousDrawCall2D type, float x, float y, float width, float height, float rotation)
{
	if (type != previousDrawCall2d)
	{
		flushInstanceData2D();
		previousDrawCall2d = type;
	}
	InstanceData2D instanceData;
	instanceData.scalePosOffset.x = width;
	instanceData.scalePosOffset.y = height;
	instanceData.scalePosOffset.z = x;
	instanceData.scalePosOffset.w = y;
	instanceData.rotation = rotation;
	instanceData.color.x = m_color2d.r;
	instanceData.color.y = m_color2d.g;
	instanceData.color.z = m_color2d.b;
	instanceData.color.w = m_color2d.a;
	instanceDatas.add(instanceData);
}

void bbe::INTERNAL::openGl::OpenGLManager::flushInstanceData2D()
{
	if (instanceDatas.getLength() == 0) return;

	GLuint vbo = 0;
	GLuint ibo = 0;
	GLsizei size = 0;
	GLenum mode = 0;
	if (previousDrawCall2d == PreviousDrawCall2D::RECT)
	{
		vbo = OpenGLRectangle::getVbo();
		ibo = OpenGLRectangle::getIbo();
		size = (GLsizei)OpenGLRectangle::getAmountOfIndices();
		mode = GL_TRIANGLE_STRIP;
	}
	else if (previousDrawCall2d == PreviousDrawCall2D::CIRCLE)
	{
		vbo = OpenGLCircle::getVbo();
		ibo = OpenGLCircle::getIbo();
		size = (GLsizei)OpenGLCircle::getAmountOfIndices();
		mode = GL_TRIANGLE_FAN;
	}
	else
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

#ifdef __APPLE__
	// Generate a VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	GLuint program = m_program2d.program;
	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLint positionAttribute = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint instanceVBO = genBuffer("FlushInstanceVBO", BufferTarget::ARRAY_BUFFER, sizeof(InstanceData2D) * instanceDatas.getLength(), instanceDatas.getRaw());

	GLint pos = 1;
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData2D), (const void*)(0 * sizeof(float)));
	glVertexAttribDivisor(pos, 1);
	pos = 2;
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData2D), (const void*)(4 * sizeof(float)));
	glVertexAttribDivisor(pos, 1);
	pos = 3;
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData2D), (const void*)(5 * sizeof(float)));
	glVertexAttribDivisor(pos, 1);

#ifdef __APPLE__
	glBindVertexArray(vao);
#endif
	glDrawElementsInstanced(mode, size, GL_UNSIGNED_INT, 0, (GLsizei)instanceDatas.getLength()); addDrawcallStat();
	instanceDatas.clear();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &instanceVBO);
}

bbe::INTERNAL::openGl::OpenGLImage* bbe::INTERNAL::openGl::OpenGLManager::toRendererData(const bbe::Image& image) const
{
	if (image.m_prendererData == nullptr)
	{
		return new bbe::INTERNAL::openGl::OpenGLImage(image);
	}
	else
	{
		return (bbe::INTERNAL::openGl::OpenGLImage*)image.m_prendererData.get();
	}
}

void bbe::INTERNAL::openGl::OpenGLManager::drawLight(const bbe::PointLight& light, bool baking, GLuint ibo)
{
#ifdef __APPLE__
	// Create and bind VAO for macOS Core Profile compatibility
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	if (!baking)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, openGl::OpenGLSphere::getIbo());
		glBindBuffer(GL_ARRAY_BUFFER, openGl::OpenGLSphere::getVbo());
		GLint positionAttribute = glGetAttribLocation(m_program3dLight.program, "inPos");
		glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(positionAttribute);
		glVertexAttribDivisor(positionAttribute, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	}

	LightProgram& program = baking ? m_program3dLightBaking : m_program3dLight;
	program.setLightUniform(light, baking ? bbe::Matrix4() : m_view);

#ifdef __APPLE__
	glBindVertexArray(vao);
#endif
	glDrawElements(GL_TRIANGLES, baking ? 6 : (GLsizei)openGl::OpenGLSphere::getAmountOfIndices(), GL_UNSIGNED_INT, 0); addDrawcallStat();
}

static bbe::Vector2i getPos(size_t i, uint32_t width)
{
	return bbe::Vector2i((int32_t)(i % width), (int32_t)(i / width));
}

static size_t getIndex(const bbe::Vector2i& pos, uint32_t width)
{
	return pos.y * width + pos.x;
}

static void transferBorderPixels(bbe::List<bbe::byte>& byteBuffer, const bbe::List<float>& colorFloatBuffer, size_t i, const bbe::Vector2i& src, int32_t width, int32_t height)
{
	if (src.x < 0 || src.y < 0 || src.x >= width || src.y >= height) return;
	const size_t srcIndex = getIndex(src, width) * 4;
	if (   colorFloatBuffer[srcIndex + 0] != 0
		|| colorFloatBuffer[srcIndex + 1] != 0
		|| colorFloatBuffer[srcIndex + 2] != 0)
	{
		byteBuffer[i + 0] = byteBuffer[srcIndex + 0];
		byteBuffer[i + 1] = byteBuffer[srcIndex + 1];
		byteBuffer[i + 2] = byteBuffer[srcIndex + 2];
	}
}

bbe::Image bbe::INTERNAL::openGl::OpenGLManager::framebufferToImage(uint32_t width, uint32_t height) const
{
	const size_t bufferSize = width * height * 4/*channels*/;
	bbe::List<float> colorFloatBuffer;
	bbe::List<byte> byteBuffer;
	colorFloatBuffer.resizeCapacityAndLengthUninit(bufferSize);
	byteBuffer      .resizeCapacityAndLengthUninit(bufferSize);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, colorFloatBuffer.getRaw());
	for (size_t i = 0; i < colorFloatBuffer.getLength(); i += 4)
	{
		byteBuffer[i + 0] = (bbe::byte)(bbe::Math::pow(bbe::Math::clamp01(colorFloatBuffer[i + 0]), 1.0 / 2.2) * 255);
		byteBuffer[i + 1] = (bbe::byte)(bbe::Math::pow(bbe::Math::clamp01(colorFloatBuffer[i + 1]), 1.0 / 2.2) * 255);
		byteBuffer[i + 2] = (bbe::byte)(bbe::Math::pow(bbe::Math::clamp01(colorFloatBuffer[i + 2]), 1.0 / 2.2) * 255);
		byteBuffer[i + 3] = 255;
	}

	// TODO this should be put elsewhere. It isn't really the job of this function.
	for (size_t i = 0; i < byteBuffer.getLength(); i += 4)
	{
		if (   colorFloatBuffer[i + 0] == 0
			&& colorFloatBuffer[i + 1] == 0
			&& colorFloatBuffer[i + 2] == 0)
		{
			const bbe::Vector2i pos = getPos(i / 4, width);
			transferBorderPixels(byteBuffer, colorFloatBuffer, i, pos + bbe::Vector2i( 1,  0), (int32_t)width, (int32_t)height);
			transferBorderPixels(byteBuffer, colorFloatBuffer, i, pos + bbe::Vector2i(-1,  0), (int32_t)width, (int32_t)height);
			transferBorderPixels(byteBuffer, colorFloatBuffer, i, pos + bbe::Vector2i( 0,  1), (int32_t)width, (int32_t)height);
			transferBorderPixels(byteBuffer, colorFloatBuffer, i, pos + bbe::Vector2i( 0, -1), (int32_t)width, (int32_t)height);
		}
	}
	return bbe::Image(width, height, byteBuffer.getRaw(), bbe::ImageFormat::R8G8B8A8);
}

uint32_t bbe::INTERNAL::openGl::OpenGLManager::getAmountOfDrawcalls()
{
	return *amountOfDrawcallsRead;
}

void bbe::INTERNAL::openGl::OpenGLManager::addDrawcallStat()
{
	(*amountOfDrawcallsWrite)++;
}

void bbe::INTERNAL::openGl::OpenGLManager::flipDrawcallStats()
{
	uint32_t* temp = amountOfDrawcallsRead;
	amountOfDrawcallsRead = amountOfDrawcallsWrite;
	amountOfDrawcallsWrite = temp;
	*amountOfDrawcallsWrite = 0;
}

GLuint bbe::INTERNAL::openGl::OpenGLManager::getModeFramebuffer()
{
	switch (m_renderMode)
	{
	case(bbe::RenderMode::DEFERRED):          return mrtFb           .framebuffer;
	case(bbe::RenderMode::FORWARD_NO_LIGHTS): return forwardNoLightFb.framebuffer;
	default: bbe::Crash(bbe::Error::IllegalState);
	}
}

bbe::INTERNAL::openGl::OpenGLManager::OpenGLManager()
{
}

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	// Filter too spammy types.
	if (type == GL_DEBUG_TYPE_OTHER) return;
	if (type == GL_DEBUG_TYPE_PERFORMANCE) return;

	bbe::String typeString;
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               typeString = "ERROR";               break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "UNDEFINED_BEHAVIOR";  break;
	case GL_DEBUG_TYPE_PORTABILITY:         typeString = "PORTABILITY";         break;
	case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "PERFORMANCE";         break;
	case GL_DEBUG_TYPE_OTHER:               typeString = "OTHER";               break;
	case GL_DEBUG_TYPE_MARKER:              typeString = "MARKER";              break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          typeString = "PUSH_GROUP";          break;
	case GL_DEBUG_TYPE_POP_GROUP:           typeString = "POP_GROUP";           break;
	default:                                typeString = "UNKNOWN";             break;
	}

	BBELOGLN("OpenGL " << typeString << " Callback: "
		"\n   type     = " << type <<
		"\n   severity = " << severity <<
		"\n   message  = " << message);
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

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, nullptr);
#endif

	m_program2d = init2dShaders();
	m_program2dTex = init2dTexShaders();
	m_program3dMrt = init3dShadersMrt(false);
	m_program3dForwardNoLight = init3dForwardNoLight();
	m_program3dAmbient = init3dShadersAmbient();
	m_programPostProcessing = init3dPostProcessing();
	m_programBakingGammaCorrection = initBakingGammaCorrection();
	m_program3dLight = init3dShadersLight(false);

	m_program3dMrtBaking = init3dShadersMrt(true);
	m_program3dLightBaking = init3dShadersLight(true);
	initFrameBuffers();

	OpenGLRectangle::init();
	OpenGLCircle::init();
	OpenGLCube::init();
	OpenGLSphere::init();

	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
	imguiStart();

	const uint32_t indices[] = { 0, 3, 1, 1, 3, 2 };
	quadIbo = genBuffer("quadIbo", BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices);
}

void bbe::INTERNAL::openGl::OpenGLManager::destroy()
{
	glDeleteBuffers(1, &quadIbo);
	imguiStop();

	OpenGLSphere   ::destroy();
	OpenGLCube     ::destroy();
	OpenGLCircle   ::destroy();
	OpenGLRectangle::destroy();

	mrtFb                         .destroy();
	forwardNoLightFb              .destroy();
	postProcessingFb              .destroy();
	m_program3dMrt                .destroy();
	m_programPostProcessing       .destroy();
	m_programBakingGammaCorrection.destroy();
	m_program3dLight              .destroy();
	m_program3dMrtBaking          .destroy();
	m_program3dLightBaking        .destroy();
	glDeleteBuffers(1, &m_imageUvBuffer);
	m_program2dTex.destroy();
	m_program2d   .destroy();
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw2D()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	// Draw the stuff of 3D
	m_program3dAmbient.use();
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFb.framebuffer);
	postProcessingFb.clearTextures();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, forwardNoLightFb.framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcessingFb.framebuffer);
	glBlitFramebuffer(0, 0, m_windowWidth, m_windowHeight, 0, 0, m_windowWidth, m_windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFb.framebuffer);

	mrtFb.useAsInput();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
#ifdef __APPLE__
	// Create and bind VAO for macOS Core Profile compatibility
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLRectangle::getIbo());
#ifdef __APPLE__
	glBindVertexArray(vao);
#endif
	glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)OpenGLRectangle::getAmountOfIndices(), GL_UNSIGNED_INT, 0); addDrawcallStat();

	m_program3dLight.use();
	mrtFb.useAsInput();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	for (size_t i = 0; i < pointLights.getLength(); i++)
	{
		const bbe::PointLight& l = pointLights[i];
		drawLight(l, false);
	}

#ifdef __APPLE__
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	m_programPostProcessing.use();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	postProcessingFb.useAsInput();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIbo);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

#ifdef __APPLE__
	glBindVertexArray(vao);
#endif
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); addDrawcallStat();

	// Switch to 2D
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_primitiveBrush2D.INTERNAL_beginDraw(m_pwindow, m_windowWidth, m_windowHeight, this);

	instanceDatas.clear();

	previousDrawCall2d = PreviousDrawCall2D::NONE;
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw3D()
{
	glEnable(GL_DEPTH_TEST);
	m_primitiveBrush3D.INTERNAL_beginDraw(m_windowWidth, m_windowHeight, this);
	glBindFramebuffer(GL_FRAMEBUFFER, forwardNoLightFb.framebuffer);
	forwardNoLightFb.clearTextures();
	m_program3dMrt.use();
	glBindFramebuffer(GL_FRAMEBUFFER, mrtFb.framebuffer);
	mrtFb.clearTextures();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
}

void bbe::INTERNAL::openGl::OpenGLManager::preDraw()
{
	flipDrawcallStats();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imguiStartFrame();
	pointLights.clear();
	m_color2d = bbe::Color(1, 1, 1, 1);
	m_color3d = bbe::Color(1, 1, 1, 1);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glScissor (0, 0, m_windowWidth, m_windowHeight);
}

void bbe::INTERNAL::openGl::OpenGLManager::postDraw()
{
	flushInstanceData2D();
	imguiEndFrame();
	glfwSwapBuffers(m_pwindow);
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
	m_program3dAmbient.uniform2f(screenSizeAmbient, (float)width, (float)height);
	m_program3dLight.uniform2f(m_program3dLight.screenSize3dLight, (float)width, (float)height);
	m_programPostProcessing.uniform2f(screenSizePostProcessing, (float)width, (float)height);

	m_windowWidth = width;
	m_windowHeight = height;

	initFrameBuffers();
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
	m_color2d = color;
}

void bbe::INTERNAL::openGl::OpenGLManager::fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader)
{
	if (!shader)
	{
		addInstancedData2D(PreviousDrawCall2D::RECT, rect.x, rect.y, rect.width, rect.height, rotation);
		return;
	}

	flushInstanceData2D();
	bbe::INTERNAL::openGl::OpenGLFragmentShader* fs = nullptr;
	if (shader->m_prendererData != nullptr)
	{
		fs = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)shader->m_prendererData.get();
	}
	else
	{
		fs = new bbe::INTERNAL::openGl::OpenGLFragmentShader(*shader);
	}
	GLuint program = fs->getTwoD().program;

	GLint scalePosOffsetPos = fs->getTwoD().scalePosOffsetPos;
	GLint rotationPos = fs->getTwoD().rotationPos;

	glUseProgram(program);

#ifdef __APPLE__
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	if (previousDrawCall2d != PreviousDrawCall2D::RECT_SHADER)
	{
		previousDrawCall2d = PreviousDrawCall2D::RECT_SHADER;
		glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGLRectangle::getIbo());

		GLint positionAttribute = glGetAttribLocation(program, "position");
		glEnableVertexAttribArray(positionAttribute);

		glBindBuffer(GL_ARRAY_BUFFER, OpenGLRectangle::getVbo());
		glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(positionAttribute, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

#ifdef __APPLE__
	glBindVertexArray(vao);
#endif

	glUniform2f(fs->getTwoD().screenSizePos, (float)m_windowWidth, (float)m_windowHeight);
	glUniform4f(scalePosOffsetPos, rect.width, rect.height, rect.x, rect.y);
	glUniform1f(rotationPos, rotation);
	glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)OpenGLRectangle::getAmountOfIndices(), GL_UNSIGNED_INT, 0); addDrawcallStat();
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCircle2D(const Circle& circle)
{
	addInstancedData2D(PreviousDrawCall2D::CIRCLE, circle.getX(), circle.getY(), circle.getWidth(), circle.getHeight(), 0);
}

void bbe::INTERNAL::openGl::OpenGLManager::drawImage2D(const Rectangle& rect, const Image& image, float rotation)
{
	// TODO make proper implementation
	flushInstanceData2D();
	m_program2dTex.use();
	m_program2dTex.uniform4f(inColorPos2dTex, m_color2d);
	previousDrawCall2d = PreviousDrawCall2D::IMAGE;
	bbe::List<bbe::Vector2> vertices;
	rect.getVertices(vertices);
	for (bbe::Vector2& v : vertices)
	{
		v = v.rotate(rotation, rect.getCenter());
	}

	if (image.m_format == ImageFormat::R8)
	{
		m_program2dTex.uniform1i(swizzleModePos, 1);
	}
	else
	{
		m_program2dTex.uniform1i(swizzleModePos, 0);
	}

#ifdef __APPLE__
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	GLuint vbo = genBuffer("drawImageVBO", BufferTarget::ARRAY_BUFFER, sizeof(bbe::Vector2) * vertices.getLength(), vertices.getRaw());
	constexpr uint32_t indices[] = { 0, 1, 3, 1, 2, 3 };
	static GLuint ibo = genBuffer("drawImageIBO", BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices);
	glBindBuffer((GLenum)BufferTarget::ELEMENT_ARRAY_BUFFER, ibo);
	//TODO: The ibo is never freed - is that actually bad or does OpenGL clean it up when we remove the context?

	glUniform2f(scalePos2dTex, 1.0f, 1.0f);

	GLint positionAttribute = glGetAttribLocation(m_program2dTex.program, "position");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLint uvPosition = glGetAttribLocation(m_program2dTex.program, "uv");
	glEnableVertexAttribArray(uvPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_imageUvBuffer);
	glVertexAttribPointer(uvPosition, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glVertexAttribDivisor(uvPosition, 0);

	glUniform1i(texPos2dTex, 0);
	glActiveTexture(GL_TEXTURE0);
	bbe::INTERNAL::openGl::OpenGLImage* ogi = toRendererData(image);
	glBindTexture(GL_TEXTURE_2D, ogi->tex);

#ifdef __APPLE__
	glBindVertexArray(vao);
#endif
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); addDrawcallStat();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);
}

void bbe::INTERNAL::openGl::OpenGLManager::fillVertexIndexList2D(const uint32_t* indices, size_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& p, const bbe::Vector2& scale)
{
	flushInstanceData2D();
	m_program2d.use();

	previousDrawCall2d = PreviousDrawCall2D::VERTEX_INDEX_LIST;
	GLuint vbo = genBuffer("fillVertexIndexList2DVBO", BufferTarget::ARRAY_BUFFER, sizeof(bbe::Vector2) * amountOfVertices, vertices);
	GLuint ibo = genBuffer("fillVertexIndexList2DIBO", BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * amountOfIndices, indices);

	InstanceData2D instanceData2D;
	instanceData2D.scalePosOffset.x = scale.x;
	instanceData2D.scalePosOffset.y = scale.y;
	instanceData2D.scalePosOffset.z = p.x;
	instanceData2D.scalePosOffset.w = p.y;
	instanceData2D.rotation = 0.f;
	instanceData2D.color.x = m_color2d.r;
	instanceData2D.color.y = m_color2d.g;
	instanceData2D.color.z = m_color2d.b;
	instanceData2D.color.w = m_color2d.a;

	GLuint instanceVBO = genBuffer("fillVertexIndexList2DInstanceVBO", BufferTarget::ARRAY_BUFFER, sizeof(InstanceData2D), &instanceData2D);

#ifdef __APPLE__
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif

	GLint pos = 1;
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData2D), (const void*)(0 * sizeof(float)));
	glVertexAttribDivisor(pos, 1);
	pos = 2;
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData2D), (const void*)(4 * sizeof(float)));
	glVertexAttribDivisor(pos, 1);
	pos = 3;
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData2D), (const void*)(5 * sizeof(float)));
	glVertexAttribDivisor(pos, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint positionAttribute = glGetAttribLocation(m_program2d.program, "position");
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(positionAttribute, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef __APPLE__
	glBindVertexArray(vao);
#endif

	glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)amountOfIndices, GL_UNSIGNED_INT, 0, 1); addDrawcallStat();

#ifdef __APPLE__
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
#endif

	glDeleteBuffers(1, &instanceVBO);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
}

void bbe::INTERNAL::openGl::OpenGLManager::setColor3D(const bbe::Color& color)
{
	bbe::Color copy = color;
	copy.a = 1.f;
	m_program3dMrt.uniform4f(m_program3dMrt.inColorPos3dMrt, copy);
	m_program3dForwardNoLight.uniform4f(m_program3dForwardNoLight.inColorPos3dMrt, copy);
	m_color3d = copy;
}

void bbe::INTERNAL::openGl::OpenGLManager::setCamera3D(const Vector3& cameraPos, const bbe::Matrix4& view, const bbe::Matrix4& projection)
{
	m_program3dMrt.uniformMatrix4fv(m_program3dMrt.viewPos3dMrt, GL_FALSE, view);
	m_program3dMrt.uniformMatrix4fv(m_program3dMrt.projectionPos3dMrt, GL_FALSE, projection);
	m_program3dForwardNoLight.uniformMatrix4fv(m_program3dForwardNoLight.viewPos3dMrt, GL_FALSE, view);
	m_program3dForwardNoLight.uniformMatrix4fv(m_program3dForwardNoLight.projectionPos3dMrt, GL_FALSE, projection);
	m_program3dLight.uniformMatrix4fv(m_program3dLight.projectionPos3dLight, GL_FALSE, projection);
	m_view = view;
	m_projection = projection;
	m_cameraPos = cameraPos;
}

void bbe::INTERNAL::openGl::OpenGLManager::fillCube3D(const Cube& cube)
{
	fillInternalMesh(&cube.getTransform()[0], OpenGLCube::getIbo(), OpenGLCube::getVbo(), OpenGLCube::getAmountOfIndices(), nullptr, nullptr, nullptr, nullptr, getModeFramebuffer(), false, bbe::Color::white());
}

void bbe::INTERNAL::openGl::OpenGLManager::fillSphere3D(const IcoSphere& sphere)
{
	fillInternalMesh(&sphere.getTransform()[0], OpenGLSphere::getIbo(), OpenGLSphere::getVbo(), OpenGLSphere::getAmountOfIndices(), nullptr, nullptr, nullptr, nullptr, getModeFramebuffer(), false, bbe::Color::white());
}

void bbe::INTERNAL::openGl::OpenGLManager::fillModel(const bbe::Matrix4& transform, const Model& model, const Image* albedo, const Image* normals, const Image* emissions, const FragmentShader* shader)
{
	fillModel(transform, model, albedo, normals, emissions, shader, getModeFramebuffer(), false, bbe::Color::white());
}

struct OcclusionQuery : public bbe::DataProvider<bool>
{
	GLuint id = 0;
	OcclusionQuery(bbe::INTERNAL::openGl::OpenGLManager* manager, const bbe::Cube& cube)
	{
		glGenQueries(1, &id);
		glBeginQuery(GL_ANY_SAMPLES_PASSED, id);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilMask(GL_FALSE);
		manager->fillCube3D(cube);
		glStencilMask(GL_TRUE);
		glDepthMask(GL_TRUE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEndQuery(GL_ANY_SAMPLES_PASSED);
	}

	~OcclusionQuery() override
	{
		glDeleteQueries(1, &id);
	}

	virtual bool isValueReady() const override
	{
		GLuint val;
		glGetQueryObjectuiv(id, GL_QUERY_RESULT_AVAILABLE, &val);
		return val;
	}
	virtual bool getValue() const override
	{
		GLuint val;
		glGetQueryObjectuiv(id, GL_QUERY_RESULT, &val);
		return val;
	}
};

bbe::Future<bool> bbe::INTERNAL::openGl::OpenGLManager::isCubeVisible(const Cube& cube)
{
	return bbe::Future<bool>(new OcclusionQuery(this, cube));
}

void bbe::INTERNAL::openGl::OpenGLManager::setRenderMode(bbe::RenderMode renderMode)
{
	m_renderMode = renderMode;
}

void bbe::INTERNAL::openGl::OpenGLManager::addLight(const bbe::Vector3& pos, float lightStrength, const bbe::Color& lightColor, const bbe::Color& specularColor, LightFalloffMode falloffMode)
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
	ImPlot::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_pwindow, false);

	m_imguiInitSuccessful = ImGui_ImplOpenGL3_Init();
	if (!m_imguiInitSuccessful)
	{
		bbe::Crash(bbe::Error::IllegalState);
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
		ImPlot::DestroyContext();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
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

void bbe::INTERNAL::openGl::OpenGLManager::bakeLightMrt(bbe::LightBaker& lightBaker)
{
	if (lightBaker.m_prendererData != nullptr) bbe::Crash(bbe::Error::IllegalState);

	OpenGLLightBaker* ogllb = new OpenGLLightBaker();
	lightBaker.m_prendererData = ogllb;

	ogllb->geometryBuffer = getGeometryBuffer("BakeLightsGeometryBuffer", lightBaker.m_resolution.x, lightBaker.m_resolution.y, true);
	ogllb->colorBuffer = Framebuffer(lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	ogllb->colorBuffer.addTexture("BakeLights ColorBuffer");
	ogllb->colorBuffer.finalize("BakeLightsColorBuffer");
	ogllb->colorBufferGamma = Framebuffer(lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	ogllb->colorBufferGamma.addTexture("BakeLightsGamma ColorBuffer", 1);
	ogllb->colorBufferGamma.finalize("BakeLightsGammaColorBuffer");

	glViewport(0, 0, lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	glScissor(0, 0, lightBaker.m_resolution.x, lightBaker.m_resolution.y);

	// Pre draw
	m_program3dMrtBaking.use();
	glBindFramebuffer(GL_FRAMEBUFFER, ogllb->geometryBuffer.framebuffer);
	ogllb->geometryBuffer.clearTextures();

	// MRT Pass
	m_program3dMrtBaking.use();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	m_program3dMrtBaking.uniformMatrix4fv(m_program3dMrtBaking.modelPos3dMrt, false, lightBaker.m_transform);
	fillModel(lightBaker.m_transform, lightBaker.m_model.toBakingModel(), nullptr, lightBaker.m_pnormals, nullptr, lightBaker.m_pfragementShader, ogllb->geometryBuffer.framebuffer, true, bbe::Color::white());

	glBindFramebuffer(GL_FRAMEBUFFER, ogllb->colorBuffer.framebuffer);
	ogllb->colorBuffer.clearTextures();

	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glScissor(0, 0, m_windowWidth, m_windowHeight);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void bbe::INTERNAL::openGl::OpenGLManager::bakeLight(bbe::LightBaker& lightBaker, const bbe::PointLight& light)
{
	if (lightBaker.m_prendererData == nullptr) bbe::Crash(bbe::Error::IllegalState);

	OpenGLLightBaker* ogllb = (OpenGLLightBaker*)lightBaker.m_prendererData.get();

	glViewport(0, 0, lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	glScissor(0, 0, lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, ogllb->colorBuffer.framebuffer);

	// Light Passes
	m_program3dLightBaking.use();
	m_program3dLightBaking.uniform2f(m_program3dLightBaking.screenSize3dLight, (GLfloat)lightBaker.m_resolution.x, (GLfloat)lightBaker.m_resolution.y);

	ogllb->geometryBuffer.useAsInput();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	PointLight copy = light;
	copy.pos -= lightBaker.m_lightOffset;
	drawLight(copy, true, quadIbo);
	lightBaker.m_amountOfBakedLights++;

	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glScissor(0, 0, m_windowWidth, m_windowHeight);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void bbe::INTERNAL::openGl::OpenGLManager::bakeLightGammaCorrect(bbe::LightBaker& lightBaker)
{
	if (lightBaker.m_prendererData == nullptr) bbe::Crash(bbe::Error::IllegalState);

	OpenGLLightBaker* ogllb = (OpenGLLightBaker*)lightBaker.m_prendererData.get();

	glViewport(0, 0, lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	glScissor(0, 0, lightBaker.m_resolution.x, lightBaker.m_resolution.y);
	// Gamma Correction Step
	glBindFramebuffer(GL_FRAMEBUFFER, ogllb->colorBufferGamma.framebuffer);
	m_programBakingGammaCorrection.use();
	m_programBakingGammaCorrection.uniform2f(screenSizeBakingGammaCorrection, (GLfloat)lightBaker.m_resolution.x, (GLfloat)lightBaker.m_resolution.y);
	ogllb->colorBuffer.useAsInput();

#ifdef __APPLE__
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIbo);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
#ifdef __APPLE__
	glBindVertexArray(vao);
#endif
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); addDrawcallStat();

	// Read the frambuffer to image
	glBindTexture(GL_TEXTURE_2D, ogllb->colorBufferGamma.textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glScissor(0, 0, m_windowWidth, m_windowHeight);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bbe::Image bbe::INTERNAL::openGl::OpenGLManager::bakeLightDetach(bbe::LightBaker& lightBaker)
{
	if (lightBaker.m_prendererData == nullptr) bbe::Crash(bbe::Error::IllegalState);

	OpenGLLightBaker* ogllb = (OpenGLLightBaker*)lightBaker.m_prendererData.get();
	bbe::Image image;
	image.m_format = bbe::ImageFormat::R8G8B8A8;
	new OpenGLImage(image, ogllb->colorBufferGamma.textures[0]);
	ogllb->colorBufferGamma.textures.clear();

	return image;
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
		bbe::Crash(bbe::Error::IllegalState);
	}

	retVal += name;
	retVal += ";";

	return retVal;
}

void bbe::INTERNAL::openGl::LightProgram::setLightUniform(const bbe::PointLight& light, const bbe::Matrix4& view)
{
	bbe::Vector4 p(light.pos, 1.0f);
	p = view * p;
	uniform4f(lightPosPos3dLight, p.x, p.y, p.z, light.lightStrength);
	uniform1i(falloffModePos3dLight, (int)light.falloffMode);
	uniform4f(lightColorPos3dLight, light.lightColor);
	uniform4f(specularColorPos3dLight, light.specularColor);
	uniform1f(lightRadiusPos, light.getLightRadius());
}
