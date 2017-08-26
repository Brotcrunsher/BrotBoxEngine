#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inUvCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D tex;

layout(push_constant) uniform PushConstants
{
	vec4 color;
} pushConts;

void main() {
	outColor = pushConts.color * texture(tex, inUvCoord);
}