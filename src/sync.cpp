#include "device.h"

#include <iostream>

Device::Sync::Sync(
    const VkDevice &device,
    const uint32_t &swapchainSize)
{
    m_device = device;
    size_t maxFramesInFlight = swapchainSize; // TODO: Is this correct?
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

Device::Sync::Sync(Sync &&other) noexcept
{
    *this=std::move(other);
}

Device::Sync& Device::Sync::operator=(Sync &&other) noexcept
{
    if (*this==other) return *this;
    m_device=other.m_device;
    other.m_device=VK_NULL_HANDLE;
    m_imageAvailableSemaphores=other.m_imageAvailableSemaphores;
    other.m_imageAvailableSemaphores.resize(0);
    m_renderFinishedSemaphores=other.m_renderFinishedSemaphores;
    other.m_renderFinishedSemaphores.resize(0);
    m_fencesInFlight=other.m_fencesInFlight;
    other.m_fencesInFlight.resize(0);
    m_imagesInFlight=other.m_imagesInFlight;
    other.m_imagesInFlight.resize(0);
    return *this;
}

bool Device::Sync::operator==(const Sync &other)
{
    bool result = true;
    result &= (m_device==other.m_device);
    result &= std::equal(
        m_fencesInFlight.begin(), m_fencesInFlight.end(),
        other.m_fencesInFlight.begin()
    );
    result &= std::equal(
        m_imageAvailableSemaphores.begin(), m_imageAvailableSemaphores.end(),
        other.m_imageAvailableSemaphores.begin()
    );
    result &= std::equal(
        m_imagesInFlight.begin(), m_imagesInFlight.end(),
        other.m_imagesInFlight.begin()
    );
    result &= std::equal(
        m_renderFinishedSemaphores.begin(), m_renderFinishedSemaphores.end(),
        other.m_renderFinishedSemaphores.begin()
    );
    return result;
}

Device::Sync::~Sync() noexcept
{
    for (auto &s : m_renderFinishedSemaphores) vkDestroySemaphore(m_device, s, nullptr);
    for (auto &s : m_imageAvailableSemaphores) vkDestroySemaphore(m_device, s, nullptr);
    for (auto &f : m_fencesInFlight) vkDestroyFence(m_device, f, nullptr);
}