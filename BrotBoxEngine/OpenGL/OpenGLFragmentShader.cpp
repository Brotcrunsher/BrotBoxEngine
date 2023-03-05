#include "BBE/OpenGL/OpenGLFragmentShader.h"
#include "BBE/FatalErrors.h"

static GLuint getShader(GLenum shaderType, const bbe::String& src, bbe::String& errorLog)
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
		errorLog += log.getRaw();
		errorLog += "\n";
	}
	return shader;
}

static void build(GLuint& vertex, GLuint& fragment, GLuint& program, const char* vertexShaderSource, const char* fragmentShaderSource, bbe::String& errorLog)
{
	vertex = getShader(GL_VERTEX_SHADER, vertexShaderSource, errorLog);
	fragment = getShader(GL_FRAGMENT_SHADER, fragmentShaderSource, errorLog);

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
	}
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::OpenGLFragmentShader(const bbe::FragmentShader& shader)
{
	if (shader.m_prendererData != nullptr)
	{
		throw IllegalStateException();
	}
	shader.m_prendererData = this;

	bbe::List<char> chars;
	chars.resizeCapacityAndLengthUninit(shader.m_rawData.getLength() + 1);
	memcpy(chars.getRaw(), shader.m_rawData.getRaw(), shader.m_rawData.getLength());
	chars.last() = '\0';
	char const* fragmentShaderSource = chars.getRaw();

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
		"   passUvCoord = vec2(inUvCoord.x, 1.0 - inUvCoord.y);" //TODO: wtf? Where does that y flip come from?
		"}";

	build(vertex2d, fragment2d, program2d, vertexShader2dSource, fragmentShaderSource, errorLog2d);
	build(vertex3d, fragment3d, program3d, vertexShader3dSource, fragmentShaderSource, errorLog3d);

	screenSizePos     = glGetUniformLocation(program2d, "screenSize");
	scalePosOffsetPos = glGetUniformLocation(program2d, "scalePosOffset");
	rotationPos       = glGetUniformLocation(program2d, "rotation");

	viewPos       = glGetUniformLocation(program3d, "view");
	projectionPos = glGetUniformLocation(program3d, "projection");
	modelPos      = glGetUniformLocation(program3d, "model");
	color3DPos    = glGetUniformLocation(program3d, "inColor");
}

bbe::INTERNAL::openGl::OpenGLFragmentShader::~OpenGLFragmentShader()
{
	glDeleteProgram(program2d);
	glDeleteShader(fragment2d);
	glDeleteShader(vertex2d);
	glDeleteProgram(program3d);
	glDeleteShader(fragment3d);
	glDeleteShader(vertex3d);
}
