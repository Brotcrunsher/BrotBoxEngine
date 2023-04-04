#include "BBE/OpenGL/OpenGLLightBaker.h"

bbe::INTERNAL::openGl::OpenGLLightBaker::~OpenGLLightBaker()
{
	geometryBuffer  .destroy();
	colorBuffer     .destroy();
	colorBufferGamma.destroy();
}
