#ifndef EVK_DRAW
#define EVK_DRAW

#include <array>
#include "buffer.h"
#include "command.h"
#include "descriptor.h"
#include "framebuffer.h"
#include "pass.h"
#include "pipeline.h"
#include "swapchain.h"
#include "sync.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

void executeDrawCommands(
    const Device &device,
    const std::vector<Pipeline> &pipelines,
    const Swapchain &swapchain,
    const Commands &commands,
    Sync &sync
);

void recordDrawCommands(
    Device &device,
    const Buffer &indexBuffer,
    const Buffer &vertexBuffer,
    std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass,
    Swapchain &swapchain,
    Framebuffer &framebuffers,
    Commands &commands
);

#endif