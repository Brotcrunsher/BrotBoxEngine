#pragma once

#ifdef BBE_RENDERER_OPENGL
#include "../BBE/Matrix4.h"
#include "../BBE/Model.h"
#include "../BBE/Vector2.h"
#include "../BBE/AutoRefCountable.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace openGl
		{
			class OpenGLManager;
		}
	}
	class PrimitiveBrush3D;
	class Image;
	class FragmentShader;
	struct PointLight;

	class LightBaker
	{
		friend class bbe::INTERNAL::openGl::OpenGLManager;
		friend class bbe::PrimitiveBrush3D;
	public:
		enum class State
		{
			UNINIT,
			INIT,
			MRT_DONE,
			GAMMA_CORRECTED,
			DETACHED,
		};
	private:
		bbe::Matrix4 m_transform;
		bbe::Model m_model;
		const bbe::Image* m_pnormals = nullptr;
		const bbe::FragmentShader* m_pfragementShader = nullptr;
		bbe::Vector2i m_resolution;
		State m_state = State::UNINIT;
		bbe::AutoRef m_prendererData;
		bbe::Vector3 m_lightOffset;
		uint32_t m_amountOfBakedLights = 0;

	public:
		LightBaker();
		LightBaker(const bbe::Matrix4 &transform, const Model& model, const Image* normals, const FragmentShader* shader, const bbe::Vector2i& resolution);

		void init(const bbe::Matrix4& transform, const Model& model, const Image* normals, const FragmentShader* shader, const bbe::Vector2i& resolution);

		State getState() const;
		uint32_t getAmountOfBakedLights() const;
	};
}
#endif