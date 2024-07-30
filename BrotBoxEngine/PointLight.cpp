#include "BBE/PointLight.h"

bbe::PointLight::PointLight()
{
}

bbe::PointLight::PointLight(const Vector3& pos)
	: pos(pos)
{
}

float bbe::PointLight::getLightRadius() const
{
	// Rationale for these numbers:
	//    (1/2.2) is the inverse of gamma used for gamma correction.
	//    (1 / 255) is the smallest renderable difference for a single channel.
	float radius = 0;
	switch (falloffMode)
	{
	case LightFalloffMode::LIGHT_FALLOFF_NONE:
		return 1000000.f;
	case LightFalloffMode::LIGHT_FALLOFF_LINEAR:
		radius = /*(1 / x  )^(1/2.2) < (1 / 255)*/ 500 * lightStrength;
		break;
	case LightFalloffMode::LIGHT_FALLOFF_SQUARED:
		radius = /*(1 / x^2)^(1/2.2) < (2 / 255)*/ 25 * lightStrength;
		break;
	case LightFalloffMode::LIGHT_FALLOFF_CUBIC:
		radius = /*(1 / x^3)^(1/2.2) < (1 / 255)*/ 58.183f * lightStrength;
		break;
	case LightFalloffMode::LIGHT_FALLOFF_SQRT:
		return 1000000.f;
	default:
		bbe::Crash(bbe::Error::IllegalState);
	}

	if (radius > 10000)
	{
		return 10000;
	}

	return radius;
}
