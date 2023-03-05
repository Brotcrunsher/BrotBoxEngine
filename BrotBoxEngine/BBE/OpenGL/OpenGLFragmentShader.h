#pragma once

#include "../BBE/AutoRefCountable.h"
#include "../BBE/FragmentShader.h"
#include "../BBE/glfwWrapper.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			struct OpenGLFragmentShader : public AutoRefCountable
			{
				GLuint vertex2d = 0;
				GLuint fragment2d = 0;
				GLuint program2d = 0;
				GLint screenSizePos = 0;
				GLint scalePosOffsetPos = 0;
				GLint rotationPos = 0;

				GLuint vertex3d = 0;
				GLuint fragment3d = 0;
				GLuint program3d = 0;
				GLint viewPos = 0;
				GLint projectionPos = 0;
				GLint modelPos = 0;
				GLint color3DPos = 0;

				bbe::String errorLog2d;
				bbe::String errorLog3d;

				OpenGLFragmentShader(const bbe::FragmentShader& shader);
				~OpenGLFragmentShader();

				OpenGLFragmentShader(const OpenGLFragmentShader&) = delete;
				OpenGLFragmentShader(OpenGLFragmentShader&&) = delete;
				OpenGLFragmentShader& operator =(const OpenGLFragmentShader&) = delete;
				OpenGLFragmentShader&& operator ==(const OpenGLFragmentShader&&) = delete;
			};
		}
	}
}
