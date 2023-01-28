#pragma once

#include "BBE/glfwWrapper.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			namespace OpenGLRectangle
			{
				void init();
				void destroy();

				GLuint getVbo();
				GLuint getIbo();

				size_t getAmountOfIndices();
			}
		}
	}
}
