#ifdef BBE_RENDERER_OPENGL
#include "BBE/LightBaker.h"
#include "BBE/PrimitiveBrush3D.h"
#include "BBE/Image.h"
#include "BBE/FragmentShader.h"
#include "BBE/PointLight.h"

bbe::LightBaker::LightBaker()
{
}

bbe::LightBaker::LightBaker(const bbe::Matrix4& transform, const Model& model, const Image* normals, const FragmentShader* shader, const bbe::Vector2i& resolution)
{
	init(transform, model, normals, shader, resolution);
}
void bbe::LightBaker::init(const bbe::Matrix4& transform, const Model& model, const Image* normals, const FragmentShader* shader, const bbe::Vector2i& resolution)
{
	if (m_state != State::UNINIT) throw bbe::IllegalStateException();
	m_state = State::INIT;
	m_transform = transform;
	m_model = model;
	m_pnormals = normals;
	m_pfragementShader = shader;
	m_resolution = resolution;


	// Subtract the transforms position from itself and all the lights. This way
	// all the coordinates stay closer to the origin where they have a higher
	// precision and thus lead to less visual artifacts.
	m_lightOffset = transform.extractTranslation();
	m_transform = bbe::Matrix4::createTranslationMatrix(-m_lightOffset) * transform;
}
bbe::LightBaker::State bbe::LightBaker::getState() const
{
	return m_state;
}
uint32_t bbe::LightBaker::getAmountOfBakedLights() const
{
	return m_amountOfBakedLights;
}

#endif
