#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inViewVec;
layout(location = 2) in vec3 inLightVec;

layout(push_constant) uniform PushConstants
{
	vec4 color;
} pushConts;

void main() {
	vec3 texColor = vec3(1, 1, 1); //TODO change to pushConts.color

	float distToLight = length(inLightVec);
	float lightPower = 1;
	if(distToLight > 0)
	{
		lightPower = 1 / distToLight * 10.0f;
	}

	vec3 N = normalize(inNormal);	
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);

	vec3 ambient = texColor * 0.01;
	vec3 diffuse = max(dot(N, L), 0.0) * texColor * lightPower;
	vec3 specular = pow(max(dot(R, V), 0.0), 4.0) * vec3(0.35) * lightPower;

	outColor = vec4(ambient + diffuse + specular, 1.0);
}