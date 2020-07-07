#ifndef SYNC
#define SYNC

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "swapchain.h"

class Sync
{
    public:
    Sync()=default;
    Sync(const Device &device, const Swapchain &swapchain);
    
    void destroy();

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_fencesInFlight;
    std::vector<VkFence> m_imagesInFlight;

    private:
    VkDevice m_device;
};

#endif