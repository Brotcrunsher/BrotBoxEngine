#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const int AMOUNT_OF_LIGHTS   = 4;
layout(constant_id = 1) const int AMOUNT_OF_TEXTURES = 2;

#define FALLOFF_NONE    0
#define	FALLOFF_LINEAR  1
#define FALLOFF_SQUARED 2
#define FALLOFF_CUBIC   3
#define FALLOFF_SQRT    4

struct LightInput
{
	vec3 lightVec;
	float lightUsed;
};

layout(location = 0) out vec4 outColor;

layout(set = 4, binding = 0) uniform sampler2D baseTex;

layout(set = 5, binding = 0) uniform sampler2D additionalTex[AMOUNT_OF_TEXTURES];
layout(set = 6, binding = 0) uniform sampler2D textureWeights[AMOUNT_OF_TEXTURES];

layout(push_constant) uniform PushConstants
{
	layout(offset = 0)vec4 color;
	layout(offset = 16)vec4 textureBias;
} pushConts;

struct Light
{
	float lightStrength;
	int falloffMode;
	float pad1;
	float pad2;
	vec4 lightColor;
	vec4 specularColor;
};
layout(set = 2, binding = 0) uniform UBOLights
{
	Light light[AMOUNT_OF_LIGHTS];
} uboLights;

void main() {
	outColor = vec4(1, 1, 1, 1);
}