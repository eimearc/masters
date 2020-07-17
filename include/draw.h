#ifndef EVK_DRAW
#define EVK_DRAW

#include <array>
#include "buffer.h"
#include "descriptor.h"
#include "pass.h"
#include "pipeline.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

void executeDrawCommands(
    Device &device,
    const std::vector<Pipeline> &pipelines
);

void recordDrawCommands(
    Device &device,
    const Buffer &indexBuffer,
    const Buffer &vertexBuffer,
    std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass
);

#endif