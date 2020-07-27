#ifndef EVK_TEXTURE_H_
#define EVK_TEXTURE_H_

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Texture
{
    public:
    Texture()=default;
    Texture(
        const std::string &fileName,
        const Device &device
    );

    void destroy();

    VkDevice m_device;
    VkImage m_image;
    VkImageView m_imageView;
    VkDeviceMemory m_memory;
    VkSampler m_imageSampler;
    VkDescriptorImageInfo m_imageDescriptor;

    bool m_allocated=false;

    private:
    void transitionImageLayout(
        const Device &device,
        VkCommandPool commandPool,
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );
    void copyBufferToImage(
        const Device &device,
        VkCommandPool commandPool,
        VkBuffer buffer,
        VkImage image,
        VkExtent2D extent
    );
};

#endif