#include "evulkan_core.h"

void evk::Instance::createSwapChain(const SwapChainCreateInfo *pCreateInfo)
{
    m_maxFramesInFlight=pCreateInfo->maxFramesInFlight;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(m_window, swapChainSupport.capabilities);

    uint32_t imageCount = m_maxFramesInFlight;
    if (imageCount < swapChainSupport.capabilities.minImageCount || imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        throw std::runtime_error("Please specify an image count within the swapchain capabilites.");
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);
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

    if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.aspectFlags=VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.format=m_swapChainImageFormat;
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
    {
        imageViewCreateInfo.image=m_swapChainImages[i];
        createImageView(&imageViewCreateInfo, &m_swapChainImageViews[i]);
    }

    Attachment &attachment = m_evkattachments[evk::FRAMEBUFFER_ATTACHMENT];
    attachment.images=m_swapChainImages;
    attachment.imageViews=m_swapChainImageViews;   
}