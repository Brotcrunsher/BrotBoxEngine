#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec4 inColor;

layout(push_constant) uniform PushConstants
{
	vec4 color;
} pushConts;

void main() {
	//outColor = pushConts.color;
	outColor = vec4(inColor.xyz, 1.0);
}