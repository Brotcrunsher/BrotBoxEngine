#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UBOProjection
{
	mat4 viewProjection;
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

layout(location = 0) out vec4 outPos;


void main() 
{
	gl_Position = uboProjection.viewProjection * uboModel.model[pushConts.uboModelIndex] * vec4(inPos, 1.0);
	outPos = gl_Position;
}