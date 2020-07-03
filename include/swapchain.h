#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "attachment.h"
#include "device.h"

class Swapchain
{
    public:
    Swapchain()=default;
    Swapchain(
        const uint32_t swapchainSize,
        Attachment &framebuffer,
        const Device &device
    );

    void destroy();

    VkDevice m_device;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    private:

};

#endif