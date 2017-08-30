#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const int AMOUNT_OF_LIGHTS = 4;

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

layout(push_constant) uniform PushConstants
{
	layout(offset = 16)mat4 modelMatrix;
	layout(offset = 80)int lod;
} pushConts;

layout(location = 0) in float inPos;

layout(location = 1) out vec3 outViewVec;
layout(location = 2) out OutLightVertexInput
{
	vec3 outLightVec;
	float lightUsed;
}outLightVertexInput[AMOUNT_OF_LIGHTS];


void main() 
{
	int posX = gl_VertexIndex / ((256 >> pushConts.lod) + 1);
	int posY = gl_VertexIndex % ((256 >> pushConts.lod) + 1);

	vec4 worldPos = pushConts.modelMatrix * vec4(posX * 0.5f * (1 << pushConts.lod), posY * 0.5f * (1 << pushConts.lod), inPos, 1.0);
	gl_Position = uboProjection.projection * uboProjection.view * worldPos;
	outViewVec = -(uboProjection.view * worldPos).xyz;
	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		outLightVertexInput[i].lightUsed = uboLights.light[i].used;
		if(uboLights.light[i].used > 0.0f)
		{
			vec3 lightPos = uboLights.light[i].pos;
			outLightVertexInput[i].outLightVec = mat3(uboProjection.view) * (lightPos - vec3(worldPos));
		}
	}
	
}