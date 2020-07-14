#include "attachment.h"

#include "swapchain.h"

Attachment::Attachment(
    const Device &device,
    uint32_t index,
    const Type &type)
{
    m_device = device.m_device;
    m_index = index;

    m_inputReference = {index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    m_colorReference = {index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    m_depthReference = {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    switch(type)
    {
        case Type::FRAMEBUFFER:
            setFramebufferAttachment();
            break;
        case Type::COLOR:
            setColorAttachment(device);
            break;
        case Type::DEPTH:
            setDepthAttachment(device);
            break;
    }
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

    m_clearValue.color = {0.0f,0.0f,0.0f,1.0f};
}

void Attachment::setColorAttachment(const Device &device)
{
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    m_description.flags = 0;
    m_description.format = format;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    createImage(
        device.m_device, device.m_physicalDevice,
        device.m_swapchain.m_extent, format, tiling, usage, properties,
        &m_image, &m_imageMemory);

    createImageView(
        device.m_device, m_image, format,
        aspectMask, &m_imageView);

    m_clearValue.color = {0.0f,0.0f,0.0f,1.0f};
}

void Attachment::setDepthAttachment(const Device &device)
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

    createImage(
        device.m_device, device.m_physicalDevice,
        device.m_swapchain.m_extent, format, tiling, usage, properties,
        &m_image, &m_imageMemory);

    createImageView(
        device.m_device, m_image, format,
        aspectMask, &m_imageView);

    m_clearValue.depthStencil = {1.0f,1};
}

void Attachment::destroy()
{
    if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_imageView, nullptr);
    if (m_image != VK_NULL_HANDLE) vkDestroyImage(m_device, m_image, nullptr);
    if (m_imageMemory != VK_NULL_HANDLE) vkFreeMemory(m_device, m_imageMemory, nullptr);
}