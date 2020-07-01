#version 450 core

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = vec4(subpassLoad(inputColor).rgb,1);
    outColor=color;
}