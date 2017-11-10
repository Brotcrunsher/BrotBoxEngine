#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const int AMOUNT_OF_LIGHTS = 4;
layout(quads, equal_spacing) in;


out gl_PerVertex {
	vec4 gl_Position;
};

struct Light
{
	vec3 pos;
	float used;
};

layout(set = 0, binding = 0) uniform UBOLights
{
	Light light[AMOUNT_OF_LIGHTS];
} uboLights;

layout(set = 1, binding = 0) uniform UBOProjection
{
	mat4 view;
	mat4 projection;
} uboProjection;

layout(push_constant) uniform PushConstants
{
	layout(offset = 16)mat4 modelMatrix;
	layout(offset = 80)float maxTerrainHeight;
} pushConts;

layout(location = 0) in vec2 inHeightMapPos[];
layout(location = 1) in float inHeightOffset[];

layout(location = 1) out vec3 outViewVec;
layout(location = 2) out vec2 outHeightMapPos;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out Light outLight[AMOUNT_OF_LIGHTS];

layout(set = 3, binding = 0) uniform sampler2D tex;

void main() 
{
	vec2 heightMapPos1 = mix(inHeightMapPos[0], inHeightMapPos[1], gl_TessCoord.x);
	vec2 heightMapPos2 = mix(inHeightMapPos[3], inHeightMapPos[2], gl_TessCoord.x);
	vec2 heightMapPos = mix(heightMapPos1, heightMapPos2, gl_TessCoord.y);

	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

	vec3 inPos = pos.xyz;
	inPos.z = texture(tex, heightMapPos).x * pushConts.maxTerrainHeight + inHeightOffset[0];

	vec4 worldPos = pushConts.modelMatrix * vec4(inPos, 1.0);
	vec4 viewPos = uboProjection.view * worldPos;
	gl_Position = uboProjection.projection * viewPos;
	outViewVec = -(uboProjection.view * worldPos).xyz;
	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		outLight[i].used = uboLights.light[i].used;
		if(uboLights.light[i].used > 0.0f)
		{
			vec3 lightPos = uboLights.light[i].pos;
			outLight[i].pos = mat3(uboProjection.view) * (lightPos - vec3(worldPos));
		}
	}
	
	outNormal = mat3(uboProjection.view) * vec3(0, 0, 1);

	outHeightMapPos = heightMapPos;
}