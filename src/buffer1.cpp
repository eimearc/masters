#include "buffer.h"

Buffer::Buffer(size_t swapchainSize, VkDevice device, VkPhysicalDevice physicalDevice)
{
    m_swapchainSize = swapchainSize;
    m_device = device;
    m_physicalDevice = physicalDevice;
    m_buffers.resize(m_swapchainSize);
    m_bufferMemories.resize(m_swapchainSize);
}

void Buffer::destroy()
{
    for (int i = 0; i < m_swapchainSize; ++i)
    {
        vkDestroyBuffer(m_device, m_buffers[i], nullptr);
        vkFreeMemory(m_device, m_bufferMemories[i], nullptr);
    }
}

void Buffer::setBuffer(const VkDeviceSize &bufferSize)
{
    m_bufferSize=bufferSize;
    for (size_t i = 0; i < m_swapchainSize; ++i)
    {
        createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_buffers[i], &m_bufferMemories[i]);
    }
}

void Buffer::updateBuffer(const void *srcBuffer)
{
    for (auto &memory : m_bufferMemories)
    {
        void* dstBuffer;
        vkMapMemory(m_device, memory, 0, m_bufferSize, 0, &dstBuffer);
        memcpy(dstBuffer, srcBuffer, m_bufferSize);
        vkUnmapMemory(m_device, memory);
    }
}

void Buffer::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, pBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, *pBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, pBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(m_device, *pBuffer, *pBufferMemory, 0);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1<<i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type.");
}