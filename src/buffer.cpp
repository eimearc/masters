#include "evulkan.h"

#include <glm/gtx/string_cast.hpp>

#include "evulkan_util.h"

void EVulkan::setupVertices()
{
    int i=0;
    const size_t numVerts = 8;
    Vertex vertex = {{}, {1,0,0}};
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint32_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color=cube.color;
            vertices.push_back(vertex);
        }
        for(size_t j = 0; j<ind.size(); ++j)
        {
            indices.push_back(ind[j]+i*numVerts);
        }
        ++i;
    }
}

void EVulkan::createGrid()
{
    float gridSize = 2.0f;
    float cubeSize = (gridSize/NUM_CUBES)*0.5;
    grid = Grid(gridSize, cubeSize, NUM_CUBES);
    setupVertices();
}

void evkCreateIndexBuffer(
    VkDevice device,
    const EVkIndexBufferCreateInfo *pCreateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
)
{
    VkDeviceSize bufferSize = sizeof(pCreateInfo->indices[0]) * pCreateInfo->indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        device,
        pCreateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, pCreateInfo->indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        device,
        pCreateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        pBuffer, pBufferMemory);

    copyBuffer(device, pCreateInfo->commandPool, pCreateInfo->queue, stagingBuffer, *pBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void evkCreateUniformBuffers(
    VkDevice device,
    const EVkUniformBufferCreateInfo *pCreateInfo,
    std::vector<VkBuffer> *pBuffer,
    std::vector<VkDeviceMemory> *pBufferMemory
)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    const size_t &size = pCreateInfo->swapchainImages.size();
    pBuffer->resize(size);
    pBufferMemory->resize(size);

    for (size_t i = 0; i < size; i++)
    {
        createBuffer(
            device,
            pCreateInfo->physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &(*pBuffer)[i], &(*pBufferMemory)[i]);
    }
}

void evkCreateVertexBuffer(
    VkDevice device,
    const EVkVertexBufferCreateInfo *pUpdateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory,    
    ThreadPool &threadpool)
{
    size_t NUM_THREADS=FLAGS_num_threads;
    const VkDeviceSize wholeBufferSize = sizeof((pUpdateInfo->pVertices)[0]) * pUpdateInfo->pVertices->size();
    const VkQueue queue = pUpdateInfo->graphicsQueue;
    std::vector<Vertex> &verts = pUpdateInfo->pVertices[0];
    const int num_verts = verts.size();
    int num_verts_each = num_verts/NUM_THREADS;
    size_t threadBufferSize = wholeBufferSize/NUM_THREADS;

    std::vector<std::thread> workers;
    auto &commandPools = pUpdateInfo->commandPools;
    std::vector<VkCommandBuffer> commandBuffers(NUM_THREADS);
    std::vector<VkBuffer> buffers(NUM_THREADS);
    std::vector<VkDeviceMemory> bufferMemory(NUM_THREADS);

    // Use a device-local buffer as the actual vertex buffer.
    createBuffer(
        device,
        pUpdateInfo->physicalDevice,
        wholeBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        pBuffer,
        pBufferMemory);

    auto f = [&](int i)
    {
        int vertsOffset = num_verts_each*i;
        size_t bufferOffset=(num_verts_each*sizeof(verts[0]))*i;
        if (i==(FLAGS_num_threads-1))
        {
            num_verts_each = verts.size()-(i*num_verts_each);
        }
        size_t numVerts=num_verts_each;
        size_t bufferSize = numVerts*sizeof(verts[0]);
        auto &stagingBuffer = buffers[i];
        auto &stagingBufferMemory = bufferMemory[i];

        // Use a host visible buffer as a staging buffer.
        createBuffer(
            device,
            pUpdateInfo->physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &buffers[i], &bufferMemory[i]);

        // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
        // accessible memory.
        void *data;
        vkMapMemory(device, bufferMemory[i], 0, bufferSize, 0, &data);
        memcpy(data, &verts[vertsOffset], bufferSize);
        vkUnmapMemory(device, bufferMemory[i]);

        // Copy the vertex data from the staging buffer to the device-local buffer.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPools[i];
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.size = bufferSize;
        copyRegion.dstOffset = bufferOffset;
        vkCmdCopyBuffer(commandBuffers[i], buffers[i], *pBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffers[i]);
    };

    int i = 0;
    for (auto &t: threadpool.threads)
    {
        t->addJob(std::bind(f,i++));
    }
    threadpool.wait();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    for (size_t i = 0; i<NUM_THREADS; ++i)
    {
        vkFreeCommandBuffers(device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(device, buffers[i], nullptr);
        vkFreeMemory(device, bufferMemory[i], nullptr);
    }
}

void evkUpdateUniformBuffer(VkDevice device, const EVkUniformBufferUpdateInfo *pUpdateInfo)
{
    static int counter = 0;
    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f)*counter, glm::vec3(1.0f,0.0f,0.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), pUpdateInfo->swapchainExtent.width / (float) pUpdateInfo->swapchainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    std::vector<VkDeviceMemory> &uniformBufferMemory = *pUpdateInfo->pUniformBufferMemory;
    vkMapMemory(device, uniformBufferMemory[pUpdateInfo->currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBufferMemory[pUpdateInfo->currentImage]);
    counter++;
}