#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUvCoord;

layout(location = 0) out vec2 outUvCoord;

layout(push_constant) uniform PushConstants
{
	layout(offset = 16) vec4 posScale;
} pushConts;

void main() 
{
	vec2 pos = inPos * pushConts.posScale.zw + pushConts.posScale.xy;
	gl_Position = vec4(pos, 0.0, 1.0);
	outUvCoord = inUvCoord;

}