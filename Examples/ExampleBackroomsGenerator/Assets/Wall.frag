#version 300 es
precision highp float;
uniform vec4 inColor;
uniform sampler2D albedo;
uniform sampler2D normals;
uniform sampler2D emissions;
in vec4 passPos;
in vec4 passWorldPos;
in vec4 passNormal;
in vec2 passUvCoord;
in vec3 worldNormal;
in vec3 upViewSpace;
layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outSpecular;
layout (location = 4) out vec4 outEmissions;

float hash(vec3 p) {
	p = mod(p, 10000.0f);
    float x = dot(p, vec3(442.843, 657.486, 270.766));
    float y = dot(p, vec3(114.474, 132.400, 261.903));
    float z = dot(p, vec3(136.280, 536.801, 704.473));
    return fract(sin(x) * 704.745 + sin(y) * 981.705 + sin(z) * 271.722);
}

#define M_PI 3.1415926535897932384626433832795
float cosMix(float a, float b, float t)
{
	return mix(a, b, (1.0-cos(t*M_PI))/2.0);
}

float octave(int oc, vec3 x) {
	float mult = pow(2.0, float(oc));
	x *= mult;
	vec3 fl = floor(x);
	vec3 fr = abs(x - fl);
	
	float a1 = hash(fl + vec3(0.0, 0.0, 0.0));
	float a2 = hash(fl + vec3(0.0, 0.0, 1.0));
	float a3 = hash(fl + vec3(0.0, 1.0, 0.0));
	float a4 = hash(fl + vec3(0.0, 1.0, 1.0));
	float a5 = hash(fl + vec3(1.0, 0.0, 0.0));
	float a6 = hash(fl + vec3(1.0, 0.0, 1.0));
	float a7 = hash(fl + vec3(1.0, 1.0, 0.0));
	float a8 = hash(fl + vec3(1.0, 1.0, 1.0));
	
	float b1 = cosMix(a1, a2, fr.z);
	float b2 = cosMix(a3, a4, fr.z);
	float b3 = cosMix(a5, a6, fr.z);
	float b4 = cosMix(a7, a8, fr.z);
	
	float c1 = cosMix(b1, b2, fr.y);
	float c2 = cosMix(b3, b4, fr.y);
	
	return cosMix(c1, c2, fr.x) / mult;
}

float noise(vec3 x) {
	float retVal = 0.0;
	for(int i = 0; i<8; i++)
	{
		retVal += octave(i, x);
	}
	return retVal / 2.0;
}

vec3 wallMat(vec3 x, vec3 normal, float fragmentSpread) {
	vec3 right = cross(normal, vec3(0, 0, 1));
	float retVal = 0.0;
	int samples = 8;
	x += vec3(noise(x)) * 0.1;
	for(int i = 0; i<samples; i++)
	{
		float percentage = float(i) / float(samples) - 0.5f;
		vec3 v = x + right * percentage * fragmentSpread;
		float z = fract((v.x + v.y) * 5.0);
		if(z > 0.7 && z < 0.75) retVal += 1.0;
		else if(z >= 0.825 && z < 0.875 && fract(v.z * 10.0) < 0.5) retVal += 1.0;
		else if(z > 0.95) retVal += 1.0;
		else if(z <= 0.7) retVal += 0.25;
	}
	retVal /= float(samples);
	return vec3(retVal / 4.0);
}

void main()
{
   vec3 normalNormalized = normalize(passNormal.xyz);
   vec3 right = normalize(cross(normalNormalized, upViewSpace));
   vec3 posNormalized = normalize(passPos.xyz);
   float viewAngle = acos(abs(dot(posNormalized, right)));
   float distanceToCamera = length(passPos);
   float fragmentSpread = (distanceToCamera * sin(M_PI / 3.0 / 1280.0 * 2.0)) / sin(viewAngle); // TODO, "M_PI / 3.0 / 1280.0" must be replaces with pixel angel
   
   outNormal = vec4(normalNormalized, 1.0);
   outPos    = passPos;
   
   float mult = 100.0;
   float noiseVal = noise(mult * (passWorldPos.xyz * 0.01));
   vec3 c1 = vec3(83.0 / 255.0, 67.0 / 255.0, 16.0 / 255.0);
   vec3 c2 = vec3(104.0 / 255.0, 85.0 / 255.0, 27.0 / 255.0);
   outAlbedo = vec4((mix(c1, c2, noiseVal)) + wallMat(passWorldPos.xyz, normalize(worldNormal), fragmentSpread), 1.0) * inColor;
   outSpecular = vec4(1.0, 0.4, 0.0, 1.0);
   outEmissions = texture(emissions, passUvCoord);
}