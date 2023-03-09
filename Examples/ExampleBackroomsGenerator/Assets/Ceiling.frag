#version 300 es
precision highp float;
uniform vec4 inColor;
in vec4 passPos;
in vec4 passWorldPos;
in vec4 passNormal;
in vec2 passUvCoord;
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

vec3 getColor(vec3 x) {
	vec3 fl = floor(x);
	vec3 fr = fract(x);
	
	if(fr.x < 0.1 || fr.y < 0.1) return vec3(117.0 / 255.0, 109.0 / 255.0, 46.0 / 255.0);
	//float h = hash(fl);
	//if(h < 0.01) return vec3(10.0, 10.0, 10.0);
	return vec3(125.0 / 255.0, 115.0 / 255.0, 54.0 / 255.0);
}

void main()
{
   outPos      = passPos;
   outNormal   = vec4(normalize(passNormal.xyz), 1.0);
   outAlbedo   = vec4(getColor(passWorldPos.xyz), 1.0) * inColor;
   outSpecular = vec4(1.0, 0.0, 0.0, 1.0);
   outEmissions = vec4(0.0, 0.0, 0.0, 1.0);
}