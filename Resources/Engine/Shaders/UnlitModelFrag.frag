#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main() 
{
	outColor = vec4(texture(texSampler, inUV).xyz, 1);
}