for f in *.vert
do
    prefix="$(cut -d'.' -f1 <<<"$f")"
    $VULKAN_SDK/bin/glslc $f -o ${prefix}_vert.spv
done

for f in *.frag
do
    prefix="$(cut -d'.' -f1 <<<"$f")"
    $VULKAN_SDK/bin/glslc $f -o ${prefix}_frag.spv
done
