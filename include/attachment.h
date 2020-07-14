#ifndef EVK_ATTACHMENT
#define EVK_ATTACHMENT

#include "device.h"
#include "swapchain.h"
#include <vulkan/vulkan.h>
#include "util.h"

class Attachment
{
    public:
    enum class Type{FRAMEBUFFER,COLOR,DEPTH};

    Attachment()=default;
    Attachment(
        const Device &device,
        uint32_t index,
        const Swapchain &swapchain,
        const Type &type
    );

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
    void setFramebufferAttachment();
    void setColorAttachment(
        const Device &device,
        const Swapchain &swapchain
    );
    void setDepthAttachment(
        const Device &device,
        const Swapchain &swapchain
    );
};

#endif