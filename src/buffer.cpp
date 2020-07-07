#include "buffer.h"

Buffer::Buffer(const Device &device)
{
    m_device = device.m_device;
    m_physicalDevice = device.m_physicalDevice;
    m_queue = device.m_graphicsQueue;
    m_numThreads = device.m_numThreads;
}

void Buffer::destroy()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_bufferMemory, nullptr);
}

void Buffer::setBuffer(const VkDeviceSize &bufferSize)
{
    m_bufferSize=bufferSize;
    createBuffer(
        m_device,
        m_physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &m_buffer, &m_bufferMemory);
}

void Buffer::updateBuffer(const void *srcBuffer)
{
    void* dstBuffer;
    vkMapMemory(m_device, m_bufferMemory, 0, m_bufferSize, 0, &dstBuffer);
    memcpy(dstBuffer, srcBuffer, m_bufferSize);
    vkUnmapMemory(m_device, m_bufferMemory);
}

void Buffer::setIndexBuffer(
    const void *indices,
    const VkDeviceSize &elementSize,
    const size_t numElements,
    Commands &commands)
{
    VkCommandPool &commandPool = commands.m_commandPools[0];

    m_bufferSize = elementSize*numElements;
    m_numElements = numElements;

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
    memcpy(data, indices, m_bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffer, &m_bufferMemory);

    copyBuffer(commandPool, m_queue, stagingBuffer, m_buffer);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Buffer::setVertexBuffer(
    const void *vertices,
    const VkDeviceSize &elementSize,
    const size_t numElements,
    Commands &commands)
{
    std::vector<VkCommandPool> &commandPools = commands.m_commandPools;
    m_numElements = numElements;
    const int numVertsEach = numElements/m_numThreads;
    m_bufferSize = numElements * elementSize;

    std::vector<VkCommandBuffer> commandBuffers(1);
    std::vector<VkBuffer> buffers(1);
    std::vector<VkDeviceMemory> bufferMemory(1);

    // Use a device-local buffer as the actual vertex buffer.
    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffer,
        &m_bufferMemory);

    auto &stagingBuffer = buffers;
    auto &stagingBufferMemory = bufferMemory;

    // Use a host visible buffer as a staging buffer.
    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &buffers[0], &bufferMemory[0]);

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *data;
    vkMapMemory(m_device, bufferMemory[0], 0, m_bufferSize, 0, &data);
    memcpy(data, vertices, m_bufferSize);
    vkUnmapMemory(m_device, bufferMemory[0]);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPools[0];
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffers[0]);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = m_bufferSize;
    copyRegion.dstOffset = 0;
    vkCmdCopyBuffer(commandBuffers[0], buffers[0], m_buffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffers[0]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);

    // for (size_t i = 0; i<commandPools.size(); ++i)
    for (size_t i = 0; i<1; ++i) // Change this back in when multithreading works.
    {
        vkFreeCommandBuffers(m_device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(m_device, buffers[i], nullptr);
        vkFreeMemory(m_device, bufferMemory[i], nullptr);
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