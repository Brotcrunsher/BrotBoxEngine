#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;

layout(push_constant) uniform PushConstants
{
	layout(offset = 64) vec4 color;
	
	layout(offset = 80) vec2 magnetPos[3];
	layout(offset = 104) float magnetDistance;
	layout(offset = 108) int maxIter;
	layout(offset = 112) float tickTime;
	layout(offset = 116) float power;
	layout(offset = 120) float magnetStrength;
} pushConts;

int getMagnetIndexForPos(vec2 pos)
{
	float minDist = length(pos - pushConts.magnetPos[0]);
	int index = 0;
	for(int i = 1; i<pushConts.magnetPos.length(); i++)
	{
		float dist = length(pos - pushConts.magnetPos[i]);
		if(dist < minDist)
		{
			minDist = dist;
			index = i;
		}
	}
	return index;
}

vec2 inPosToWorldPos(vec2 inPos)
{
	return vec2(inPos.x * 1280, inPos.y * 720);
}

vec4 indexToColor(int index)
{
	if(index == 0) return vec4(1, 0, 0, 1);
	if(index == 1) return vec4(0, 1, 0, 1);
	if(index == 2) return vec4(0, 0, 1, 1);
	else           return vec4(0, 0, 0, 1);
}

struct SimulateResult
{
	int magnetIndex;
	float percentage;
};

SimulateResult simulatePendulum(vec2 pendulumPos)
{
	vec2 pendulumSpeed = vec2(0, 0);
	int iteration = 0;
	const vec2 middle = vec2(1280 / 2, 720 / 2);
	
	while(true)
	{
		iteration++;
		const vec2 middleAccel = normalize(middle - pendulumPos);
		pendulumSpeed += middleAccel * pushConts.tickTime * 0;
		
		for(int i = 0; i<pushConts.magnetPos.length(); i++)
		{
			vec3 toMagnet = vec3(pushConts.magnetPos[i], pushConts.magnetDistance) - vec3(pendulumPos, 0);
			float length = length(toMagnet);
			float v = pushConts.magnetStrength / pow(length, pushConts.power);
			pendulumSpeed += normalize(toMagnet.xy) * v;
		}
		
		float percentage = float(iteration) / pushConts.maxIter;
		pendulumSpeed *= 1 - percentage;
		pendulumSpeed *= 0.95;
		
		pendulumPos += pendulumSpeed * pushConts.tickTime;
		int magnetIndex = getMagnetIndexForPos(pendulumPos);
		
		if(iteration > pushConts.maxIter)
		{
			return SimulateResult(0xFFFF, percentage);
		}
		
		float dist = length(pendulumPos - pushConts.magnetPos[magnetIndex]);
		if(dist < 10)
		{
			return SimulateResult(magnetIndex, percentage);
		}
	}
}

void main()
{
	vec2 worldPos = inPosToWorldPos(inPos);
	SimulateResult simRes = simulatePendulum(worldPos);
	
	outColor = indexToColor(simRes.magnetIndex) * (1 - simRes.percentage);
}