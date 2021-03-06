#ifndef EVK_UTIL_H_
#define EVK_UTIL_H_

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "vertex.h"
#include <vulkan/vulkan.h>

// Macro for GTest friend class.
#define FRIEND_TEST(test_case_name, test_name)\
    friend class test_case_name##_##test_name##_Test

namespace internal
{

/**
 * Creates a VkImage and binds it to VkDeviceMemory.
 * @param[in] device the VkDevice to use for image creation.
 * @param[in] physicalDevice the VkPhysicalDevice to use for image creation.
 * @param[in] extent the width and height of the image.
 * @param[in] format the format the image should be in.
 * @param[in] tiling describes how the texels should be laid out in memory.
 * @param[in] usage how the image will be used.
 * @param[in] properties where in memory the image should be allocated.
 * @param[out] pImage a pointer to the allocated image.
 * @param[out] pImageMemory a pointer to the allocated memory, to which the
 *  image is bound.
 **/
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
) noexcept;

/**
 * Creates a VkImageView.
 * @param[in] device the VkDevice to use for image creation.
 * @param[in] image the VkImage the VkImageView will be created for.
 * @param[in] format the format the image is in.
 * @param[in] aspectMask specifies which aspects of an image are includeded
 *  in the view.
 * @param[out] pImageView a pointer to the allocated VkImageView.
 **/
void createImageView(
    const VkDevice &device,
    const VkImage &image,
    const VkFormat &format,
    const VkImageAspectFlags &aspectMask,
    VkImageView *pImageView
) noexcept;

/**
 * Represents the capabilities of a VkQueueFamily.
 **/
struct QueueFamilyIndices
{
    int graphicsFamily=-1;
    int presentFamily=-1;

    bool isComplete() noexcept
    {
        return graphicsFamily>=0 && presentFamily>=0;
    }
};

/**
 * Represents the support for the Swap Chain.
 **/
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/**
 * Creates a VkBuffer and binds it to a VkBufferMemory.
 * @param[in] device the VkDevice to use for allocation.
 * @param[in] physicalDevice the VkPhysicalDevice to use for allocation.
 * @param[in] size the size of the buffer in bytes.
 * @param[in] usage how the buffer will be used.
 * @param[in] properties the desired properties of the VkDeviceMemory.
 * @param[out] pBuffer a pointer to the allocated VkBuffer.
 * @param[out] pBufferMemory a pointer to the allocated VkDeviceMemory.
 **/
void createBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
) noexcept;

/**
 * Finds the index of a suitable memory type, matching desired properties.
 * @param[in] physicalDevice the VkPhysicalDevice to query.
 * @param[in] typeFilter a filter for the type of memory.
 * @param[in] properties the desired properties for the memory.
 * @returns the index of a suitable memory type.
 **/
uint32_t findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
) noexcept;

/**
 * Begins recording work into a newly-allocated command buffer.
 * @param[in] device the VkDevice to allocate the buffer from.
 * @param[in] commandPool the VkCommandPool from which to allocate the buffer.
 * @param[out] pCommandBuffer the newly-allocated VkCommandBuffer.
 **/
void beginSingleTimeCommands(
    VkDevice device,
    VkCommandPool commandPool,
    VkCommandBuffer *pCommandBuffer
) noexcept;

/**
 * Ends a command buffer and submits it to a queue.
 * @param[in] device the VkDevice owning the buffer.
 * @param[in] queue the VkQueue to submit the buffer to.
 * @param[in] commandPool the VkCommandPool the buffer was allocated from.
 * @param[in] commandBuffer the VkCommandBuffer containing the recorded work.
 **/
void endSingleTimeCommands(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer
) noexcept;

/**
 * Finds the VkQueue families which support a surface.
 * @param[in] device the VkPhysicalDevice used to search for queue families.
 * @param[in] surface the VkSurfaceKHR for which to query support.
 * @returns the indices of the queue families which support this surface.
 **/
QueueFamilyIndices findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) noexcept;
    
/**
 * Finds the requirements for swapchain support.
 * @param[in] device the VkPhysicalDevice used to search for support.
 * @param[in] surface the surface used to query for swapchain support.
 * @returns the details of support required for the swapchain.
 **/
SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) noexcept;

} // namespace internal

#endif