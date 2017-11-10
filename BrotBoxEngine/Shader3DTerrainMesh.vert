#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const int AMOUNT_OF_LIGHTS   = 4;

out gl_PerVertex {
	vec4 gl_Position;
};

struct Light
{
	vec3 pos;
	float used;
};


layout(location = 0) in vec3 inPos;

layout(location = 1) out vec3 outViewVec;
layout(location = 2) out vec2 outUVPos;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out Light outLight[AMOUNT_OF_LIGHTS];


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
} pushConts;


void main() 
{
	vec4 worldPos = pushConts.modelMatrix * vec4(inPos, 1.0);
	vec4 viewPos = uboProjection.view * worldPos;

	gl_Position = uboProjection.projection * viewPos;
	
	outViewVec = -(uboProjection.view * worldPos).xyz;

	outUVPos = worldPos.xy / 4096; //TODO
	outNormal = mat3(uboProjection.view) * vec3(0, 0, 1);
	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		outLight[i].used = uboLights.light[i].used;
		if(uboLights.light[i].used > 0.0f)
		{
			vec3 lightPos = uboLights.light[i].pos;
			outLight[i].pos = mat3(uboProjection.view) * (lightPos - vec3(worldPos));
		}
	}

}