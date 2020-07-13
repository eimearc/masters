#ifndef EVK_SWAPCHAIN
#define EVK_SWAPCHAIN

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Swapchain
{
    public:
    Swapchain()=default;
    Swapchain(
        const Device &device,
        const uint32_t swapchainSize
    );

    void destroy();

    VkDevice m_device;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkFormat m_format;
    VkExtent2D m_extent;

    private:

};

#endif