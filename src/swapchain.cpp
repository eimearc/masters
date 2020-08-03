#include "device.h"

Device::Swapchain::Swapchain(
    const VkDevice &device,
    const VkPhysicalDevice &physicalDevice,
    const VkSurfaceKHR &surface,
    GLFWwindow *window, //TODO: Make const.
    const uint32_t swapchainSize)
{
    m_device = device;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

    uint32_t imageCount = swapchainSize;
    if (imageCount < swapChainSupport.capabilities.minImageCount || imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        throw std::runtime_error("Please specify an image count within the swapchain capabilites.");
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
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

    if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data());

    m_format = surfaceFormat.format;
    m_extent = extent;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkFormat format = m_format;
    m_imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        createImageView(
            device,
            m_images[i],
            format,
            aspectMask,
            &m_imageViews[i]
        );
    }
}

Device::Swapchain::Swapchain(Swapchain &&other) noexcept
{
    *this=std::move(other);
}

Device::Swapchain& Device::Swapchain::operator=(Swapchain &&other) noexcept
{
    if (*this == other) return *this;
    m_device=other.m_device;
    m_swapchain=other.m_swapchain;
    m_images=other.m_images;
    m_imageViews=other.m_imageViews;
    m_format=other.m_format;
    m_extent=other.m_extent;
    other.reset();
    return *this;
}

void Device::Swapchain::reset() noexcept
{
    m_device=VK_NULL_HANDLE;
    m_swapchain=VK_NULL_HANDLE;
    m_imageViews.resize(0);
    m_imageViews.resize(0);
    m_format={};
    m_extent={};
}

bool Device::Swapchain::operator==(const Swapchain &other) const
{
    if (m_device!=other.m_device) return false;
    if (m_extent.width!=other.m_extent.width) return false;
    if (m_extent.height!=other.m_extent.height) return false;
    if (m_format!=other.m_format) return false;
    if (!std::equal(
            m_images.begin(), m_images.end(), other.m_images.begin()
        ))
        return false;
    if (!std::equal(
            m_imageViews.begin(), m_imageViews.end(), other.m_imageViews.begin()
        ))
        return false;
    if (m_swapchain!=other.m_swapchain) return false;
    return true;
}

bool Device::Swapchain::operator!=(const Swapchain &other) const
{
    return !(*this==other);
}

Device::Swapchain::~Swapchain() noexcept
{
    for (auto &iv : m_imageViews) vkDestroyImageView(m_device, iv, nullptr);
    if (m_swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}