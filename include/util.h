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
#include <set>
#include <vector>
#include "vertex.h"
#include <vulkan/vulkan.h>

namespace evk
{
    struct SubpassDependency
    {
        uint32_t srcSubpass;
        uint32_t dstSubpass;
    };

    void loadOBJ(const std::string &fileName, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
}

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
    VkDeviceMemory *pBufferMemory);

uint32_t findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties);

void beginSingleTimeCommands(
    VkDevice device,
    VkCommandPool commandPool,
    VkCommandBuffer *pCommandBuffer);

void endSingleTimeCommands(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer);

void createShaderModule(
    VkDevice device,
    const std::vector<char>& code,
    VkShaderModule *pShaderModule);

QueueFamilyIndices getQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);

QueueFamilyIndices findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);
    
SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);

std::vector<const char*> getRequiredExtensions();

#endif