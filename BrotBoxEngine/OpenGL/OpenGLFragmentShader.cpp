#include "BBE/OpenGL/OpenGLFragmentShader.h"
#include "BBE/FatalErrors.h"

static GLuint getShader(GLenum shaderType, const char* src, bbe::String& errorLog)
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
		errorLog += log.getRaw();
		errorLog += "\n";
	}
	return shader;
}

static void build(bbe::INTERNAL::openGl::OpenGLFragmentShader::ShaderProgramTripple& prog, const char* vertexShaderSource, const char* fragmentShaderSource)
{
	if (prog.built) return;
	prog.built = true;

	prog.vertex = getShader(GL_VERTEX_SHADER, vertexShaderSource, prog.errorLog);
	prog.fragment = getShader(GL_FRAGMENT_SHADER, fragmentShaderSource, prog.errorLog);

	prog.program = glCreateProgram();
	glAttachShader(prog.program, prog.vertex);
	glAttachShader(prog.program, prog.fragment);
	glLinkProgram(prog.program);
	GLint success = 0;
	glGetProgramiv(prog.program, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLint length = 0;
		glGetProgramiv(prog.program, GL_INFO_LOG_LENGTH, &length);

		bbe::List<char> log;
		log.resizeCapacityAndLength(length);
		glGetProgramInfoLog(prog.program, length, &length, log.getRaw());
		std::cout << log.getRaw() << std::endl;
	}
	prog.determinePositions();
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::OpenGLFragmentShader(const bbe::FragmentShader& shader)
{
	if (shader.m_prendererData != nullptr)
	{
		throw IllegalStateException();
	}
	shader.m_prendererData = this;

	code.resizeCapacityAndLengthUninit(shader.m_rawData.getLength() + 1);
	memcpy(code.getRaw(), shader.m_rawData.getRaw(), shader.m_rawData.getLength());
	code.last() = '\0';
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::TwoD& bbe::INTERNAL::openGl::OpenGLFragmentShader::getTwoD()
{
	char const* vertexShader2dSource =
		"#version 300 es\n"
		"precision highp float;"
		""
		"uniform vec2 screenSize;"
		"uniform vec4 scalePosOffset;"
		"uniform float rotation;"
		"in vec2 position;"
		"out vec2 passPosition;"
		"void main()"
		"{"
		"   float s = sin(rotation);"
		"   float c = cos(rotation);"
		"   vec2 scaledPos = position * scalePosOffset.xy;"
		"   vec2 firstTranslatedPos = scaledPos - scalePosOffset.xy * 0.5;"
		"   vec2 rotatedPos = vec2(c * firstTranslatedPos.x - s * firstTranslatedPos.y, s * firstTranslatedPos.x + c * firstTranslatedPos.y);"
		"   vec2 secondTranslatedPos = rotatedPos + scalePosOffset.xy * 0.5;"
		"	vec2 pos = (((secondTranslatedPos + scalePosOffset.zw) / screenSize * vec2(2, -2)) + vec2(-1, +1));\n"
		"   passPosition = position;"
		"	gl_Position = vec4(pos, 0.0, 1.0);"
		"}";
	char const* fragmentShaderSource = code.getRaw();
	build(twoD, vertexShader2dSource, fragmentShaderSource);

	return twoD;
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::ThreeD& bbe::INTERNAL::openGl::OpenGLFragmentShader::getThreeD()
{
	char const* vertexShader3dSource =
		"#version 300 es\n"
		"precision highp float;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"uniform mat4 model;"
		"in vec3 inPos;"
		"in vec3 inNormal;"
		"in vec2 inUvCoord;"
		"out vec4 passPos;"
		"out vec4 passWorldPos;"
		"out vec4 passNormal;"
		"out vec2 passUvCoord;"
		"void main()"
		"{"
		"   passWorldPos = model * vec4(inPos, 1.0);"
		"   gl_Position = projection * view * passWorldPos * vec4(1.0, -1.0, 1.0, 1.0);"
		"   passPos = view * passWorldPos;"
		"   passNormal = view * model * vec4(inNormal, 0.0);"
		"   passUvCoord = inUvCoord;"
		"}";
	char const* fragmentShaderSource = code.getRaw();
	build(threeD, vertexShader3dSource, fragmentShaderSource);

	return threeD;
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::ThreeD& bbe::INTERNAL::openGl::OpenGLFragmentShader::getThreeDBake()
{
	char const* vertexShader3dBakeSource =
		"#version 300 es\n"
		"precision highp float;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"uniform mat4 model;"
		"in vec3 inPos;"
		"in vec3 inNormal;"
		"in vec2 inUvCoord;"
		"out vec4 passPos;"
		"out vec4 passWorldPos;"
		"out vec4 passNormal;"
		"out vec2 passUvCoord;"
		"void main()"
		"{"
		"   passWorldPos = model * vec4(inPos, 1.0);"
		"   gl_Position = vec4(inUvCoord * 2.0 - vec2(1, 1), 0.0, 1.0);"
		"   passPos = passWorldPos;"
		"   passNormal = model * vec4(inNormal, 0.0);"
		"   passUvCoord = inUvCoord;"
		"}";

	char const* fragmentShaderSource = code.getRaw();
	build(threeDBake, vertexShader3dBakeSource, fragmentShaderSource);
	return threeDBake;
}

bool bbe::INTERNAL::openGl::OpenGLFragmentShader::hasTwoD() const
{
	return twoD.built;
}

bool bbe::INTERNAL::openGl::OpenGLFragmentShader::hasThreeD() const
{
	return threeD.built;
}

bool bbe::INTERNAL::openGl::OpenGLFragmentShader::hasThreeDBake() const
{
	return threeDBake.built;
}

void bbe::INTERNAL::openGl::OpenGLFragmentShader::ShaderProgramTripple::destroy()
{
	glDeleteProgram(program);
	glDeleteShader(fragment);
	glDeleteShader(vertex);
}

void bbe::INTERNAL::openGl::OpenGLFragmentShader::TwoD::determinePositions()
{
	screenSizePos     = glGetUniformLocation(program, "screenSize");
	scalePosOffsetPos = glGetUniformLocation(program, "scalePosOffset");
	rotationPos       = glGetUniformLocation(program, "rotation");
}

void bbe::INTERNAL::openGl::OpenGLFragmentShader::ThreeD::determinePositions()
{
	viewPos       = glGetUniformLocation(program, "view");
	projectionPos = glGetUniformLocation(program, "projection");
	modelPos      = glGetUniformLocation(program, "model");
	color3DPos    = glGetUniformLocation(program, "inColor");
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::~OpenGLFragmentShader()
{
	twoD      .destroy();
	threeD    .destroy();
	threeDBake.destroy();
}
