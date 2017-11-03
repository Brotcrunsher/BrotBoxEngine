#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (vertices = 4) out;

layout(location = 0) in vec2 inHeightMapPos[];

layout(location = 0) out vec2 outHeightMapPos[4];
layout(location = 1) in float inHeightOffset[];
layout(location = 1) out float outHeightOffset[4];

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
	if (gl_InvocationID == 0)
	{
		vec3 p0 = (/*uboProjection.view **/ pushConts.modelMatrix * gl_in[0].gl_Position).xyz;
		vec3 p1 = (/*uboProjection.view **/ pushConts.modelMatrix * gl_in[1].gl_Position).xyz;
		vec3 p2 = (/*uboProjection.view **/ pushConts.modelMatrix * gl_in[2].gl_Position).xyz;
		vec3 p3 = (/*uboProjection.view **/ pushConts.modelMatrix * gl_in[3].gl_Position).xyz;

		float l0 = clamp(16384.0f / length(p0), 2, 1024);
		float l1 = clamp(16384.0f / length(p1), 2, 1024);
		float l2 = clamp(16384.0f / length(p2), 2, 1024);
		float l3 = clamp(16384.0f / length(p3), 2, 1024);

		
		gl_TessLevelOuter[0] = max(l0, l3);
		gl_TessLevelOuter[1] = max(l0, l1);
		gl_TessLevelOuter[2] = max(l2, l1);
		gl_TessLevelOuter[3] = max(l2, l3);
		gl_TessLevelInner[0] = max(gl_TessLevelOuter[0], gl_TessLevelOuter[3]);
		gl_TessLevelInner[1] = max(gl_TessLevelOuter[2], gl_TessLevelOuter[1]);
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	outHeightMapPos[gl_InvocationID] = inHeightMapPos[gl_InvocationID];

	outHeightOffset[gl_InvocationID] = inHeightOffset[gl_InvocationID];
}