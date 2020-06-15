#include "evulkan_core.h"

void evk::Instance::draw()
{
    static size_t currentFrame=0;
    vkWaitForFences(m_device, 1, &m_fencesInFlight[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device, m_swapChain, UINT64_MAX,
        m_imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_device, 1, &(m_imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    m_imagesInFlight[imageIndex] = m_fencesInFlight[currentFrame];

    // Update the uniform buffers.
    EVkUniformBufferUpdateInfo updateInfo = {};
    updateInfo.currentImage = imageIndex;
    updateInfo.swapchainExtent = m_swapChainExtent;
    updateInfo.pUniformBufferMemory = &m_uniformBuffersMemory;
    evkUpdateUniformBuffer(m_device, &updateInfo);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_primaryCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {(m_renderFinishedSemaphores)[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_device, 1, &m_fencesInFlight[currentFrame]);

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_fencesInFlight[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    if (vkQueuePresentKHR(m_presentQueue, &presentInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image.");
    }

    vkQueueWaitIdle(m_presentQueue);

    currentFrame = ((currentFrame)+1) % m_maxFramesInFlight;
}

void evk::Instance::createDrawCommands(const std::vector<uint32_t> &indices)
{
    size_t NUM_THREADS=FLAGS_num_threads;

    m_primaryCommandBuffers.resize(m_maxFramesInFlight);
    m_secondaryCommandBuffers.resize(NUM_THREADS);
    auto &secondaryCommandBuffers = m_secondaryCommandBuffers;

    for (int index = 0; index < m_maxFramesInFlight; ++index)
    {
        // Create primary command buffer.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPools[0];
        allocInfo.commandBufferCount = 1;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(m_device, &allocInfo, &m_primaryCommandBuffers[index]);

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_framebuffers[index];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = m_swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(
            m_primaryCommandBuffers[index],
            &beginInfo);

        vkCmdBeginRenderPass(
            m_primaryCommandBuffers[index],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        // EVkCommandPoolCreateInfo poolCreateInfo{};
        // poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        std::vector<std::thread> workers;
        const std::vector<VkCommandPool> &commandPools=m_commandPools;

        auto f =[&](int i)
        {
            size_t numIndices=indices.size()/NUM_THREADS;
            size_t indexOffset=numIndices*i;
            if (i==(FLAGS_num_threads-1))
            {
                numIndices = indices.size()-(i*numIndices);
            }

            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPools[i];
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(m_device, &allocInfo, &secondaryCommandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate command buffers.");
            }

            VkCommandBufferInheritanceInfo inheritanceInfo = {};
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritanceInfo.renderPass = m_renderPass;
            inheritanceInfo.framebuffer = m_framebuffers[index];

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = &inheritanceInfo;

            if (vkBeginCommandBuffer(secondaryCommandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer.");
            }
            
            vkCmdBindPipeline(secondaryCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

            VkBuffer vertexBuffers[] = {m_vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(secondaryCommandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(secondaryCommandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(secondaryCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipelineLayout, 0, 1, &(m_descriptorSets[0]), 0, nullptr);

            vkCmdDrawIndexed(secondaryCommandBuffers[i], numIndices, 1, indexOffset, 0, 0);

            if (vkEndCommandBuffer(secondaryCommandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer.");
            }
        };

        int i = 0;
        for (auto &t: m_threadPool.threads)
        {
            t->addJob(std::bind(f,i++));
        }
        m_threadPool.wait();

        vkCmdExecuteCommands(m_primaryCommandBuffers[index], secondaryCommandBuffers.size(), secondaryCommandBuffers.data());

        vkCmdEndRenderPass(m_primaryCommandBuffers[index]);

        if (vkEndCommandBuffer(m_primaryCommandBuffers[index]) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not end primaryCommandBuffer.");   
        }
    }
}