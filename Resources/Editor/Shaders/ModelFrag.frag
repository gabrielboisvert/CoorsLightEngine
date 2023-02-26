#version 450

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main() 
{
//    vec2 res = inUV / 2;
//    res = res / 0.1;
//
//    vec3 colorA = vec3(0.270833, 0.256448, 0.25916);
//    vec3 colorB = vec3(0.359266, 0.401042, 0.36405);
//
//    vec3 colorFinal = mix(colorA, colorB, texture(texSampler, res).z);
//
//
//    outColor = vec4(colorFinal, 1);


	outColor = vec4(texture(texSampler, inUV).xyz, 1);
}