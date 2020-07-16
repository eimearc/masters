#ifndef EVK_SYNC
#define EVK_SYNC

#include "util.h"
#include <vulkan/vulkan.h>

class Sync
{
    public:
    Sync()=default;
    Sync(const VkDevice &device, const uint32_t &swapchainSize);
    
    void destroy();

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_fencesInFlight;
    std::vector<VkFence> m_imagesInFlight;

    private:
    VkDevice m_device;
};

#endif