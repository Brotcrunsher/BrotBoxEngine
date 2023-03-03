#include "BBE/PointLight.h"

bbe::PointLight::PointLight()
{
}

bbe::PointLight::PointLight(const Vector3& pos)
	: pos(pos)
{
}

bbe::IcoSphere bbe::PointLight::getLightVolume(const bbe::Vector3& cameraPos) const
{
	// Rationale for these numbers:
	//    (1/2.2) is the inverse of gamma used for gamma correction.
	//    (1 / 255) is the smallest renderable difference for a single channel.
	float radius = 0;
	switch (falloffMode)
	{
	case LightFalloffMode::LIGHT_FALLOFF_NONE:
		return bbe::IcoSphere(cameraPos);
	case LightFalloffMode::LIGHT_FALLOFF_LINEAR:
		radius = /*(1 / x )^(1/2.2) < (1 / 255)*/ 196964.699f * lightStrength;
		break;
	case LightFalloffMode::LIGHT_FALLOFF_SQUARED:
		radius = /*(1 / x²)^(1/2.2) < (1 / 255)*/ 443.807f * lightStrength;
		break;
	case LightFalloffMode::LIGHT_FALLOFF_CUBIC:
		radius = /*(1 / x³)^(1/2.2) < (1 / 255)*/ 58.183f * lightStrength;
		break;
	case LightFalloffMode::LIGHT_FALLOFF_SQRT:
		// Would be (1 / sqrt(x))^(1/2.2) < (1 / 255)
		// But that's 10^10, so we might as well...
		return bbe::IcoSphere(cameraPos);
	default:
		throw IllegalStateException();
	}

	return bbe::IcoSphere(pos, bbe::Vector3(radius));
}
