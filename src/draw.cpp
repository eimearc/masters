#include "device.h"

#include "buffer.h"
#include "pipeline.h"

namespace evk {

void Device::draw()
{
    static size_t currentFrame=0;
    static int previousImageIndex=-1;
    const auto &device = this->device();
    auto &frameFences = this->frameFences();
    auto &imageFences = this->imageFences();
    const auto &imageSemaphores = imageSempahores();
    const auto &renderSemaphores = renderSempahores();
    auto &frameFence = frameFences[currentFrame];

    vkWaitForFences(device, 1, &frameFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device, m_swapchain->m_swapchain, UINT64_MAX, imageSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        resizeWindow();
        currentFrame = 0;
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    auto &imageFence = imageFences[imageIndex];

    // if (currentFrame != imageIndex)
        // throw std::runtime_error("failed to find imageIndex and currentFrame equal"); // TODO: Remove.

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (imageFence != VK_NULL_HANDLE)
    {
        vkWaitForFences(device, 1, &(imageFence), VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    imageFence = frameFence;

    const auto &primaryCommandBuffers = this->primaryCommandBuffers();

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

    vkResetFences(device, 1, &frameFence);

    if (vkQueueSubmit(graphicsQueue(), 1, &submitInfo, frameFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {swapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    const auto &presentQueue = this->presentQueue();
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resizeRequired)
    {
        m_resizeRequired = false;
        resizeWindow();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    else currentFrame = ((currentFrame)+1) % swapchainSize();

    vkQueueWaitIdle(presentQueue);

    // currentFrame = ((currentFrame)+1) % swapchainSize();
}

void Device::finalize(
    Buffer &indexBuffer,
    Buffer &vertexBuffer,
    std::vector<Pipeline*> &pipelines)
{
    auto renderpass=pipelines[0]->renderpass();
    m_framebuffer = std::make_unique<Framebuffer>(
        *this, *renderpass
    );

    m_indexBuffer=&indexBuffer;
    m_vertexBuffer=&vertexBuffer;
    m_pipelines=pipelines;

    record();
}

void Device::record()
{
    const auto &primaryCommandBuffers = this->primaryCommandBuffers();
    auto secondaryCommandBuffers = this->secondaryCommandBuffers();
    const auto &commandPools = this->commandPools();
    const auto numThreads = this->numThreads();
    const size_t numIndicesEach=m_indexBuffer->numElements()/this->numThreads();
    const auto &framebuffers = this->framebuffers();
    auto renderpass=m_pipelines[0]->renderpass(); // TODO: Check if this is suitable. Only one renderpass supported. Singleton?
    const auto &clearValues = renderpass->clearValues();
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderpass->renderpass();
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = this->extent();
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    const auto &numSubpasses = renderpass->subpasses().size();

    for (size_t imageIndex = 0; imageIndex < this->swapchainSize(); ++imageIndex)
    {
        auto &primaryCommandBuffer = primaryCommandBuffers[imageIndex];

        renderPassInfo.framebuffer = framebuffers[imageIndex];
        vkBeginCommandBuffer(primaryCommandBuffer, &beginInfo);

        for (size_t pass = 0; pass < numSubpasses; ++pass)
        {
            const auto &pipeline = m_pipelines[pass]->pipeline();
            const auto &pipelineLayout = m_pipelines[pass]->layout();
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
                if (i==(numThreads-1)) numIndices = m_indexBuffer->numElements()-(i*numIndicesEach);

                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPools[i];
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                allocInfo.commandBufferCount = 1;
                if (vkAllocateCommandBuffers(this->device(), &allocInfo, &secondaryCommandBuffer) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to allocate command buffers.");
                }

                VkCommandBufferInheritanceInfo inheritanceInfo = {};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass->renderpass();
                inheritanceInfo.framebuffer = framebuffers[imageIndex];
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

                auto vBuffer = m_vertexBuffer->buffer();
                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, &vBuffer, offsets);
                vkCmdBindIndexBuffer(secondaryCommandBuffer, m_indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);
                if (m_pipelines[pass]->descriptor()!=nullptr)
                {
                    const auto &descriptorSets = m_pipelines[pass]->descriptor()->sets();
                    vkCmdBindDescriptorSets(
                        secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                        0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
                }
                vkCmdDrawIndexed(secondaryCommandBuffer, numIndices, 1, indexOffset, 0, 0);

                if (vkEndCommandBuffer(secondaryCommandBuffer) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to record command buffer.");
                }
            };

            int counter = 0;
            for (auto &t: this->threads())
            {
                t->addJob(std::bind(createDrawCommands,counter++));
            }
            this->wait();
            vkCmdExecuteCommands(primaryCommandBuffer, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
        }

        vkCmdEndRenderPass(primaryCommandBuffer);

        if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not end primaryCommandBuffer.");   
        }
    }
}

void Device::resizeWindow()
{
    vkDeviceWaitIdle(device());
    m_swapchain->recreate();
    m_framebuffer->recreate();
    for (auto &p: m_pipelines) p->recreate();
    record();
}

} // namespace evk