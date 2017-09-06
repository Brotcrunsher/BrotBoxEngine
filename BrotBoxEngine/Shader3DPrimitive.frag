#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const int AMOUNT_OF_LIGHTS = 4;

#define FALLOFF_NONE    0
#define	FALLOFF_LINEAR  1
#define FALLOFF_SQUARED 2
#define FALLOFF_CUBIC   3
#define FALLOFF_SQRT    4

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inViewVec;
layout(location = 2) in InLightVertexInput
{
	vec3 inLightVec[AMOUNT_OF_LIGHTS];
	float lightUsed[AMOUNT_OF_LIGHTS];
}inLightVertexInput;

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
	vec3 texColor = pushConts.color.xyz;
	vec3 N = normalize(inNormal);	
	vec3 V = normalize(inViewVec);
	vec3 ambient = texColor * 0.1;
	vec3 diffuse  = vec3(0);
	vec3 specular = vec3(0);


	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		if(inLightVertexInput.lightUsed[i] <= 0.0)
		{
			continue;
		}
		float distToLight = length(inLightVertexInput.inLightVec[i]);
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

		vec3 L = normalize(inLightVertexInput.inLightVec[i]);
		vec3 R = reflect(-L, N);

	
		diffuse += max(dot(N, L), 0.0) * (texColor * uboLights.light[i].lightColor.xyz) * lightPower;
		specular += pow(max(dot(R, V), 0.0), 4.0) * uboLights.light[i].specularColor.xyz * lightPower;
	}
	

	outColor = vec4(ambient + diffuse + specular, 1.0);
}