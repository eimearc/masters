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

void evkDrawFrame(
    VkDevice device,
    const EVkDrawFrameInfo *pDrawInfo,
    size_t *pCurrentFrame,
    std::vector<VkFence> *pImagesInFlight,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    VkCommandBuffer *pPrimaryCommandBuffer)
{
    const std::vector<VkFence> &inFlightFences = *(pDrawInfo->pInFlightFences);
    const std::vector<VkSemaphore> &imageAvailableSemaphores = *(pDrawInfo->pImageAvailableSemaphores);
    std::vector<VkFence> &imagesInFlight = *(pImagesInFlight);

    vkWaitForFences(device, 1, &inFlightFences[*pCurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device, pDrawInfo->swapchain, UINT64_MAX,
        imageAvailableSemaphores[*pCurrentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device, 1, &(imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    imagesInFlight[imageIndex] = inFlightFences[*pCurrentFrame];

    // Update the uniform buffers.
    EVkUniformBufferUpdateInfo updateInfo = {};
    updateInfo.currentImage = imageIndex;
    updateInfo.swapchainExtent = pDrawInfo->swapchainExtent;
    updateInfo.pUniformBufferMemory = pDrawInfo->pUniformBufferMemory;
    evkUpdateUniformBuffer(device, &updateInfo);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[*pCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = pPrimaryCommandBuffer;

    VkSemaphore signalSemaphores[] = {(*pRenderFinishedSemaphores)[*pCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[*pCurrentFrame]);

    if (vkQueueSubmit(pDrawInfo->graphicsQueue, 1, &submitInfo, inFlightFences[*pCurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {pDrawInfo->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    if (vkQueuePresentKHR(pDrawInfo->presentQueue, &presentInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image.");
    }

    vkQueueWaitIdle(pDrawInfo->presentQueue);

    *pCurrentFrame = ((*pCurrentFrame)+1) % pDrawInfo->maxFramesInFlight;
}