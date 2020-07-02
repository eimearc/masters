#ifndef BUF
#define BUF

#include <vulkan/vulkan.h>
#include <vector>
#include "evulkan_util.h"
#include "device.h"

class Buffer
{
    public:
    Buffer()=default;
    Buffer(size_t swapchainSize, const Device &device);

    void setBuffer(const VkDeviceSize &bufferSize);
    void updateBuffer(const void *srcBuffer);

    void setIndexBuffer(
        const void *indices,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        VkCommandPool commandPool
    );
    void setVertexBuffer(
        const void *vertices,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        Device &device,
        std::vector<VkCommandPool> commandPools
    );

    void destroy();

    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;
    size_t m_numElements;

    private:
    size_t m_swapchainSize;
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkDeviceSize m_bufferSize;
    VkQueue m_queue;

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