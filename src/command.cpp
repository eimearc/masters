#include "device.h"

namespace evk {

Device::Commands::Commands(
    const VkDevice &device,
    const VkPhysicalDevice &physicalDevice,
    const VkSurfaceKHR &surface,
    const uint32_t &swapchainSize,
    const uint32_t &numThreads
)
{
    m_device = device;
    m_commandPools.resize(numThreads);
    for (auto &commandPool : m_commandPools)
    {
        auto queueFamilyIndices = internal::findQueueFamilies(
            physicalDevice, surface
        );
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
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

Device::Commands::Commands(Commands &&other) noexcept
{
    *this=std::move(other);
}

Device::Commands& Device::Commands::operator=(Commands &&other) noexcept
{
    if (*this==other) return *this;
    m_device = other.m_device;
    m_commandPools = other.m_commandPools;
    m_primaryCommandBuffers = other.m_primaryCommandBuffers;
    m_secondaryCommandBuffers = other.m_secondaryCommandBuffers;
    other.reset();
    return *this;
}

void Device::Commands::reset() noexcept
{
    m_device=VK_NULL_HANDLE;
    m_commandPools.resize(0);
    m_primaryCommandBuffers.resize(0);
    m_secondaryCommandBuffers.resize(0);  
}

bool Device::Commands::operator==(const Commands &other)
{
    bool result = true;
    result &= std::equal(
        m_commandPools.begin(), m_commandPools.end(), other.m_commandPools.begin()
    );
    result &= (m_device==other.m_device);
    result &= std::equal(
        m_primaryCommandBuffers.begin(), m_primaryCommandBuffers.end(),
        other.m_primaryCommandBuffers.begin()
    );
    result &= std::equal(
        m_secondaryCommandBuffers.begin(), m_secondaryCommandBuffers.end(),
        other.m_secondaryCommandBuffers.begin()
    );
    return result;
}

Device::Commands::~Commands() noexcept
{
    for (int i = 0; i < m_commandPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, m_commandPools[i], 1, &m_secondaryCommandBuffers[i]);
        vkDestroyCommandPool(m_device, m_commandPools[i], nullptr);
    }
}

} // namespace evk