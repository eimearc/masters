#ifndef EVK_COMMAND
#define EVK_COMMAND

#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

class Device;

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
};

#endif