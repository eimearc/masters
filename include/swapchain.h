#ifndef EVK_SWAPCHAIN
#define EVK_SWAPCHAIN

#include "attachment.h"
#include "device.h"
#include "pass.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Swapchain
{
    public:
    Swapchain()=default;
    Swapchain(
        const Device &device,
        const uint32_t swapchainSize,
        Attachment &framebuffer
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

class Framebuffer
{
    public:
    Framebuffer()=default;
    Framebuffer(
        const Device &device,
        const Renderpass &renderpass,
        const Swapchain &swapchain
    );

    void destroy();

    std::vector<VkFramebuffer> m_framebuffers;

    private:
    VkDevice m_device;
};

#endif