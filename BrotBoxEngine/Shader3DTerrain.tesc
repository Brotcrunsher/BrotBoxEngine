#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (vertices = 4) out;

layout(location = 0) in vec2 inHeightMapPos[];

layout(location = 0) out vec2 outHeightMapPos[4];

layout(set = 1, binding = 0) uniform UBOProjection
{
	mat4 view;
	mat4 projection;
} uboProjection;

layout(push_constant) uniform PushConstants
{
	layout(offset = 32)mat4 modelMatrix;
} pushConts;


/*layout(set = 5, binding = 0) uniform frustUBO 
{
	vec4 frustum[5];
} frustUbo;*/

/*bool isInsideFrustum()
{
	vec4 pos = gl_in[0].gl_Position;

	for(int i = 0; i<5; i++)
	{
		if(dot(pos, frustUbo.frustum[i]) + 512.0f < 0.0f)
		{
			return false;
		}
	}
	return true;
}*/

void main()
{
	if (gl_InvocationID == 0)
	{
		/*if(!isInsideFrustum())
		{
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelOuter[3] = 0;
			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
		}
		else
		{*/
			vec3 p0 = (uboProjection.view * pushConts.modelMatrix * gl_in[0].gl_Position).xyz;
			vec3 p1 = (uboProjection.view * pushConts.modelMatrix * gl_in[1].gl_Position).xyz;
			vec3 p2 = (uboProjection.view * pushConts.modelMatrix * gl_in[2].gl_Position).xyz;
			vec3 p3 = (uboProjection.view * pushConts.modelMatrix * gl_in[3].gl_Position).xyz;

			float l0 = clamp(1024.0f / length(p0), 2, 64);
			float l1 = clamp(1024.0f / length(p1), 2, 64);
			float l2 = clamp(1024.0f / length(p2), 2, 64);
			float l3 = clamp(1024.0f / length(p3), 2, 64);

		
			gl_TessLevelOuter[0] = max(l0, l3);
			gl_TessLevelOuter[1] = max(l0, l1);
			gl_TessLevelOuter[2] = max(l2, l1);
			gl_TessLevelOuter[3] = max(l2, l3);
			gl_TessLevelInner[0] = max(gl_TessLevelOuter[0], gl_TessLevelOuter[3]);
			gl_TessLevelInner[1] = max(gl_TessLevelOuter[2], gl_TessLevelOuter[1]);
		//}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	outHeightMapPos[gl_InvocationID] = inHeightMapPos[gl_InvocationID];
}