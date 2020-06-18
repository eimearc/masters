$VULKAN_SDK/bin/glslc shader.vert -o vert.spv
$VULKAN_SDK/bin/glslc shader.frag -o frag.spv

$VULKAN_SDK/bin/glslc multipass.vert -o multipass_vert.spv
$VULKAN_SDK/bin/glslc multipass.frag -o multipass_frag.spv
