#include "buffer.h"

StaticBuffer::StaticBuffer(
    const Device &device,
    Commands &commands,
    void *data,
    const VkDeviceSize &elementSize,
    const size_t numElements,
    const Type &type
)
{
    m_device = device.m_device;
    m_physicalDevice = device.m_physicalDevice;
    m_queue = device.m_graphicsQueue;
    m_numThreads = device.m_numThreads;

    m_data=data;
    m_elementSize=elementSize;
    m_numElements=numElements;
    m_bufferSize = m_numElements * m_elementSize;
}

void StaticBuffer::finalizeIndex(Device &device, Commands &commands)
{
    std::vector<VkCommandPool> &commandPools = commands.m_commandPools;
    const int num_elements_each = m_numElements/m_numThreads;
    m_bufferSize = m_elementSize*m_numElements;

    std::vector<VkCommandBuffer> commandBuffers(m_numThreads);
    std::vector<VkBuffer> buffers(m_numThreads);
    std::vector<VkDeviceMemory> bufferMemory(m_numThreads);

    // Create the device-local index buffer.
    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffer, &m_bufferMemory);

    auto setupCopyFunction = [&](int thread)
    {
        int element_offset = num_elements_each*thread;
        
        int num_elements=num_elements_each;
        if (thread==(m_numThreads-1)) 
            num_elements = m_numElements-(thread*num_elements_each);

        auto &staging_buffer = buffers[thread];
        auto &staging_buffer_memory = bufferMemory[thread];

        copyData(
            m_device, m_physicalDevice, commandPools[thread], commandBuffers[thread], 
            m_buffer, staging_buffer, staging_buffer_memory, num_elements,
            m_elementSize, element_offset
        );
    };

    int counter = 0;
    for (auto &t: device.m_threadPool.threads)
    {
        t->addJob(std::bind(setupCopyFunction,counter++));
    }
    device.m_threadPool.wait();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);

    for (size_t i = 0; i<m_numThreads; ++i)
    {
        vkFreeCommandBuffers(m_device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(m_device, buffers[i], nullptr);
        vkFreeMemory(m_device, bufferMemory[i], nullptr);
    }

    // VkBuffer stagingBuffer;
    // VkDeviceMemory stagingBufferMemory;
    // createBuffer(
    //     m_device,
    //     m_physicalDevice,
    //     m_bufferSize,
    //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //     &stagingBuffer, &stagingBufferMemory);

    // void *data;
    // vkMapMemory(m_device, stagingBufferMemory, 0, m_bufferSize, 0, &data);
    // memcpy(data, m_data, m_bufferSize);
    // vkUnmapMemory(m_device, stagingBufferMemory);

    // copyBuffer(commandPool, m_queue, stagingBuffer, m_buffer);

    // vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    // vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void StaticBuffer::copyData(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkCommandPool command_pool,
    VkCommandBuffer &command_buffer,
    VkBuffer device_buffer,
    VkBuffer &staging_buffer,
    VkDeviceMemory &staging_buffer_memory,
    const size_t num_elements,
    const VkDeviceSize element_size,
    const size_t element_offset
)
{
    const size_t buffer_offset = element_offset*element_size;
    const size_t buffer_size = num_elements*element_size;

    // Use a host visible buffer as a staging buffer.
    createBuffer(
        device,
        physicalDevice,
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_memory);

    // TODO: Check if below is valid.
    const unsigned char* bytePtr = reinterpret_cast<const unsigned char*>(m_data);
    auto offset = element_offset*element_size;

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *data;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, bytePtr+offset, buffer_size); // Need to offset vertices.
    vkUnmapMemory(device, staging_buffer_memory);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &allocInfo, &command_buffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = buffer_size;
    copyRegion.dstOffset = buffer_offset;
    vkCmdCopyBuffer(command_buffer, staging_buffer, device_buffer, 1, &copyRegion);

    vkEndCommandBuffer(command_buffer);
}

void StaticBuffer::finalizeVertex(Device &device, Commands &commands)
{
    std::vector<VkCommandPool> &commandPools = commands.m_commandPools;
    const int numVertsEach = m_numElements/m_numThreads;

    std::vector<VkCommandBuffer> commandBuffers(m_numThreads);
    std::vector<VkBuffer> buffers(m_numThreads);
    std::vector<VkDeviceMemory> bufferMemory(m_numThreads);

    // Use a device-local buffer as the actual vertex buffer.
    createBuffer(
        m_device,
        m_physicalDevice,
        m_bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffer,
        &m_bufferMemory);

    auto copyVertices = [&](int i)
    {
        int numVerts=numVertsEach;
        int vertsOffset = numVertsEach*i;
        size_t bufferOffset=(numVertsEach*m_elementSize)*i;
        if (i==(m_numThreads-1)) numVerts = m_numElements-(i*numVertsEach);
        size_t bufferSize = numVerts*m_elementSize;

        auto &stagingBuffer = buffers[i];
        auto &stagingBufferMemory = bufferMemory[i];

        copyData(
            m_device, m_physicalDevice, commandPools[i], commandBuffers[i], 
            m_buffer, stagingBuffer, stagingBufferMemory, numVerts,
            m_elementSize, vertsOffset
        );
    };

    int counter = 0;
    for (auto &t: device.m_threadPool.threads)
    {
        t->addJob(std::bind(copyVertices,counter++));
    }
    device.m_threadPool.wait();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);

    for (size_t i = 0; i<m_numThreads; ++i)
    {
        vkFreeCommandBuffers(m_device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(m_device, buffers[i], nullptr);
        vkFreeMemory(m_device, bufferMemory[i], nullptr);
    }
}

void Buffer::destroy()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_bufferMemory, nullptr);
}

DynamicBuffer::DynamicBuffer(
    const Device &device,
    const VkDeviceSize &bufferSize)
{
    m_device = device.m_device;
    m_bufferSize=bufferSize;
    createBuffer(
        m_device,
        device.m_physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &m_buffer, &m_bufferMemory);
}

void DynamicBuffer::update(const void *srcBuffer)
{
    void* dstBuffer;
    vkMapMemory(m_device, m_bufferMemory, 0, m_bufferSize, 0, &dstBuffer);
    memcpy(dstBuffer, srcBuffer, m_bufferSize);
    vkUnmapMemory(m_device, m_bufferMemory);
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