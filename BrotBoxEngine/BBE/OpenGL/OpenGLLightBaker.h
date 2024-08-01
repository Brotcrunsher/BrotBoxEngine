#pragma once

#include "BBE/glfwWrapper.h"
#include "BBE/AutoRefCountable.h"
#include "BBE/OpenGL/OpenGLManager.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct OpenGLLightBaker : bbe::AutoRefCountable
			{
				Framebuffer geometryBuffer;
				Framebuffer colorBuffer;
				Framebuffer colorBufferGamma;

				virtual ~OpenGLLightBaker() override;
			};
		}
	}
}
