#version 300 es
precision highp float;

//TODO: Currently the colors are evaluated on every ray marcher step. It would be better if
//      they only got evaluated once the Ray marching porcess completed and a surface was hit.

out vec4 outColor;
in vec2 passPosition;

uniform float camX;
uniform float camY;
uniform float camZ;
uniform float camForwardX;
uniform float camForwardY;
uniform float camForwardZ;

vec3 sunDirection = vec3(-1, -5, -1);
float intersectionDistance = 0.00001;

vec3 background(vec3 direction)
{
	return vec3(dot(sunDirection, direction), dot(sunDirection, direction), 1);
}

vec4 sphereSdf(vec3 sdfPos, vec3 sphereCenter, float radius)
{
	vec3 toCenter = sphereCenter - sdfPos;
	return vec4(0.5, 0.5, 0.5, length(toCenter) - radius);
}

vec4 yPlaneSdf(vec3 sdfPos, float y)
{
	bool bx = mod(sdfPos.x, 2.0f) > 1.0f;
	bool by = mod(sdfPos.y, 2.0f) > 1.0f;
	bool bz = mod(sdfPos.z, 2.0f) > 1.0f;
	vec3 color = vec3(1, 1, 1);
	if(bx ^^ by ^^ bz)
	{
		color *= 0.4;
	}
	else
	{
		color *= 0.8;
	}
	return vec4(color, y - sdfPos.y);
}

vec4 combine(vec4 sdf1, vec4 sdf2)
{
	if(sdf1.w < sdf2.w) return sdf1;
	else return sdf2;
}

vec4 intersect(vec4 sdf1, vec4 sdf2)
{
	if(sdf1.w > sdf2.w) return sdf1;
	else return sdf2;
}

vec4 subtraction(vec4 sdf1, vec4 sdf2)
{
	sdf2.w *= -1.0f;
	if(sdf1.w > sdf2.w) return sdf1;
	else return sdf2;
}

vec4 sdf(vec3 pos)
{
	return combine(
		subtraction(
			sphereSdf(pos, vec3(0, 0, 3), 0.5f),
			sphereSdf(pos, vec3(-0.25, 0, 2.9), 0.5f)
		),
		yPlaneSdf(pos, 0.5f)
	);
}

vec3 calculateNormal(vec3 pos)
{
	float delta = 0.000001;
	float x = sdf(vec3(pos.x + delta, pos.y, pos.z)).w - sdf(vec3(pos.x - delta, pos.y, pos.z)).w;
	float y = sdf(vec3(pos.x, pos.y + delta, pos.z)).w - sdf(vec3(pos.x, pos.y - delta, pos.z)).w;
	float z = sdf(vec3(pos.x, pos.y, pos.z + delta)).w - sdf(vec3(pos.x, pos.y, pos.z - delta)).w;
	
	return normalize(vec3(x, y, z));
}

float shadowRay(vec3 pos)
{
	vec3 currPos = pos;
	float distanceTraveled = 0.0f;
	while(distanceTraveled < 10.0f)
	{
		vec4 sdfEval = sdf(currPos);
		float distanceNow = sdfEval.w;
		if(distanceNow < intersectionDistance)
		{
			return 0.4;
		}
		currPos += sunDirection * distanceNow;
		distanceTraveled += distanceNow;
	}
	return 1.0f;
}

vec3 evaluateColor(vec3 camPos, vec3 rayDir)
{
	vec3 currPos = camPos;
	vec3 prevPos = currPos;
	float distanceTraveled = 0.0f;
	while(distanceTraveled < 100.0f)
	{
		vec4 sdfEval = sdf(currPos);
		float distanceNow = sdfEval.w;
		if(distanceNow < intersectionDistance)
		{
			float mult = dot(calculateNormal(currPos), sunDirection);
			float ambient = 0.3f;
			mult += ambient;
			mult /= 1.0f + ambient;
			if(mult < 0.0f) mult = 0.0f;
			//TODO Taking the prevPos for a shadowRay is not correct and leads to some visual artifacts.
			return sdfEval.xyz * mult * shadowRay(prevPos);
		}
		prevPos = currPos;
		currPos += rayDir * distanceNow;
		distanceTraveled += distanceNow;
	}
	return background(rayDir);
}

void main() {
	vec3 camPos = vec3(camX, camY, camZ);
	vec3 forwardDir = normalize(vec3(camForwardX, camForwardY, camForwardZ));
	float distanceToScreen = 0.5f;
	float windowWidth = 1280.0f;
	float windowHeight = 720.0f;
	int superSamplingAmount = 4;
	
	sunDirection = normalize(sunDirection);
	float aspectRatio = windowWidth / windowHeight;
	float widthPerPixelOnScreen = 1.0f / windowWidth;
	float heightPerPixelOnScreen = 1.0f / windowHeight;
	
	vec3 result = vec3(0, 0, 0);
	
	for(int i = 0; i<superSamplingAmount; i++)
	{
		for(int k = 0; k<superSamplingAmount; k++)
		{
			vec3 rayDir = vec3(
				 -0.5 + passPosition.x +  widthPerPixelOnScreen * (float(i) / float(superSamplingAmount)), 
				(-0.5 + passPosition.y + heightPerPixelOnScreen * (float(i) / float(superSamplingAmount))) / aspectRatio, 
				distanceToScreen
			);
			
			
			//TODO Doing all this matrix calculation in the fragment shader is unnecessarily wasteful.
			//     Put these calculations in the C++ side and push the matrix instead of the forwardDir.
			vec3 up = vec3(0, -1, 0);
			vec3 right = normalize(cross(forwardDir, up));
			vec3 down = cross(forwardDir, right);
			
			mat3 m;
			m[0][0] = right.x;
			m[0][1] = right.y;
			m[0][2] = right.z;
			
			m[1][0] = down.x;
			m[1][1] = down.y;
			m[1][2] = down.z;
			
			m[2][0] = forwardDir.x;
			m[2][1] = forwardDir.y;
			m[2][2] = forwardDir.z;
			
			rayDir = m * rayDir;
			
			rayDir = normalize(rayDir);
			result += evaluateColor(camPos, rayDir);
		}
	}
	
	outColor = vec4(result / float(superSamplingAmount) / float(superSamplingAmount), 1.0f);
}