#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (vertices = 4) out;

layout(location = 0) in vec2 inHeightMapPos[];

layout(location = 0) out vec2 outHeightMapPos[4];


void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = 8.0;
		gl_TessLevelInner[1] = 8.0;
		gl_TessLevelOuter[0] = 8.0;
		gl_TessLevelOuter[1] = 8.0;
		gl_TessLevelOuter[2] = 8.0;
		gl_TessLevelOuter[3] = 8.0;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	outHeightMapPos[gl_InvocationID] = inHeightMapPos[gl_InvocationID];
}