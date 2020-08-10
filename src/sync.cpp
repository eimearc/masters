#include "device.h"

#include "evk_assert.h"
#include <iostream>

namespace evk {

Device::Sync::Sync(
    const VkDevice &device,
    const uint32_t &swapchainSize)
{
    m_device = device;
    size_t maxFramesInFlight = swapchainSize; // TODO: Is this correct?
    m_imageAvailableSemaphores.resize(maxFramesInFlight);
    m_renderFinishedSemaphores.resize(maxFramesInFlight);
    m_fencesInFlight.resize(maxFramesInFlight, VK_NULL_HANDLE);
    m_imagesInFlight.resize(swapchainSize, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        EVK_ASSERT(
            vkCreateSemaphore(
                m_device, &semaphoreInfo, nullptr,
                &(m_imageAvailableSemaphores)[i]),
            "failed to create image available semaphore\n"
        );
        EVK_ASSERT(
            vkCreateSemaphore(
                m_device, &semaphoreInfo, nullptr,
                &(m_renderFinishedSemaphores)[i]),
            "failed to create render finished semaphore\n"
        );
        EVK_ASSERT(
            vkCreateFence(
                m_device, &fenceInfo, nullptr, &(m_fencesInFlight)[i]),
            "failed to create fence\n"
        );
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
    m_imageAvailableSemaphores=other.m_imageAvailableSemaphores;
    m_renderFinishedSemaphores=other.m_renderFinishedSemaphores;
    m_fencesInFlight=other.m_fencesInFlight;
    m_imagesInFlight=other.m_imagesInFlight;
    other.reset();
    return *this;
}

void Device::Sync::reset() noexcept
{
    m_device=VK_NULL_HANDLE;
    m_imageAvailableSemaphores.resize(0);
    m_renderFinishedSemaphores.resize(0);
    m_fencesInFlight.resize(0);
    m_imagesInFlight.resize(0);
}

bool Device::Sync::operator==(const Sync &other) const noexcept
{
    if (m_device!=other.m_device) return false;
    if (m_fencesInFlight.size()!=other.m_fencesInFlight.size()) return false;
    if (!std::equal(
            m_fencesInFlight.begin(), m_fencesInFlight.end(),
            other.m_fencesInFlight.begin()
        ))
        return false;
    if (m_imageAvailableSemaphores.size()
            !=other.m_imageAvailableSemaphores.size())
        return false;
    if (!std::equal(
            m_imageAvailableSemaphores.begin(),
            m_imageAvailableSemaphores.end(),
            other.m_imageAvailableSemaphores.begin()
        ))
        return false;
    if (m_imagesInFlight.size()!=other.m_imagesInFlight.size()) return false;
    if (!std::equal(
            m_imagesInFlight.begin(), m_imagesInFlight.end(),
            other.m_imagesInFlight.begin()
        ))
        return false;
    if (m_renderFinishedSemaphores.size()
            !=other.m_renderFinishedSemaphores.size())
        return false;
    if (!std::equal(
            m_renderFinishedSemaphores.begin(),
            m_renderFinishedSemaphores.end(),
            other.m_renderFinishedSemaphores.begin()
        ))
        return false;
    return true;
}

bool Device::Sync::operator!=(const Sync &other) const noexcept
{
    return !(*this==other);
}

Device::Sync::~Sync() noexcept
{
    for (auto &s : m_renderFinishedSemaphores)
        vkDestroySemaphore(m_device, s, nullptr);
    for (auto &s : m_imageAvailableSemaphores)
        vkDestroySemaphore(m_device, s, nullptr);
    for (auto &f : m_fencesInFlight) vkDestroyFence(m_device, f, nullptr);
}

} // namespace evk