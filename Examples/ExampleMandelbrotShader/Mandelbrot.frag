#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;

layout(push_constant) uniform PushConstants
{
	layout(offset = 64) vec4 color;
	
	layout(offset =  80) double middleX;
	layout(offset =  88) double middleY;
	layout(offset =  96) double rangeX;
	layout(offset = 104) double rangeY;
	layout(offset = 112) int max_iteration;
} pushConts;

float myMod(float val, float mod)
{
	if (val >= 0)
	{
		if (val < mod)
		{
			return val;
		}
		int div = int(val / mod);
		return val - div * mod;
	}
	else
	{
		val = val - int(val / mod) * mod + mod;
		return val - int(val / mod) * mod;
	}
}

vec4 HSVtoRGBA(float h, float s, float v)
{
	h = myMod(h, 360);
	int hi = int(h / 60);
	float f = (h / 60 - hi);

	float p = v * (1 - s);
	float q = v * (1 - s * f);
	float t = v * (1 - s * (1 - f));

	switch (hi)
	{
	case 1:
		return vec4(q, v, p, 1);
	case 2:
		return vec4(p, v, t, 1);
	case 3:
		return vec4(p, q, v, 1);
	case 4:
		return vec4(t, p, v, 1);
	case 5:
		return vec4(v, p, q, 1);
	default:
		return vec4(v, t, p, 1);
	}
}

void main() {
	int max_iteration = pushConts.max_iteration;
	double middleX = pushConts.middleX;
	double middleY = pushConts.middleY;
	double rangeX = pushConts.rangeX;
	double rangeY = pushConts.rangeY;
	
	double x0 = inPos.x * rangeX + middleX - rangeX / 2;
	double y0 = inPos.y * rangeY + middleY - rangeY / 2;
	
	double real = 0;
	double imaginary = 0;

	int iteration = 0;
	while (real * real + imaginary * imaginary <= 4 && iteration < max_iteration)
	{
		double temp = real * real - imaginary * imaginary + x0;
		imaginary = 2 * real * imaginary + y0;
		real = temp;
		iteration++;
	}

	float colorVal = float(iteration) / max_iteration;
	
	outColor = HSVtoRGBA(-colorVal * 360 + 240, 1, 1 - colorVal);
}