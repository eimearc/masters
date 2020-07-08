#ifndef TEXTURE
#define TEXTURE

#include <vulkan/vulkan.h>
#include "command.h"
#include "device.h"
#include "evulkan_util.h"

class Texture
{
    public:
    Texture()=default;
    Texture(
        const std::string &fileName,
        const Device &device,
        const Commands &commands
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