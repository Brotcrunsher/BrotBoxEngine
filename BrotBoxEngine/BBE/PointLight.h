#pragma once
#include "../BBE/Vector3.h"
#include "../BBE/IcoSphere.h"
#include "../BBE/DynamicArray.h"
#include "../BBE/Stack.h"
#include "../BBE/Color.h"
#include "../BBE/LightFalloffMode.h"

namespace bbe
{
	struct PointLight
	{
		bbe::Vector3 pos;
		float lightStrength = 10.0f;
		bbe::Color lightColor = bbe::Color(1, 1, 1, 1);
		bbe::Color specularColor = bbe::Color(.35f, .35f, .35f, 1);
		LightFalloffMode falloffMode = LightFalloffMode::LIGHT_FALLOFF_LINEAR;

		PointLight();
		explicit PointLight(const Vector3 &pos);

		bbe::IcoSphere getLightVolume(const bbe::Vector3& cameraPos) const;
	};
}