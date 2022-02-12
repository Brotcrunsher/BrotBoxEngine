#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;

layout(push_constant) uniform PushConstants
{
	layout(offset = 64) vec4 color;
} pushConts;

void main() {
	outColor = pushConts.color;
}