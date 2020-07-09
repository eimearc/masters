#ifndef COMMAND
#define COMMAND

#include <vulkan/vulkan.h>
#include "device.h"
#include "util.h"
#include <vector>

class Commands
{
    public:
    Commands()=default;
    Commands(
        const Device &device,
        const uint32_t &swapchainSize,
        const uint32_t &numThreads
    );

    void destroy();

    std::vector<VkCommandPool> m_commandPools;
    std::vector<VkCommandBuffer> m_primaryCommandBuffers;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffers;

    private:
    VkDevice m_device;
    // size_t m_numThreads;
};

#endif