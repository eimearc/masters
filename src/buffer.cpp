#include "evulkan_core.h"

#include <glm/gtx/string_cast.hpp>

void evk::Instance::createIndexBuffer(const std::vector<Index> &indices)
{
    m_indices=indices;
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    // TODO: make thread safe.
    size_t index = m_buffers.size();
    m_bufferMap.insert(std::pair<std::string,BufferInfo>("INDEX",{index,1}));
    m_buffers.push_back(VkBuffer{});
    m_bufferMemories.push_back(VkDeviceMemory{});

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        m_device,
        m_physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(
        m_device,
        m_physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffers[index], &m_bufferMemories[index]);

    copyBuffer(m_device, m_commandPools[0], m_graphicsQueue, stagingBuffer, m_buffers[index], bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void evk::Instance::updateBufferObject(std::string name, VkDeviceSize bufferSize, void *srcBuffer, size_t imageIndex)
{
    size_t bufferIndex = m_bufferMap[name].index;
    void* dstBuffer;
    vkMapMemory(m_device, m_bufferMemories[bufferIndex+imageIndex], 0, bufferSize, 0, &dstBuffer);
    memcpy(dstBuffer, srcBuffer, bufferSize);
    vkUnmapMemory(m_device, m_bufferMemories[bufferIndex+imageIndex]);
}

void evk::Instance::createBufferObject(std::string name, VkDeviceSize bufferSize)
{
    const size_t &size = m_swapChainImages.size();

    size_t index = m_buffers.size();
    m_bufferMap.insert(std::pair<std::string,BufferInfo>(name,{index,size}));

    for (size_t i = 0; i < size; i++)
    {
        m_buffers.push_back(VkBuffer{});
        m_bufferMemories.push_back(VkDeviceMemory{});
        createBuffer(
            m_device,
            m_physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_buffers[index+i], &m_bufferMemories[index+i]);
    }

    // addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
    // addWriteDescriptorSetBuffer(m_buffers, 0, bufferSize, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,index);
}

void evk::Instance::createVertexBuffer(const std::vector<Vertex> &vertices)
{
    const VkDeviceSize wholeBufferSize = sizeof(vertices[0]) * vertices.size();
    const int numVertsEach = vertices.size()/m_numThreads;

    size_t index = m_buffers.size();
    m_bufferMap.insert(std::pair<std::string, BufferInfo>("VERTEX", {index,1}));
    m_buffers.push_back(VkBuffer{});
    m_bufferMemories.push_back(VkDeviceMemory{});

    std::vector<VkCommandBuffer> commandBuffers(m_numThreads);
    std::vector<VkBuffer> buffers(m_numThreads);
    std::vector<VkDeviceMemory> bufferMemory(m_numThreads);

    // Use a device-local buffer as the actual vertex buffer.
    createBuffer(
        m_device,
        m_physicalDevice,
        wholeBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffers[index],
        &m_bufferMemories[index]);

    auto copyVerts = [&](int i)
    {
        int numVerts=numVertsEach;
        int vertsOffset = numVertsEach*i;
        size_t bufferOffset=(numVertsEach*sizeof(vertices[0]))*i;
        if (i==(m_numThreads-1)) numVerts = vertices.size()-(i*numVertsEach);
        size_t bufferSize = numVerts*sizeof(vertices[0]);
        auto &stagingBuffer = buffers[i];
        auto &stagingBufferMemory = bufferMemory[i];

        // Use a host visible buffer as a staging buffer.
        createBuffer(
            m_device,
            m_physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &buffers[i], &bufferMemory[i]);

        // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
        // accessible memory.
        void *data;
        vkMapMemory(m_device, bufferMemory[i], 0, bufferSize, 0, &data);
        memcpy(data, &vertices[vertsOffset], bufferSize);
        vkUnmapMemory(m_device, bufferMemory[i]);

        // Copy the vertex data from the staging buffer to the device-local buffer.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPools[i];
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffers[i]);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.size = bufferSize;
        copyRegion.dstOffset = bufferOffset;
        vkCmdCopyBuffer(commandBuffers[i], buffers[i], m_buffers[index], 1, &copyRegion);

        vkEndCommandBuffer(commandBuffers[i]);
    };

    int counter = 0;
    for (auto &t: m_threadPool.threads)
    {
        t->addJob(std::bind(copyVerts,counter++));
    }
    m_threadPool.wait();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    for (size_t i = 0; i<m_numThreads; ++i)
    {
        vkFreeCommandBuffers(m_device, m_commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(m_device, buffers[i], nullptr);
        vkFreeMemory(m_device, bufferMemory[i], nullptr);
    }
}