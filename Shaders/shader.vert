#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

layout(location = 3) in mat4 models;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;

layout(binding = 0) uniform UniformBufferData{
    mat4 model;
    mat4 view;
    mat4 projection;
} uniformBufferData;



void main()
{
    normal = aNormal;
    uv = aUv;
    gl_Position = uniformBufferData.projection * uniformBufferData.view * models * vec4(aPos, 1.0);
}