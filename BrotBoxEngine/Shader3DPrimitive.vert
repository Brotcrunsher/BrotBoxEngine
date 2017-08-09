#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UBOViewProjection
{
	mat4 viewProjection;
} uboViewProjection;

layout(binding = 1) uniform UBOModel
{
	mat4 model;
} uboModel;

layout(location = 0) in vec3 inPos;

void main() 
{
	gl_Position = uboViewProjection.viewProjection * uboModel.model * vec4(inPos, 1.0);
}