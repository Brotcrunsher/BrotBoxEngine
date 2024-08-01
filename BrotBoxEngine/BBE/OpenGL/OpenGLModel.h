#pragma once

#include "BBE/glfwWrapper.h"
#include "BBE/AutoRefCountable.h"
#include "BBE/Model.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			class OpenGLModel :
				public AutoRefCountable
			{
			private:
				GLuint m_vbo = 0;
				GLuint m_ibo = 0;
				size_t m_amountOfIndices = 0;
			public:
				explicit OpenGLModel(const bbe::Model& model);
				virtual ~OpenGLModel() override;

				GLuint getVbo();
				GLuint getIbo();

				size_t getAmountOfIndices();
			};
		}
	}
}
