#include "draw.h"

void executeDrawCommands(
    const Device &device,
    const std::vector<Pipeline> &pipelines,
    const Swapchain &swapchain,
    const Commands &commands,
    Sync &sync)
{
    static size_t currentFrame=0;
    vkWaitForFences(device.m_device, 1, &sync.m_fencesInFlight[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device.m_device, swapchain.m_swapChain, UINT64_MAX,
        sync.m_imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (currentFrame != imageIndex) throw std::runtime_error("failed to find imageIndex and currentFrame equal"); // TODO: Remove.

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (sync.m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device.m_device, 1, &(sync.m_imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    sync.m_imagesInFlight[imageIndex] = sync.m_fencesInFlight[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {sync.m_imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commands.m_primaryCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {(sync.m_renderFinishedSemaphores)[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.m_device, 1, &sync.m_fencesInFlight[currentFrame]);

    if (vkQueueSubmit(device.m_graphicsQueue, 1, &submitInfo, sync.m_fencesInFlight[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain.m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    if (vkQueuePresentKHR(device.m_presentQueue, &presentInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image.");
    }

    vkQueueWaitIdle(device.m_presentQueue);

    currentFrame = ((currentFrame)+1) % swapchain.m_swapChainImages.size();
}

void recordDrawCommands(
    Device &device,
    const Buffer &indexBuffer,
    const Buffer &vertexBuffer,
    const std::vector<Descriptor> &descriptors,
    const std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass,
    const Swapchain &swapchain,
    const Framebuffer &framebuffers,
    Commands &commands)
{
    const size_t numIndicesEach=indexBuffer.m_numElements/device.m_numThreads;

    for (int frame = 0; frame < swapchain.m_swapChainImages.size(); ++frame)
    {
        std::array<VkClearValue, 3> clearValues = {}; // Start here tomorrow.
        clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = {1.0f, 1.0f, 1.0f, 1.0f};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderpass.m_renderPass;
        renderPassInfo.framebuffer = framebuffers.m_framebuffers[frame];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = swapchain.m_swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(
            commands.m_primaryCommandBuffers[frame],
            &beginInfo);

        for (size_t pass = 0; pass < renderpass.m_subpasses.size(); ++pass)
        {
            const Descriptor &descriptor = descriptors[pass];

            if (pass == 0 )
                vkCmdBeginRenderPass(
                    commands.m_primaryCommandBuffers[frame],
                    &renderPassInfo,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            else
                vkCmdNextSubpass(
                    commands.m_primaryCommandBuffers[frame],
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            auto createDrawCommands =[&](int i)
            {
                size_t numIndices=numIndicesEach;
                size_t indexOffset=numIndicesEach*i;
                if (i==(device.m_numThreads-1)) numIndices = indexBuffer.m_numElements-(i*numIndicesEach);

                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commands.m_commandPools[i];
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                allocInfo.commandBufferCount = 1;
                if (vkAllocateCommandBuffers(device.m_device, &allocInfo, &commands.m_secondaryCommandBuffers[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to allocate command buffers.");
                }

                VkCommandBufferInheritanceInfo inheritanceInfo = {};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass.m_renderPass;
                inheritanceInfo.framebuffer = framebuffers.m_framebuffers[frame];
                inheritanceInfo.subpass=pass;

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo;

                if (vkBeginCommandBuffer(commands.m_secondaryCommandBuffers[i], &beginInfo) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to begin recording command buffer.");
                }

                vkCmdBindPipeline(commands.m_secondaryCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[pass].m_pipeline);

                VkDeviceSize offsets[] = {0};

                vkCmdBindVertexBuffers(commands.m_secondaryCommandBuffers[i], 0, 1, vertexBuffer.m_buffers.data(), offsets);
                vkCmdBindIndexBuffer(commands.m_secondaryCommandBuffers[i], indexBuffer.m_buffers[0], 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(
                    commands.m_secondaryCommandBuffers[i],
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelines[pass].m_layout, 0, descriptor.m_descriptorSets.size(), descriptor.m_descriptorSets.data(), 0, nullptr); // TODO: why is m_descriptorSets 0?
                vkCmdDrawIndexed(commands.m_secondaryCommandBuffers[i], numIndices, 1, indexOffset, 0, 0);

                if (vkEndCommandBuffer(commands.m_secondaryCommandBuffers[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to record command buffer.");
                }
            };

            int counter = 0;
            for (auto &t: device.m_threadPool.threads)
            {
                t->addJob(std::bind(createDrawCommands,counter++));
            }
            device.m_threadPool.wait();
            vkCmdExecuteCommands(commands.m_primaryCommandBuffers[frame], commands.m_secondaryCommandBuffers.size(), commands.m_secondaryCommandBuffers.data());
        }

        vkCmdEndRenderPass(commands.m_primaryCommandBuffers[frame]);

        if (vkEndCommandBuffer(commands.m_primaryCommandBuffers[frame]) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not end primaryCommandBuffer.");   
        }
    }
}