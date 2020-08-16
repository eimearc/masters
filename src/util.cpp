#include "util.h"

#include "evk_assert.h"
#include <vulkan/vulkan.h>

namespace internal
{

void createImage(
    const VkDevice &device,
    const VkPhysicalDevice &physicalDevice,
    const VkExtent2D &extent,
    const VkFormat &format,
    const VkImageTiling &tiling,
    const VkImageUsageFlags &usage,
    const VkMemoryPropertyFlags &properties,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory
) noexcept
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

    auto result = vkCreateImage(device, &imageInfo, nullptr, pImage);
    EVK_ASSERT(result,"failed to create image\n");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice, memRequirements.memoryTypeBits, properties
    );

    result = vkAllocateMemory(device, &allocInfo, nullptr, pImageMemory);
    EVK_ASSERT(result, "failed to allocate image memory");

    vkBindImageMemory(device, *pImage, *pImageMemory, 0);
}

void createImageView(
    const VkDevice &device,
    const VkImage &image,
    const VkFormat &format,
    const VkImageAspectFlags &aspectMask,
    VkImageView *pImageView
) noexcept
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

    auto result = vkCreateImageView(device, &viewInfo, nullptr, pImageView);
    EVK_ASSERT(result, "failed to create texture image view\n");
}

void createBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
) noexcept
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto result = vkCreateBuffer(device, &bufferInfo, nullptr, pBuffer);
    EVK_ASSERT(result,"failed to create buffer\n");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *pBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice, memRequirements.memoryTypeBits, properties
    );

    result = vkAllocateMemory(device, &allocInfo, nullptr, pBufferMemory);
    EVK_ASSERT(result, "failed to allocate buffer memory");
    result = vkBindBufferMemory(device, *pBuffer, *pBufferMemory, 0);
    EVK_ASSERT(result, "failed to bind buffer memory");
}

uint32_t findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags requiredProperties
) noexcept
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    uint32_t currentTypeFilter;
    VkMemoryPropertyFlags currentProperties;
    bool propertiesMatch;

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        currentTypeFilter = typeFilter & (1<<i);
        currentProperties = memProperties.memoryTypes[i].propertyFlags;
        propertiesMatch = (currentProperties & requiredProperties) \
            == requiredProperties;
        if (currentTypeFilter && propertiesMatch) return i;
    }

    EVK_ABORT("failed to find suitable memory type\n");
}

void beginSingleTimeCommands(
    VkDevice device,
    VkCommandPool commandPool,
    VkCommandBuffer *pCommandBuffer
) noexcept
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocInfo, pCommandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(*pCommandBuffer, &beginInfo);
}

void endSingleTimeCommands(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer
) noexcept
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) noexcept
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, surface, &details.capabilities
    );
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &formatCount, nullptr
    );
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &formatCount, details.formats.data()
        );
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, nullptr
    );
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &presentModeCount, details.presentModes.data()
        );
    }

    return details;
}

QueueFamilyIndices findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) noexcept
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, nullptr
    );
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data()
    );

    uint32_t i = 0;
    for (const auto &family : queueFamilies)
    {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                device, i, surface, &presentSupport
            );
            if (presentSupport) indices.presentFamily = i;
        }
        if (indices.isComplete()) break;
        ++i;
    }
    return indices;
}

} // namespace internal