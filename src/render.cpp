#include "evulkan_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

void evkCreateSyncObjects(
    VkDevice device,
    const EVkSyncObjectsCreateInfo *pCreateInfo,
    std::vector<VkSemaphore> *pImageAvailableSemaphores,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    std::vector<VkFence> *pFencesInFlight,
    std::vector<VkFence> *pImagesInFlight
)
{
    pImageAvailableSemaphores->resize(pCreateInfo->maxFramesInFlight);
    pRenderFinishedSemaphores->resize(pCreateInfo->maxFramesInFlight);
    pFencesInFlight->resize(pCreateInfo->maxFramesInFlight);
    pImagesInFlight->resize(pCreateInfo->swapchainSize, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < pCreateInfo->maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &(*pImageAvailableSemaphores)[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &(*pRenderFinishedSemaphores)[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &(*pFencesInFlight)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores for a frame.");
        }
    }
}