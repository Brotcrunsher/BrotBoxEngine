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

layout(location = 1) in vec3 inViewVec;
layout(location = 2) in vec2 inHeightMapPos;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in float inDistanceToCamera;
layout(location = 5) in LightInput inLight[AMOUNT_OF_LIGHTS];

layout(set = 4, binding = 0) uniform sampler2D baseTex;
layout(set = 5, binding = 0) uniform TextureBias
{
	vec4 data;
}textureBias;
layout(set = 6, binding = 0) uniform sampler2D additionalTex[AMOUNT_OF_TEXTURES];
layout(set = 7, binding = 0) uniform sampler2D textureWeights[AMOUNT_OF_TEXTURES];

layout(push_constant) uniform PushConstants
{
	vec4 color;
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
	float weights[AMOUNT_OF_TEXTURES];
	float weightSum = 0;
	vec3 texColors[AMOUNT_OF_TEXTURES];
	vec3 texColor = vec3(0);
	for(int i = 0; i<AMOUNT_OF_TEXTURES; i++)
	{
		weights[i] = texture(textureWeights[i], inHeightMapPos).x;
		weightSum += weights[i];
		texColors[i] = texture(additionalTex[i], inHeightMapPos * textureBias.data.xy + textureBias.data.zw, 0).xyz;

		texColor += texColors[i] * weights[i];
	}

	if(weightSum < 1)
	{
		vec3 texColorBase = texture(baseTex, inHeightMapPos * textureBias.data.xy + textureBias.data.zw, 0).xyz;
		texColor += texColorBase * (1 - weightSum);
	}



	vec3 V = normalize(inViewVec);
	vec3 N = normalize(inNormal);
	vec3 ambient = texColor * 0.1;
	vec3 diffuse  = vec3(0);
	vec3 specular = vec3(0);


	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		if(inLight[i].lightUsed <= 0.0)
		{
			outColor = vec4(0.1, 0.1, 0.1, 1);
			continue;
		}
		float distToLight = length(inLight[i].lightVec);
		float lightPower = uboLights.light[i].lightStrength;
		if(distToLight > 0)
		{
			switch(uboLights.light[i].falloffMode)
			{
			case FALLOFF_NONE:
				//Do nothing
				break;
			case FALLOFF_LINEAR:
				lightPower = uboLights.light[i].lightStrength / distToLight;
				break;
			case FALLOFF_SQUARED:
				lightPower = uboLights.light[i].lightStrength / distToLight / distToLight;
				break;
			case FALLOFF_CUBIC:
				lightPower = uboLights.light[i].lightStrength / distToLight / distToLight / distToLight;
				break;
			case FALLOFF_SQRT:
				lightPower = uboLights.light[i].lightStrength / sqrt(distToLight);
				break;
			}
			
		}

		vec3 L = normalize(inLight[i].lightVec);
		vec3 R = reflect(-L, N);

	
		diffuse += max(dot(N, L), 0.0) * (texColor * uboLights.light[i].lightColor.xyz) * lightPower;
		//specular += pow(max(dot(R, V), 0.0), 4.0) * uboLights.light[i].specularColor.xyz * lightPower;
	}

	outColor = vec4(ambient + diffuse + specular, 1.0);

}