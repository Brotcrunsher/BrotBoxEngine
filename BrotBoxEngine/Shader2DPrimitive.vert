#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) in vec2 inPos;
layout(location = 0) out vec2 passOnPos;

layout(push_constant) uniform PushConstants
{
	layout(offset = 16) vec4 posScale;
	layout(offset = 32) float rotation;
	layout(offset = 40) vec2 screenRatio;
} pushConts;

void main() 
{
	passOnPos = inPos;
	
	float s = sin(pushConts.rotation);
	float c = cos(pushConts.rotation);
	
	vec2 scaledPos = inPos * pushConts.posScale.zw;
	vec2 firstTranslatedPos = scaledPos - pushConts.posScale.zw * 0.5;
	vec2 rotatedPos = vec2(c * firstTranslatedPos.x - s * firstTranslatedPos.y, s * firstTranslatedPos.x + c * firstTranslatedPos.y);
	vec2 secondTranslatedPos = rotatedPos + pushConts.posScale.zw * 0.5;
	
	vec2 pos = secondTranslatedPos / pushConts.screenRatio * 2 + pushConts.posScale.xy / pushConts.screenRatio * 2 - vec2(1, 1);
	gl_Position = vec4(pos, 0.0, 1.0);
	
	/*vec2 aspectPos = vec2((inPos.x + pushConts.posScale.x) / pushConts.screenWidth * 2.f - 1.f, (inPos.y + pushConts.posScale.y) / pushConts.screenHeight * 2.f - 1.f);
	vec2 scaledPos = aspectPos * pushConts.posScale.zw;
	vec2 translatedPos = scaledPos - pushConts.posScale.zw / 2;
	vec2 translatedBackPos = rotatedPos + pushConts.posScale.zw / 2;
	vec2 finalVecWithTranslate = translatedBackPos;
	gl_Position = vec4(finalVecWithTranslate, 0.0, 1.0);*/
	
}

