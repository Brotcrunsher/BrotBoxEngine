#include "BBE/OpenGL/OpenGLFragmentShader.h"
#include "BBE/FatalErrors.h"

bbe::INTERNAL::openGl::OpenGLFragmentShader::OpenGLFragmentShader(const bbe::FragmentShader& shader)
{
	if (shader.m_prendererData != nullptr)
	{
		throw IllegalStateException();
	}
	shader.m_prendererData = this;

	char const* vertexShaderSrc =
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

	char const* fragmentShaderSource = (char const*)shader.m_rawData.getRaw();
	vertex = getShader(GL_VERTEX_SHADER, vertexShaderSrc);
	fragment = getShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

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

	screenSizePos     = glGetUniformLocation(program, "screenSize");
	scalePosOffsetPos = glGetUniformLocation(program, "scalePosOffset");
	rotationPos       = glGetUniformLocation(program, "rotation");
}

GLuint bbe::INTERNAL::openGl::OpenGLFragmentShader::getShader(GLenum shaderType, const bbe::String& src)
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

bbe::INTERNAL::openGl::OpenGLFragmentShader::~OpenGLFragmentShader()
{
	glDeleteProgram(program);
	glDeleteShader(fragment);
	glDeleteShader(vertex);
}
