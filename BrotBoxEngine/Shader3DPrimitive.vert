#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UBOProjection
{
	mat4 view;
	mat4 projection;
} uboProjection;

layout(binding = 1) uniform UBOModel
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
layout(location = 2) out vec3 outLightVec;


void main() 
{
	vec3 lightPos = vec3(0, 0, 0);

	vec4 worldPos = uboModel.model[pushConts.uboModelIndex] * vec4(inPos, 1.0);
	gl_Position = uboProjection.projection * uboProjection.view * worldPos;
	outNormal = mat3(uboProjection.view) * mat3(uboModel.model[pushConts.uboModelIndex]) * inNormal;
	outViewVec = -(uboProjection.view * worldPos).xyz;
	outLightVec = mat3(uboProjection.view) * (lightPos - vec3(worldPos));
}