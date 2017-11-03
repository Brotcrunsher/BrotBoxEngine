#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(push_constant) uniform PushConstants
{
	layout(offset = 84)float offsetX;
	layout(offset = 88)float offsetY;
	layout(offset = 92)float patchSize;
	layout(offset = 96)float heightmapScaleX;
	layout(offset = 100)float heightmapScaleY;
} pushConts;

layout(location = 0) in vec2 inPos;

layout(location = 0) out vec2 outHeightMapPos;
layout(location = 1) out float outHeightOffset;


void main() 
{
	float x = pushConts.offsetX / pushConts.patchSize;
	float y = pushConts.offsetY / pushConts.patchSize;
	gl_Position = vec4(inPos.x * pushConts.patchSize + pushConts.offsetX, inPos.y * pushConts.patchSize + pushConts.offsetY, 0, 1.0);
	outHeightMapPos = vec2((x + inPos.x) * pushConts.heightmapScaleX, (y + inPos.y) * pushConts.heightmapScaleY);
	outHeightOffset = mod(x + y, 2);
}