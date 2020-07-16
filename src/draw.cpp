#include "draw.h"

// TODO: Make part of device.
void executeDrawCommands(
    Device &device,
    const std::vector<Pipeline> &pipelines)
{
    static size_t currentFrame=0;
    auto frameFences = device.frameFences();
    auto imageFences = device.imageFences();
    const auto &imageSemaphores = device.imageSempahores();
    const auto &renderSemaphores = device.renderSempahores();

    auto &frameFence = frameFences[currentFrame];

    vkWaitForFences(device.device(), 1, &frameFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device.device(), device.swapchain(), UINT64_MAX,
        imageSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    auto &imageFence = imageFences[imageIndex];

    if (currentFrame != imageIndex) throw std::runtime_error("failed to find imageIndex and currentFrame equal"); // TODO: Remove.

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (imageFence != VK_NULL_HANDLE)
    {
        vkWaitForFences(device.device(), 1, &(imageFence), VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    imageFence = frameFence;

    const auto &primaryCommandBuffers = device.primaryCommandBuffers();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &primaryCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {(renderSemaphores)[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.device(), 1, &frameFence);

    const auto &graphicsQueue = device.graphicsQueue();
    const auto &presentQueue = device.presentQueue();

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {device.swapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image.");
    }

    vkQueueWaitIdle(presentQueue);

    currentFrame = ((currentFrame)+1) % device.swapchainSize();
}

void recordDrawCommands(
    Device &device,
    const Buffer &indexBuffer,
    const Buffer &vertexBuffer,
    std::vector<Pipeline> &pipelines,
    const Renderpass &renderpass,
    Framebuffer &framebuffers)
{
    const auto &primaryCommandBuffers = device.primaryCommandBuffers();
    auto secondaryCommandBuffers = device.secondaryCommandBuffers();
    const auto &commandPools = device.commandPools();

    for (auto &p : pipelines) p.setup(device);

    const size_t numIndicesEach=indexBuffer.m_numElements/device.m_numThreads;
    framebuffers = {device, renderpass};
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderpass.m_renderPass;
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = device.extent();
    renderPassInfo.clearValueCount = renderpass.m_clearValues.size();
    renderPassInfo.pClearValues = renderpass.m_clearValues.data();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    for (size_t imageIndex = 0; imageIndex < device.swapchainSize(); ++imageIndex)
    {
        auto &primaryCommandBuffer = primaryCommandBuffers[imageIndex];

        renderPassInfo.framebuffer = framebuffers.m_framebuffers[imageIndex];
        vkBeginCommandBuffer(primaryCommandBuffer, &beginInfo);

        for (size_t pass = 0; pass < renderpass.m_subpasses.size(); ++pass)
        {
            auto descriptor = *(pipelines[pass].m_descriptor);
            auto &descriptorSets = descriptor.m_descriptorSets;
            auto &pipeline = pipelines[pass].m_pipeline;
            auto &pipelineLayout = pipelines[pass].m_layout;

            if (pass == 0 )
                vkCmdBeginRenderPass(
                    primaryCommandBuffer,
                    &renderPassInfo,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            else
                vkCmdNextSubpass(
                    primaryCommandBuffer,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            auto createDrawCommands =[&](int i)
            {
                auto &secondaryCommandBuffer = secondaryCommandBuffers[i];

                size_t numIndices=numIndicesEach;
                size_t indexOffset=numIndicesEach*i;
                if (i==(device.m_numThreads-1)) numIndices = indexBuffer.m_numElements-(i*numIndicesEach);

                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPools[i];
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                allocInfo.commandBufferCount = 1;
                if (vkAllocateCommandBuffers(device.device(), &allocInfo, &secondaryCommandBuffer) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to allocate command buffers.");
                }

                VkCommandBufferInheritanceInfo inheritanceInfo = {};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass.m_renderPass;
                inheritanceInfo.framebuffer = framebuffers.m_framebuffers[imageIndex];
                inheritanceInfo.subpass=pass;

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo;

                if (vkBeginCommandBuffer(secondaryCommandBuffer, &beginInfo) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to begin recording command buffer.");
                }

                vkCmdBindPipeline(secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                VkDeviceSize offsets[] = {0};

                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, &vertexBuffer.m_buffer, offsets);
                vkCmdBindIndexBuffer(secondaryCommandBuffer, indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(
                    secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                    0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
                vkCmdDrawIndexed(secondaryCommandBuffer, numIndices, 1, indexOffset, 0, 0);

                if (vkEndCommandBuffer(secondaryCommandBuffer) != VK_SUCCESS)
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
            vkCmdExecuteCommands(primaryCommandBuffer, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
        }

        vkCmdEndRenderPass(primaryCommandBuffer);

        if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not end primaryCommandBuffer.");   
        }
    }
}