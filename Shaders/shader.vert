#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

layout(location = 0) out vec3 normal;

layout(binding = 0) uniform UniformBufferData{
    mat4 model;
    mat4 view;
    mat4 projection;
} uniformBufferData;



void main()
{
    normal = aNormal;
    gl_Position = uniformBufferData.projection * uniformBufferData.view * uniformBufferData.model * vec4(aPos, 1.0);
}