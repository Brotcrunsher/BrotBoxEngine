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
					bool built = false;

					void destroy();
					virtual void determinePositions() = 0;
				};

				struct TwoD : ShaderProgramTripple
				{
					GLint screenSizePos = 0;
					GLint scalePosOffsetPos = 0;
					GLint rotationPos = 0;

					void determinePositions() override;
				};

				struct ThreeD : ShaderProgramTripple
				{
					GLint viewPos = 0;
					GLint projectionPos = 0;
					GLint modelPos = 0;
					GLint color3DPos = 0;

					void determinePositions() override;
				};

			private:
				bbe::List<char> code;
				TwoD twoD;
				ThreeD threeD;
				ThreeD threeDForwardNoLight;
				ThreeD threeDBake;
			public:
				TwoD& getTwoD();
				ThreeD& getThreeD();
				ThreeD& getThreeDForwardNoLight();
				ThreeD& getThreeDBake();

				bool hasTwoD() const;
				bool hasThreeD() const;
				bool hasThreeDBake() const;

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
