#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 MVP_model;
    mat4 MVP_light;
    mat4 MV;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outLight;

void main() {
    gl_Position = ubo.MVP_model * vec4(inPosition, 1.0);
    outLight = (ubo.MVP_light * vec4(2,5,2,1)).xyz;
    outPosition = gl_Position.xyz;
}