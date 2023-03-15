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
				struct ShaderProgramTripple
				{
					GLuint vertex = 0;
					GLuint fragment = 0;
					GLuint program = 0;
					bbe::String errorLog;

					void destroy();
				};

				struct TwoD : ShaderProgramTripple
				{
					GLint screenSizePos = 0;
					GLint scalePosOffsetPos = 0;
					GLint rotationPos = 0;

					void determinePositions();
				};

				struct ThreeD : ShaderProgramTripple
				{
					GLint viewPos = 0;
					GLint projectionPos = 0;
					GLint modelPos = 0;
					GLint color3DPos = 0;

					void determinePositions();
				};

				TwoD twoD;
				ThreeD threeD;
				ThreeD threeDBake;

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
