#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	layout(offset = 100)float offsetX;
	layout(offset = 104)float offsetY;
	layout(offset = 108)float patchSize;
	layout(offset = 112)float heightmapScaleX;
	layout(offset = 116)float heightmapScaleY;
} pushConts;

layout(location = 0) in vec2 inPos;

layout(location = 0) out vec2 outHeightMapPos;




void main() 
{
	float x = inPos.x + pushConts.offsetX / pushConts.patchSize;
	float y = inPos.y + pushConts.offsetY / pushConts.patchSize;
	gl_Position = vec4(x * pushConts.patchSize, y * pushConts.patchSize, 0, 1.0);
	outHeightMapPos = vec2(x * pushConts.heightmapScaleX, y * pushConts.heightmapScaleY);
}