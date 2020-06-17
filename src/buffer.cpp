#include "evulkan_core.h"

#include <glm/gtx/string_cast.hpp>

void evk::Instance::createIndexBuffer(const std::vector<Index> &indices)
{
    m_indices=indices;
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

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
        &m_indexBuffer, &m_indexBufferMemory);

    copyBuffer(m_device, m_commandPools[0], m_graphicsQueue, stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void evk::Instance::createUniformBufferObject()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    const size_t &size = m_swapChainImages.size();
    m_uniformBuffers.resize(size);
    m_uniformBuffersMemory.resize(size);

    for (size_t i = 0; i < size; i++)
    {
        createBuffer(
            m_device,
            m_physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &(m_uniformBuffers)[i], &(m_uniformBuffersMemory)[i]);
    }

    addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
    addWriteDescriptorSetBuffer(m_uniformBuffers, 0, sizeof(UniformBufferObject), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

void evk::Instance::createVertexBuffer(const std::vector<Vertex> &vertices)
{
    const VkDeviceSize wholeBufferSize = sizeof(vertices[0]) * vertices.size();
    const int numVertsEach = vertices.size()/m_numThreads;

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
        &m_vertexBuffer,
        &m_vertexBufferMemory);

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
        vkCmdCopyBuffer(commandBuffers[i], buffers[i], m_vertexBuffer, 1, &copyRegion);

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

void evk::Instance::updateUniformBuffer(const EVkUniformBufferUpdateInfo *pUpdateInfo)
{
    static int counter = 0;
    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f)*counter, glm::vec3(0.0f,0.0f,1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), pUpdateInfo->swapchainExtent.width / (float) pUpdateInfo->swapchainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    std::vector<VkDeviceMemory> &uniformBufferMemory = *pUpdateInfo->pUniformBufferMemory;
    vkMapMemory(m_device, uniformBufferMemory[pUpdateInfo->currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_device, uniformBufferMemory[pUpdateInfo->currentImage]);
    counter++;
}