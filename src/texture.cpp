#include "texture.h"

#include "evk_assert.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace evk {

Texture::Texture(Texture &&other) noexcept
{
    *this=std::move(other);
}

Texture& Texture::operator=(Texture &&other) noexcept
{
    if (*this==other) return *this;
    m_device=std::move(other.m_device);
    m_image=std::move(other.m_image);
    m_imageSampler=std::move(other.m_imageSampler);
    m_imageView=std::move(other.m_imageView);
    m_memory=std::move(other.m_memory);
    other.reset();
    return *this;
}

void Texture::reset() noexcept
{
    m_device=VK_NULL_HANDLE;
    m_image=VK_NULL_HANDLE;
    m_imageSampler=VK_NULL_HANDLE;
    m_imageView=VK_NULL_HANDLE;
    m_memory=VK_NULL_HANDLE;
}

Texture::~Texture() noexcept
{
    if (m_imageSampler!=VK_NULL_HANDLE)
        vkDestroySampler(m_device, m_imageSampler, nullptr);
    if (m_imageView!=VK_NULL_HANDLE)
        vkDestroyImageView(m_device, m_imageView, nullptr);
    if (m_image!=VK_NULL_HANDLE)
        vkDestroyImage(m_device, m_image, nullptr);
    if (m_memory!=VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_memory, nullptr);
}

bool Texture::operator==(const Texture &other) const noexcept
{
    if (m_device!=other.m_device) return false;
    if (m_image!=other.m_image) return false;
    if (m_imageSampler!=other.m_imageSampler) return false;
    if (m_imageView!=other.m_imageView) return false;
    if (m_memory!=other.m_memory) return false;
    return true;
}

bool Texture::operator!=(const Texture &other) const noexcept
{
    return !(*this==other);
}

Texture::Texture(
    const Device &device,
    const std::string &fileName
)
{
    m_device = device.device();
    const auto &commandPools = device.commandPools();
    auto &commandPool = commandPools[0];

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha
    );

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    internal::createBuffer(
        device.device(), device.physicalDevice(), imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device.device(), stagingBufferMemory);

    stbi_image_free(pixels);

    VkExtent2D extent = {
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)
    };
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT;
    VkMemoryPropertyFlagBits properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    internal::createImage(
        device.device(), device.physicalDevice(), extent, format, tiling, usage,
        properties, &m_image, &m_memory
    );

    transitionImageLayout(
        device, commandPool, m_image, format, Transition::INITIAL
    );
    copyBufferToImage(device, commandPool, stagingBuffer, m_image, extent);
    transitionImageLayout(
        device, commandPool, m_image, format, Transition::SHADER
    );

    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
    vkFreeMemory(device.device(), stagingBufferMemory, nullptr);

    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    internal::createImageView(
        device.device(), m_image, format, aspectFlags, &m_imageView
    );

    // Create sampler.
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    auto result = vkCreateSampler(
        device.device(), &samplerInfo, nullptr, &m_imageSampler
    );
    EVK_ASSERT(result,"failed to create texture sampler\n");
}

void Texture::transitionImageLayout(
    const Device &device,
    VkCommandPool commandPool,
    VkImage image,
    VkFormat format,
    Transition transition
) noexcept
{
    VkCommandBuffer commandBuffer;
    internal::beginSingleTimeCommands(
        device.device(), commandPool, &commandBuffer
    );

    VkImageLayout oldLayout{};
    VkImageLayout newLayout{};

    switch(transition)
    {
        case Transition::INITIAL:
            oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            break;
        case Transition::SHADER:
            oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO?
    barrier.dstAccessMask = 0; // TODO?

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    switch(transition)
    {
        case Transition::INITIAL:
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case Transition::SHADER:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
    }

    vkCmdPipelineBarrier(
        commandBuffer, sourceStage, destinationStage, 0, 0, nullptr,
        0, nullptr, 1, &barrier
    );

    internal::endSingleTimeCommands(
        device.device(), device.graphicsQueue(), commandPool, commandBuffer
    );
}

void Texture::copyBufferToImage(
    const Device &device,
    VkCommandPool commandPool,
    VkBuffer buffer,
    VkImage image,
    VkExtent2D extent
) noexcept
{
    VkCommandBuffer commandBuffer;
    internal::beginSingleTimeCommands(
        device.device(), commandPool, &commandBuffer
    );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {extent.width, extent.height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region
    );

    internal::endSingleTimeCommands(
        device.device(), device.graphicsQueue(), commandPool, commandBuffer
    );
}

} // namespace evk