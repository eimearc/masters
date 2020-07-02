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
    m_bufferSize = bufferSize0*numElements;
    m_numElements = numElements;
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

void Buffer::setVertexBuffer(const VkDeviceSize wholeBufferSize, const VkDeviceSize elementSize, const void *vertexBuffer, const size_t numElements, Device &device, std::vector<VkCommandPool> commandPools)
{
    m_numElements = numElements;
    const int numVertsEach = numElements/device.m_numThreads;

    std::vector<VkCommandBuffer> commandBuffers(1);
    std::vector<VkBuffer> buffers(1);
    std::vector<VkDeviceMemory> bufferMemory(1);

    // Use a device-local buffer as the actual vertex buffer.
    createBuffer(
        device.m_device,
        device.m_physicalDevice,
        wholeBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffers[0],
        &m_bufferMemories[0]);

    auto &stagingBuffer = buffers[0];
    auto &stagingBufferMemory = bufferMemory[0];

    // Use a host visible buffer as a staging buffer.
    createBuffer(
        device.m_device,
        device.m_physicalDevice,
        wholeBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &buffers[0], &bufferMemory[0]);

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *data;
    vkMapMemory(device.m_device, bufferMemory[0], 0, wholeBufferSize, 0, &data);
    memcpy(data, vertexBuffer, wholeBufferSize);
    vkUnmapMemory(device.m_device, bufferMemory[0]);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPools[0];
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device.m_device, &allocInfo, &commandBuffers[0]);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = wholeBufferSize;
    copyRegion.dstOffset = 0;
    vkCmdCopyBuffer(commandBuffers[0], buffers[0], m_buffers[0], 1, &copyRegion);

    vkEndCommandBuffer(commandBuffers[0]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(device.m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.m_graphicsQueue);

    for (size_t i = 0; i<device.m_numThreads; ++i)
    {
        vkFreeCommandBuffers(device.m_device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(device.m_device, buffers[i], nullptr);
        vkFreeMemory(device.m_device, bufferMemory[i], nullptr);
    }
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