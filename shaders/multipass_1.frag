#version 450 core

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputDepth;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(subpassLoad(inputColor).b,1,0,1);
    // outColor = vec4(subpassLoad(inputDepth).rgb,1);
}