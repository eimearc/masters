#include "swapchain.h"

Swapchain::Swapchain(const uint32_t swapchainSize, Attachment &framebuffer, const Device &device)
{
    m_device = device.m_device;

    framebuffer = {device,0,swapchainSize};
    framebuffer.setFramebufferAttachment();

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device.m_physicalDevice, device.m_surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(device.m_window, swapChainSupport.capabilities);

    uint32_t imageCount = swapchainSize;
    if (imageCount < swapChainSupport.capabilities.minImageCount || imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        throw std::runtime_error("Please specify an image count within the swapchain capabilites.");
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device.m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(device.m_physicalDevice, device.m_surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device.m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(device.m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkFormat format = m_swapChainImageFormat;
    m_swapChainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        createImageView(
            device.m_device,
            m_swapChainImages[i],
            format,
            aspectMask,
            &m_swapChainImageViews[i]
        );
    }

    // m_swapChainImages = framebuffer.m_images;
    // m_swapChainImageViews = framebuffer.m_imageViews;
}

void Swapchain::destroy()
{
    for (auto &iv : m_swapChainImageViews) vkDestroyImageView(m_device, iv, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

Framebuffer::Framebuffer(
    const Device &device,
    const Renderpass &renderpass,
    const Swapchain &swapchain) // This should be part of attachment creation.
{
    m_device = device.m_device;
    const size_t swapchainSize = swapchain.m_swapChainImages.size();
    m_framebuffers.resize(swapchainSize);

    // For each image in the swapchain.
    for (size_t i = 0; i < swapchainSize; i++)
    {
        std::vector<VkImageView> imageViews(renderpass.m_attachments.size());
        imageViews[0] = swapchain.m_swapChainImageViews[i];
        for (size_t j = 1; j < renderpass.m_attachments.size(); j++)
        {
            auto attachment = renderpass.m_attachments[j];
            const uint32_t &index = attachment.m_index;
            imageViews[index]=attachment.m_imageView;
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass.m_renderPass;
        framebufferInfo.attachmentCount = imageViews.size();
        framebufferInfo.pAttachments = imageViews.data();
        framebufferInfo.width = swapchain.m_swapChainExtent.width;
        framebufferInfo.height = swapchain.m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &(m_framebuffers)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer.");
        }
    }
}

void Framebuffer::destroy()
{
    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
}