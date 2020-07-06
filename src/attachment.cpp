#include "attachment.h"

Attachment::Attachment(const Device &device, uint32_t index, uint32_t swapchainSize)
{
    m_device = device.m_device;
    m_index = index;
    m_swapchainSize = swapchainSize;

    m_inputReference = {index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    m_colorReference = {index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    m_depthReference = {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    m_images.resize(m_swapchainSize, VK_NULL_HANDLE);
    m_imageViews.resize(m_swapchainSize, VK_NULL_HANDLE);
    m_imageMemories.resize(m_swapchainSize, VK_NULL_HANDLE);
}

void Attachment::setFramebufferAttachment()
{
    m_description.flags = 0;
    m_description.format = VK_FORMAT_B8G8R8A8_SRGB;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    framebuffer=true;
}

void Attachment::setColorAttachment(const VkExtent2D &extent, const Device &device)
{
    VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

    m_description.flags = 0;
    m_description.format = format;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    for (int i = 0; i < m_swapchainSize; ++i)
    {
        createImage(
            device.m_device, device.m_physicalDevice,
            extent, format, tiling, usage, properties,
            &m_images[i], &m_imageMemories[i]);

        createImageView(
            device.m_device, m_images[i], format,
            aspectMask, &m_imageViews[i]);
    }
}

void Attachment::setDepthAttachment(
    const VkExtent2D &extent,
    const Device &device)
{
    m_description.flags = 0;
    m_description.format = device.m_depthFormat;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkFormat format = device.m_depthFormat;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    for (int i = 0; i < m_swapchainSize; ++i)
    {
        createImage(
            device.m_device, device.m_physicalDevice,
            extent, format, tiling, usage, properties,
            &m_images[i], &m_imageMemories[i]);

        createImageView(
            device.m_device, m_images[i], format,
            aspectMask, &m_imageViews[i]);
    }
}

void Attachment::destroy()
{
    for (auto &iv : m_imageViews)
    {
        if (iv != VK_NULL_HANDLE) vkDestroyImageView(m_device, iv, nullptr);
    }
    if (!framebuffer) // TODO change this - have separate attachment for framebuffer.
    {
        for (auto &i : m_images)
        {
            if (i != VK_NULL_HANDLE) vkDestroyImage(m_device, i, nullptr);
        }
        for (auto &m : m_imageMemories)
        {
            if (m != VK_NULL_HANDLE) vkFreeMemory(m_device, m, nullptr);
        }
    }
}