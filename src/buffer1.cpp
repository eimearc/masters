#include "buffer.h"

Buffer::Buffer(size_t swapchainSize, const Device &device)
{
    m_swapchainSize = swapchainSize;
    m_device = device.m_device;
    m_physicalDevice = device.m_physicalDevice;
    m_buffers.resize(m_swapchainSize);
    m_bufferMemories.resize(m_swapchainSize);
    m_queue = device.m_graphicsQueue;
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
            m_device,
            m_physicalDevice,
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

void Buffer::setIndexBuffer(const VkDeviceSize &bufferSize0, const void *indexBuffer, const size_t numElements, VkCommandPool commandPool)
{
    // m_indices=indices; Change this.
    m_bufferSize = bufferSize0*numElements;

    m_buffers.resize(1);
    m_bufferMemories.resize(1);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, m_bufferSize, 0, &data);
    memcpy(data, indexBuffer, m_bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffers[0], &m_bufferMemories[0]);

    copyBuffer(commandPool, m_queue, stagingBuffer, m_buffers[0]);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Buffer::copyBuffer(
    VkCommandPool commandPool,
    VkQueue queue,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer)
{
    VkCommandBuffer commandBuffer;
    beginSingleTimeCommands(m_device, commandPool, &commandBuffer);

    VkBufferCopy copyRegion = {};
    copyRegion.size = m_bufferSize;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(m_device, queue, commandPool, commandBuffer);
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