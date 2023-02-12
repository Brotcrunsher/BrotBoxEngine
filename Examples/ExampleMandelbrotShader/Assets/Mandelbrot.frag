#version 400
precision highp float;

out vec4 outColor;
in vec2 passPosition;

uniform double middleX;
uniform double middleY;
uniform double rangeX;
uniform double rangeY;
uniform int max_iteration;

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
	h = myMod(h, 360.0f);
	int hi = int(h / 60.0f);
	float f = (h / 60.0f - hi);

	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (hi)
	{
	case 1:
		return vec4(q, v, p, 1.0f);
	case 2:
		return vec4(p, v, t, 1.0f);
	case 3:
		return vec4(p, q, v, 1.0f);
	case 4:
		return vec4(t, p, v, 1.0f);
	case 5:
		return vec4(v, p, q, 1.0f);
	default:
		return vec4(v, t, p, 1.0f);
	}
}

void main() {
	double x0 = passPosition.x * rangeX + middleX - rangeX / 2.0f;
	double y0 = passPosition.y * rangeY + middleY - rangeY / 2.0f;
	
	double real = 0;
	double imaginary = 0;

	int iteration = 0;
	while (real * real + imaginary * imaginary <= 4 && iteration < max_iteration)
	{
		double temp = real * real - imaginary * imaginary + x0;
		imaginary = 2.0f * real * imaginary + y0;
		real = temp;
		iteration++;
	}

	float colorVal = float(iteration) / max_iteration;
	
	outColor = HSVtoRGBA(-colorVal * 360.0f + 240.0f, 1, 1 - colorVal);
}