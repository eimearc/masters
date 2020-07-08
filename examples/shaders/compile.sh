$VULKAN_SDK/bin/glslc shader.vert -o vert.spv
$VULKAN_SDK/bin/glslc shader.frag -o frag.spv

$VULKAN_SDK/bin/glslc multipass_0.vert -o multipass_0_vert.spv
$VULKAN_SDK/bin/glslc multipass_0.frag -o multipass_0_frag.spv

$VULKAN_SDK/bin/glslc multipass_1.vert -o multipass_1_vert.spv
$VULKAN_SDK/bin/glslc multipass_1.frag -o multipass_1_frag.spv