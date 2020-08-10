#include "device.h"

#include "evk_assert.h"

namespace evk {

Device::Swapchain::Swapchain(
    const VkDevice &device,
    const VkPhysicalDevice &physicalDevice,
    const VkSurfaceKHR &surface,
    VkExtent2D windowExtent,
    const uint32_t swapchainSize)
{
    m_device = device;
    m_swapchainSize = swapchainSize;
    m_surface = surface;
    m_physicalDevice = physicalDevice;
    m_windowExtent=windowExtent;

    setup();
}

void Device::Swapchain::setup() noexcept
{
    auto swapChainSupport = internal::querySwapChainSupport(
        m_physicalDevice, m_surface
    );
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = m_swapchainSize;
    bool validSwapchainSize =
        imageCount >= swapChainSupport.capabilities.minImageCount &&
        imageCount <= swapChainSupport.capabilities.maxImageCount;
    EVK_EXPECT_TRUE(
        validSwapchainSize, "swapchain size outside of swapchain capabilities"
    );

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = internal::findQueueFamilies(m_physicalDevice, m_surface);
    uint32_t queueFamilyIndices[] = {
        static_cast<uint32_t>(indices.graphicsFamily),
        static_cast<uint32_t>(indices.presentFamily)
    };
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

    vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain);

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

    m_format = surfaceFormat.format;
    m_extent = extent;

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkFormat format = m_format;
    m_imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        internal::createImageView(
            m_device,
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

VkExtent2D Device::Swapchain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities
) const
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D extent = m_windowExtent;

        extent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, extent.height));

        return extent;
    }   
}

VkPresentModeKHR Device::Swapchain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes
) const
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Device::Swapchain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats
) const
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    throw std::runtime_error("no suitable format found in available formats.");
}

void Device::Swapchain::recreate()
{
    destroy();
    setup();
}

void Device::Swapchain::destroy() noexcept
{
    for (auto &iv : m_imageViews) vkDestroyImageView(m_device, iv, nullptr);
    m_imageViews.resize(0);
    if (m_swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_swapchain=VK_NULL_HANDLE;
}

Device::Swapchain::~Swapchain() noexcept
{
    destroy();
}

} // namespace evk