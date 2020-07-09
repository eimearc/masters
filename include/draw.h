#ifndef EVK_DRAW
#define EVK_DRAW

#include <array>
#include "buffer.h"
#include "command.h"
#include "descriptor.h"
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
    Buffer &indexBuffer,
    Buffer &vertexBuffer,
    const std::vector<Descriptor> &descriptors,
    const std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass,
    const Swapchain &swapchain,
    Framebuffer &framebuffers,
    Commands &commands
);

#endif