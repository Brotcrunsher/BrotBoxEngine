#version 300 es
precision highp float;

out vec4 outColor;
in vec2 passPosition;

uniform vec2 magnetPos[3];
uniform float magnetDistance;
uniform int maxIter;
uniform float tickTime;
uniform float power;
uniform float magnetStrength;

int getMagnetIndexForPos(vec2 pos)
{
	float minDist = length(pos - magnetPos[0]);
	int index = 0;
	for(int i = 1; i<magnetPos.length(); i++)
	{
		float dist = length(pos - magnetPos[i]);
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
	return vec2(inPos.x * 1280.0f, inPos.y * 720.0f);
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
	vec2 pendulumSpeed = vec2(0.0f, 0.0f);
	int iteration = 0;
	const vec2 middle = vec2(1280.0f / 2.0f, 720.0f / 2.0f);
	
	while(true)
	{
		iteration++;
		vec2 middleAccel = normalize(middle - pendulumPos);
		
		for(int i = 0; i<magnetPos.length(); i++)
		{
			vec3 toMagnet = vec3(magnetPos[i], magnetDistance) - vec3(pendulumPos, 0);
			float length = length(toMagnet);
			float v = magnetStrength / pow(length, power);
			pendulumSpeed += normalize(toMagnet.xy) * v;
		}
		
		float percentage = float(iteration) / float(maxIter);
		pendulumSpeed *= 1.0f - percentage;
		pendulumSpeed *= 0.95;
		
		pendulumPos += pendulumSpeed * tickTime;
		int magnetIndex = getMagnetIndexForPos(pendulumPos);
		
		if(iteration > maxIter)
		{
			return SimulateResult(0xFFFF, percentage);
		}
		
		float dist = length(pendulumPos - magnetPos[magnetIndex]);
		if(dist < 10.0f)
		{
			return SimulateResult(magnetIndex, percentage);
		}
	}
}

void main()
{
	vec2 worldPos = inPosToWorldPos(passPosition);
	SimulateResult simRes = simulatePendulum(worldPos);
	
	outColor = indexToColor(simRes.magnetIndex) * (1.0f - simRes.percentage);
}