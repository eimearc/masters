#ifndef EVK_TEXTURE_H_
#define EVK_TEXTURE_H_

#include "device.h"
#include "util.h"
#include <vulkan/vulkan.h>

namespace evk {

/**
 * @class Texture
 * @brief A Texture is used to pass data via an image to a Shader.
 * 
 * Textures are used in shading. They can pass information such as color,
 * normal and specularity.
 * 
 * A Texture is loaded from a file and is then attached to a descriptor.
 * 
 * @example
 * Texture texture(device, "viking_room.png");
 * descriptor.addTextureSampler(1, texture, Shader::Stage::FRAGMENT);
 **/
class Texture
{
    public:
    Texture()=default;
    Texture(const Texture&)=delete;
    Texture& operator=(const Texture&)=delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;
    ~Texture() noexcept;

    /**
     * Creates a Texture.
     * @param[in] device the Device used to create the Texture.
     * @param[in] fileName the file where the Texture is located.
     **/
    Texture(
        const Device &device,
        const std::string &fileName
    );

    bool operator==(const Texture&) const noexcept;
    bool operator!=(const Texture&) const noexcept;

    private:
    void copyBufferToImage(
        const Device &device,
        VkCommandPool commandPool,
        VkBuffer buffer,
        VkImage image,
        VkExtent2D extent
    );
    void transitionImageLayout(
        const Device &device,
        VkCommandPool commandPool,
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );
    void reset() noexcept;

    VkSampler sampler() const { return m_imageSampler; };
    VkImageView view() const { return m_imageView; };

    VkDevice m_device=VK_NULL_HANDLE;
    VkImage m_image=VK_NULL_HANDLE;
    VkSampler m_imageSampler=VK_NULL_HANDLE;
    VkImageView m_imageView=VK_NULL_HANDLE;
    VkDeviceMemory m_memory=VK_NULL_HANDLE;

    friend class Descriptor;

    // Tests.
    FRIEND_TEST(TextureTest,ctor);
    FRIEND_TEST(TextureTest,get);
    FRIEND_TEST(TextureTest,move);
};

} // namespace evk

#endif