#version 450 core

layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0,1,0.1,1);
    outColor = vec4(inNormal,1);
}