#include "evulkan_core.h"

void evk::Instance::draw(const std::vector<Pipeline> &pipelines)
{
    static size_t currentFrame=0;
    vkWaitForFences(m_device, 1, &m_fencesInFlight[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device, m_swapChain, UINT64_MAX,
        m_imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (currentFrame != imageIndex) throw std::runtime_error("failed to find imageIndex and currentFrame equal"); // TODO: Remove.

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

void evk::Instance::createDrawCommands(
    const Buffer &indexBuffer,
    const Buffer &vertexBuffer,
    const std::vector<Descriptor*> descriptors,
    const std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass)
{
    m_primaryCommandBuffers.resize(m_maxFramesInFlight);
    m_secondaryCommandBuffers.resize(m_numThreads);

    const size_t numIndicesEach=indexBuffer.m_numElements/m_numThreads;

    for (int frame = 0; frame < m_maxFramesInFlight; ++frame)
    {
        // Create primary command buffer.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPools[0];
        allocInfo.commandBufferCount = 1;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(m_device, &allocInfo, &m_primaryCommandBuffers[frame]);

        std::array<VkClearValue, 3> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderpass.m_renderPass;
        renderPassInfo.framebuffer = m_framebuffers[frame];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = m_swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(
            m_primaryCommandBuffers[frame],
            &beginInfo);

        for (size_t pass = 0; pass < renderpass.m_subpasses.size(); ++pass)
        {
            const Descriptor &descriptor = *descriptors[pass];

            if (pass == 0 )
                vkCmdBeginRenderPass(
                    m_primaryCommandBuffers[frame],
                    &renderPassInfo,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            else
                vkCmdNextSubpass(
                    m_primaryCommandBuffers[frame],
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            auto createDrawCommands =[&](int i)
            {
                size_t numIndices=numIndicesEach;
                size_t indexOffset=numIndicesEach*i;
                if (i==(m_numThreads-1)) numIndices = indexBuffer.m_numElements-(i*numIndicesEach);
                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = m_commandPools[i];
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                allocInfo.commandBufferCount = 1;

                if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_secondaryCommandBuffers[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to allocate command buffers.");
                }

                VkCommandBufferInheritanceInfo inheritanceInfo = {};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass.m_renderPass;
                inheritanceInfo.framebuffer = m_framebuffers[frame];
                inheritanceInfo.subpass=pass;

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo;

                if (vkBeginCommandBuffer(m_secondaryCommandBuffers[i], &beginInfo) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to begin recording command buffer.");
                }

                vkCmdBindPipeline(m_secondaryCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[pass].m_pipeline);

                VkDeviceSize offsets[] = {0};

                vkCmdBindVertexBuffers(m_secondaryCommandBuffers[i], 0, 1, vertexBuffer.m_buffers.data(), offsets);
                vkCmdBindIndexBuffer(m_secondaryCommandBuffers[i], indexBuffer.m_buffers[0], 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(
                    m_secondaryCommandBuffers[i],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelines[pass].m_layout, 0, 1, &(descriptor.m_descriptorSets[frame]), 0, nullptr); // TODO: why is m_descriptorSets 0?
                vkCmdDrawIndexed(m_secondaryCommandBuffers[i], numIndices, 1, indexOffset, 0, 0);

                if (vkEndCommandBuffer(m_secondaryCommandBuffers[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to record command buffer.");
                }
            };

            int counter = 0;
            for (auto &t: m_threadPool.threads)
            {
                t->addJob(std::bind(createDrawCommands,counter++));
            }
            m_threadPool.wait();
            vkCmdExecuteCommands(m_primaryCommandBuffers[frame], m_secondaryCommandBuffers.size(), m_secondaryCommandBuffers.data());
        }

        vkCmdEndRenderPass(m_primaryCommandBuffers[frame]);

        if (vkEndCommandBuffer(m_primaryCommandBuffers[frame]) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not end primaryCommandBuffer.");   
        }
    }
}