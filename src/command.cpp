#include "command.h"

Commands::Commands(
    const Device &device,
    const uint32_t &swapchainSize,
    const uint32_t &numThreads
)
{
    m_device = device.m_device;
    m_commandPools.resize(numThreads);
    for (auto &commandPool : m_commandPools)
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device.m_physicalDevice, device.m_surface);
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(device.m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool.");
        }
    }

    m_primaryCommandBuffers.resize(swapchainSize);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPools[0];
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    for (auto &cb : m_primaryCommandBuffers) vkAllocateCommandBuffers(m_device, &allocInfo, &cb);

    m_secondaryCommandBuffers.resize(numThreads);
}

void Commands::destroy()
{
    for (int i = 0; i < m_commandPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, m_commandPools[i], 1, &m_secondaryCommandBuffers[i]);
        vkDestroyCommandPool(m_device, m_commandPools[i], nullptr);
    }
}