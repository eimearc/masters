#version 450

layout(set=0, binding = 0) uniform UniformBufferObject
{
    mat4 MVP_model;
    mat4 MVP_light;
    mat4 MV;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

void main() {
    gl_Position = ubo.MVP_model * vec4(inPosition, 1.0);
    outNormal = normalize(ubo.MV * vec4(inNormal,0)).xyz;
}