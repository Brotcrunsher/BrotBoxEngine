#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(constant_id = 0) const int AMOUNT_OF_LIGHTS   = 4;

out gl_PerVertex {
	vec4 gl_Position;
};

struct Light
{
	vec3 pos;
	float used;
};


layout(location = 0) in vec2 inPos;

layout(location = 1) out vec3 outViewVec;
layout(location = 2) out vec2 outUVPos;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out Light outLight[AMOUNT_OF_LIGHTS];

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
	layout(offset = 16 + 16)mat4 modelMatrix;
	layout(offset = 96)float maxTerrainHeight;
	layout(offset = 16 + 16 + 64)vec4 sizeAndOffset;
	layout(offset = 16 + 16 + 64 + 16)float upOffset;
} pushConts;

layout(set = 3, binding = 0) uniform sampler2D tex;

void main() 
{
	float SIZE = 8 * 1024;

	mat4 modelView = uboProjection.view * pushConts.modelMatrix;
	vec3 cameraPos = -(modelView[3]).xyz * mat3(modelView);
	vec3 vertexAnchor = vec3(inPos, 0.0);
	float diffX = cameraPos.x - vertexAnchor.x;
	float diffY = cameraPos.y - vertexAnchor.y;
	float dist = max(abs(diffX), abs(diffY));
	float x = vertexAnchor.x + diffX * pow(2.13f, -dist / SIZE * 2);
	float y = vertexAnchor.y + diffY * pow(2.13f, -dist / SIZE * 2);

	float percentage = 1.0f / (1.0f + pow(2.0f, (dist / 10 - SIZE / 50))) / 3;
	
	x = x * percentage + vertexAnchor.x * (1 - percentage);
	y = y * percentage + vertexAnchor.y * (1 - percentage);

	if(vertexAnchor.x == 0 || vertexAnchor.x == SIZE)
	{
		x = vertexAnchor.x;
	}

	if(vertexAnchor.y == 0 || vertexAnchor.y == SIZE)
	{
		y = vertexAnchor.y;
	}

	vec2 heightMapPos = vec2(x / (8 * 1024), y / (8 * 1024));

	vec4 worldPos = pushConts.modelMatrix * vec4(x, y, texture(tex, heightMapPos).x * pushConts.maxTerrainHeight, 1.0);
	worldPos.z += pushConts.upOffset;
	vec4 viewPos = uboProjection.view * worldPos;

	gl_Position = uboProjection.projection * viewPos;

	outViewVec = -(uboProjection.view * worldPos).xyz;

	outUVPos = (worldPos.xy - pushConts.sizeAndOffset.zw) / pushConts.sizeAndOffset.xy;
	outNormal = mat3(uboProjection.view) * vec3(0, 0, 1);
	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		outLight[i].used = uboLights.light[i].used;
		if(uboLights.light[i].used > 0.0f)
		{
			vec3 lightPos = uboLights.light[i].pos;
			outLight[i].pos = mat3(uboProjection.view) * (lightPos - vec3(worldPos));
		}
	}
}