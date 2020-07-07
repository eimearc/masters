#ifndef DRAW
#define DRAW

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "buffer.h"
#include "descriptor.h"
#include <vector>
#include "command.h"
#include "pipeline.h"
#include "swapchain.h"
#include "pass.h"
#include "sync.h"
#include <array>

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
    const std::vector<Descriptor> &descriptors,
    const std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass,
    const Swapchain &swapchain,
    Framebuffer &framebuffers,
    Commands &commands
);

#endif