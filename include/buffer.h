#ifndef BUF
#define BUF

#include <vulkan/vulkan.h>
#include <vector>
#include "evulkan_util.h"

class Buffer
{
    public:
    Buffer()=default;
    Buffer(size_t swapchainSize, VkDevice device, VkPhysicalDevice physicalDevice);

    void setBuffer(const VkDeviceSize &bufferSize);
    void updateBuffer(const void *srcBuffer);

    void setIndexBuffer(const VkDeviceSize &bufferSize, const void *indexBuffer, const size_t numElements, VkCommandPool commandPool, VkQueue queue);
    // void setVertexBuffer();

    void destroy();

    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;

    private:
    size_t m_swapchainSize;
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkDeviceSize m_bufferSize;

    void copyBuffer(
        VkCommandPool commandPool,
        VkQueue queue,
        VkBuffer srcBuffer,
        VkBuffer dstBuffer);

    uint32_t findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties);
};

#endif