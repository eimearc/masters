#include "sync.h"

#include "device.h"
#include <iostream>

Sync::Sync(const Device &device, const Swapchain &swapchain)
{
    const size_t swapchainSize = swapchain.m_images.size();
    size_t maxFramesInFlight=swapchain.m_images.size();
    m_device = device.m_device;
    m_imageAvailableSemaphores.resize(maxFramesInFlight);
    m_renderFinishedSemaphores.resize(maxFramesInFlight);
    m_fencesInFlight.resize(maxFramesInFlight);
    m_imagesInFlight.resize(swapchainSize, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &(m_imageAvailableSemaphores)[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &(m_renderFinishedSemaphores)[i]) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &(m_fencesInFlight)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores for a frame.");
        }
    }
}

void Sync::destroy()
{
    for (auto &s : m_renderFinishedSemaphores) vkDestroySemaphore(m_device, s, nullptr);
    for (auto &s : m_imageAvailableSemaphores) vkDestroySemaphore(m_device, s, nullptr);
    for (auto &f : m_fencesInFlight) vkDestroyFence(m_device, f, nullptr);
}