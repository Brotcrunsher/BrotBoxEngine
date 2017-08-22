#version 450
#extension GL_ARB_separate_shader_objects : enable

#define AMOUNT_OF_LIGHTS 4

out gl_PerVertex {
	vec4 gl_Position;
};

struct Light
{
	vec3 pos;
	float used;
};

layout(set = 0, binding = 0) uniform UBOLights
{
	Light light[AMOUNT_OF_LIGHTS];
} uboLights;

layout(set = 1, binding = 0) uniform UBOProjection
{
	mat4 view;
	mat4 projection;
} uboProjection;

layout(set = 1, binding = 1) uniform UBOModel
{
	mat4 model[1024];
} uboModel;

layout(push_constant) uniform PushConstants
{
	layout(offset = 16) int uboModelIndex;
} pushConts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outViewVec;
layout(location = 2) out vec3 outLightVec[AMOUNT_OF_LIGHTS];
layout(location = 3 + AMOUNT_OF_LIGHTS) out float lightUsed[AMOUNT_OF_LIGHTS];


void main() 
{
	

	vec4 worldPos = uboModel.model[pushConts.uboModelIndex] * vec4(inPos, 1.0);
	gl_Position = uboProjection.projection * uboProjection.view * worldPos;
	outNormal = mat3(uboProjection.view) * mat3(uboModel.model[pushConts.uboModelIndex]) * inNormal;
	outViewVec = -(uboProjection.view * worldPos).xyz;
	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		lightUsed[i] = uboLights.light[i].used;
		if(uboLights.light[i].used > 0.0f)
		{
			vec3 lightPos = uboLights.light[i].pos;
			outLightVec[i] = mat3(uboProjection.view) * (lightPos - vec3(worldPos));
		}
	}
	
}