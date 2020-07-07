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
    Attachment(const Device &device, uint32_t index);
    void setFramebufferAttachment();
    void setColorAttachment(
        const VkExtent2D &extent,
        const Device &device);
    void setDepthAttachment(
        const VkExtent2D &extent,
        const Device &device);
    void destroy();

    uint32_t m_index;
    VkAttachmentDescription m_description;

    VkAttachmentReference m_inputReference;
    VkAttachmentReference m_colorReference;
    VkAttachmentReference m_depthReference;

    VkImage m_image=VK_NULL_HANDLE;
    VkImageView m_imageView=VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory=VK_NULL_HANDLE;

    VkClearValue m_clearValue;

    private:
    VkDevice m_device;

    void createFramebuffer();
};

#endif