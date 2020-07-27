#ifndef EVK_TEXTURE_H_
#define EVK_TEXTURE_H_

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Texture
{
    public:
    Texture()=default;
    Texture(const Texture&)=delete;
    Texture& operator=(const Texture&)=delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;
    ~Texture() noexcept;

    bool operator==(const Texture&) const noexcept;

    Texture(
        const Device &device,
        const std::string &fileName
    );

    VkSampler sampler() const { return m_imageSampler; };
    VkImageView view() const { return m_imageView; };

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

    VkDevice m_device=VK_NULL_HANDLE;
    VkImage m_image=VK_NULL_HANDLE;
    VkSampler m_imageSampler=VK_NULL_HANDLE;
    VkImageView m_imageView=VK_NULL_HANDLE;
    VkDeviceMemory m_memory=VK_NULL_HANDLE;
};

#endif