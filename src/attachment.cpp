#include "attachment.h"

Attachment::Attachment(uint32_t index, uint32_t swapchainSize)
{
    m_index = index;
    m_swapchainSize = swapchainSize;

    m_inputReference = {index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    m_colorReference = {index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    m_depthReference = {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    m_images.resize(m_swapchainSize);
    m_imageViews.resize(m_swapchainSize);
    m_imageMemories.resize(m_swapchainSize);
}

void Attachment::setFramebufferAttachment()
{
    m_description.format = VK_FORMAT_B8G8R8A8_SRGB;
    m_description.flags = 0;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void Attachment::setColorAttachment(const VkExtent2D &extent, const Device &device)
{
    m_description.format = VK_FORMAT_R8G8B8A8_UNORM;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    for (int i = 0; i < m_swapchainSize; ++i)
    {
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        // evk::ImageCreateInfo createInfo={};
        // createInfo.width = extent.width;
        // createInfo.height = extent.height;
        // createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        // createInfo.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // createInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        createImage(
            device.m_device, device.m_physicalDevice,
            extent, format, tiling, usage, properties,
            &m_images[i], &m_imageMemories[i]);

        // evk::ImageViewCreateInfo imageViewCreateInfo={};
        // imageViewCreateInfo.image=m_images[i];
        // imageViewCreateInfo.format=VK_FORMAT_R8G8B8A8_UNORM;
        // imageViewCreateInfo.aspectFlags=VK_IMAGE_ASPECT_COLOR_BIT;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createImageView(device.m_device, m_images[i], format, aspectMask, &m_imageViews[i]);
    }
}

void Attachment::setDepthAttachment(
    const VkExtent2D &extent,
    const VkFormat &depthFormat,
    const Device &device)
{
    VkAttachmentDescription attachment = {};
    m_description.flags = 0;
    m_description.format = VK_FORMAT_D32_SFLOAT;
    m_description.samples = VK_SAMPLE_COUNT_1_BIT;
    m_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    for (int i = 0; i < m_swapchainSize; ++i)
    {
        VkFormat format = depthFormat;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        createImage(
            device.m_device, device.m_physicalDevice,
            extent, format, tiling, usage, properties,
            &m_images[i], &m_imageMemories[i]);

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        createImageView(device.m_device, m_images[i], format, aspectMask, &m_imageViews[i]);
    }
}

void Attachment::createImage(
    const VkDevice &device,
    const VkPhysicalDevice &physicalDevice,
    const VkExtent2D &extent,
    const VkFormat &format,
    const VkImageTiling &tiling,
    const VkImageUsageFlags &usage,
    const VkMemoryPropertyFlags &properties,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, pImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice,
        memRequirements.memoryTypeBits,
        properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, pImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory.");
    }
    vkBindImageMemory(device, *pImage, *pImageMemory, 0);
}

void Attachment::createImageView(
    const VkDevice &device,
    const VkImage &image,
    const VkFormat &format,
    const VkImageAspectFlags &aspectMask,
    VkImageView *pImageView)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, pImageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
}