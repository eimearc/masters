#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "attachment.h"
#include "device.h"
#include "pass.h"

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

class Framebuffer
{
    public:
    Framebuffer()=default;
    Framebuffer(
        const Device &device,
        const std::vector<Attachment> &attachments,
        const Renderpass &renderpass,
        const Swapchain &swapchain
    );

    void destroy();

    std::vector<VkFramebuffer> m_framebuffers;

    private:
    VkDevice m_device;
};

#endif