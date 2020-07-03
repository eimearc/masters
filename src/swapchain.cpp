#include "swapchain.h"

Swapchain::Swapchain(const uint32_t swapchainSize, Attachment &framebuffer, const Device &device)
{
    m_device = device.m_device;

    framebuffer = {0,swapchainSize};
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
    framebuffer.m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device.m_device, m_swapChain, &imageCount, framebuffer.m_images.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkFormat format = m_swapChainImageFormat;
    VkImage image;
    framebuffer.m_imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        image = framebuffer.m_images[i];
        createImageView(
            device.m_device,
            image,
            format,
            aspectMask,
            &framebuffer.m_imageViews[i]
        );
    }

    m_swapChainImages = framebuffer.m_images;
    m_swapChainImageViews = framebuffer.m_imageViews;
}

void Swapchain::destroy()
{
    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}