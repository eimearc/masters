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
    Attachment(uint32_t index, uint32_t swapchainSize);
    void setFramebufferAttachment();
    void setColorAttachment(const VkExtent2D &extent, const Device &device);
    void setDepthAttachment(
        const VkExtent2D &extent,
        const VkFormat &depthFormat,
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
    // void createImage(
    //     const VkDevice &device,
    //     const VkPhysicalDevice &physicalDevice,
    //     const VkExtent2D &extent,
    //     const VkFormat &format,
    //     const VkImageTiling &tiling,
    //     const VkImageUsageFlags &usage,
    //     const VkMemoryPropertyFlags &properties,
    //     VkImage *pImage,
    //     VkDeviceMemory *pImageMemory
    // );
    // void createImageView(
    //     const VkDevice &device,
    //     const VkImage &image,
    //     const VkFormat &format,
    //     const VkImageAspectFlags &aspectMask,
    //     VkImageView *pImageView
    // );

    size_t m_swapchainSize;
};

#endif