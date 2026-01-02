#version 450

layout(location = 0) out vec4 outputColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(set = 1, binding = 1) uniform sampler2D tex0;

void main()
{
    outputColor = texture(tex0, uv);
}