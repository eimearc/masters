#include "evulkan_core.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "util.h"

void createSecondaryCommandBuffers(
    VkDevice device,
    const EVkCommandPoolCreateInfo *pCommandPoolCreateInfo,
    const VkCommandPool *pCommandPool,
    VkCommandBuffer *pCommandBuffer,
    size_t indexOffset,
    size_t numIndices,
    VkDescriptorSet *pDescriptorSet,
    const EVkCommandBuffersCreateInfo *pCreateInfo
)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *pCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, pCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers.");
    }

    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = pCreateInfo->renderPass;
    inheritanceInfo.framebuffer = pCreateInfo->framebuffer;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    if (vkBeginCommandBuffer(*pCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer.");
    }
    
    vkCmdBindPipeline(*pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->graphicsPipeline);

    VkBuffer vertexBuffers[] = {pCreateInfo->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*pCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*pCommandBuffer, pCreateInfo->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->pipelineLayout, 0, 1, &(pCreateInfo->descriptorSets[0]), 0, nullptr);

    vkCmdDrawIndexed(*pCommandBuffer, numIndices, 1, indexOffset, 0, 0);

    if (vkEndCommandBuffer(*pCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer.");
    }
}

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<VkCommandBuffer> *pCommandBuffers,
    const std::vector<VkCommandPool> *pCommandPools,
    ThreadPool &threadpool
)
{
    size_t NUM_THREADS=FLAGS_num_threads;

    // Create primary command buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pCreateInfo->commandPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(device, &allocInfo, pPrimaryCommandBuffer);

    const VkCommandBuffer &primaryCommandBuffer = *pPrimaryCommandBuffer;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pCreateInfo->renderPass;
    renderPassInfo.framebuffer = pCreateInfo->framebuffer;
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = pCreateInfo->swapchainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(
        primaryCommandBuffer,
        &beginInfo);

    vkCmdBeginRenderPass(
        primaryCommandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    EVkCommandPoolCreateInfo poolCreateInfo = pCreateInfo->poolCreateInfo;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    std::vector<std::thread> workers;
    const std::vector<VkCommandPool> &commandPools=*pCommandPools;
    std::vector<VkCommandBuffer> &commandBuffers=*pCommandBuffers;
    const std::vector<uint32_t> &indices = pCreateInfo->indices;

    auto f =[&](int i)
    {
        size_t numIndices=indices.size()/NUM_THREADS;
        size_t indexOffset=numIndices*i;
        if (i==(FLAGS_num_threads-1))
        {
            numIndices = indices.size()-(i*numIndices);
        }
        createSecondaryCommandBuffers(device,
            &poolCreateInfo,&commandPools[i],&commandBuffers[i],indexOffset,numIndices,nullptr,pCreateInfo);
    };

    int i = 0;
    for (auto &t: threadpool.threads)
    {
        t->addJob(std::bind(f,i++));
    }
    threadpool.wait();

	vkCmdExecuteCommands(primaryCommandBuffer, commandBuffers.size(), commandBuffers.data());

    vkCmdEndRenderPass(primaryCommandBuffer);
    if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not end primaryCommandBuffer.");   
    }
}