#ifndef EVK_UTIL_H_
#define EVK_UTIL_H_

#include <fstream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "vertex.h"
#include <vulkan/vulkan.h>

namespace evk
{
    // TODO: Move.
    struct SubpassDependency
    {
        uint32_t srcSubpass;
        uint32_t dstSubpass;
    };

    /**
     * Loads and OBJ file into a vector of vertices and indices.
     * @param[in] fileName the file where the OBJ is contained.
     * @param[out] vertices the vertices of the OBJ.
     * @param[out] indices the indices of the OBJ.
     **/
    void loadOBJ(const std::string &fileName, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
}

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
);

void createImageView(
    const VkDevice &device,
    const VkImage &image,
    const VkFormat &format,
    const VkImageAspectFlags &aspectMask,
    VkImageView *pImageView
);

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void createBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
);

uint32_t findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
);

void beginSingleTimeCommands(
    VkDevice device,
    VkCommandPool commandPool,
    VkCommandBuffer *pCommandBuffer
);

void endSingleTimeCommands(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer
);

QueueFamilyIndices findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);
    
SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);

} // namespace internal

#endif