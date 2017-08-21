#version 450
#extension GL_ARB_separate_shader_objects : enable

#define AMOUNT_OF_LIGHTS 4

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inViewVec;
layout(location = 2) in vec3 inLightVec[AMOUNT_OF_LIGHTS];
layout(location = 3 + AMOUNT_OF_LIGHTS) in float lightUsed[AMOUNT_OF_LIGHTS];

layout(push_constant) uniform PushConstants
{
	vec4 color;
} pushConts;

void main() {
	vec3 texColor = vec3(1, 1, 1); //TODO change to pushConts.color
	vec3 N = normalize(inNormal);	
	vec3 V = normalize(inViewVec);
	vec3 ambient = texColor * 0.1;
	vec3 diffuse  = vec3(0);
	vec3 specular = vec3(0);


	for(int i = 0; i<AMOUNT_OF_LIGHTS; i++)
	{
		if(lightUsed[i] <= 0.0)
		{
			continue;
		}
		float distToLight = length(inLightVec[i]);
		float lightPower = 1;
		if(distToLight > 0)
		{
			//lightPower = 1 / distToLight * 10.0f;
		}

		vec3 L = normalize(inLightVec[i]);
		vec3 R = reflect(-L, N);

	
		diffuse += max(dot(N, L), 0.0) * texColor * lightPower;
		specular += pow(max(dot(R, V), 0.0), 4.0) * vec3(0.35) * lightPower;
	}
	

	outColor = vec4(ambient + diffuse + specular, 1.0);
}