#ifndef ATTACHMENT
#define ATTACHMENT

#include <vulkan/vulkan.h>
#include <vector>
#include "evulkan_util.h"
#include "device.h"

class Attachment
{
    public:
    Attachment()=default;
    Attachment(const Device &device, uint32_t index, uint32_t swapchainSize);
    void setFramebufferAttachment();
    void setColorAttachment(const VkExtent2D &extent, const Device &device);
    void setDepthAttachment(
        const VkExtent2D &extent,
        const Device &device);
    void destroy();

    uint32_t m_index;
    VkAttachmentDescription m_description;

    VkAttachmentReference m_inputReference;
    VkAttachmentReference m_colorReference;
    VkAttachmentReference m_depthReference;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkDeviceMemory> m_imageMemories;

    private:
    void createFramebuffer();
    VkDevice m_device;
    bool framebuffer=false;

    size_t m_swapchainSize;
};

#endif