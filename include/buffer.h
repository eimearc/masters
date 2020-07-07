#ifndef BUF
#define BUF

#include <vulkan/vulkan.h>
#include <vector>
#include "command.h"
#include "device.h"
#include "evulkan_util.h"

class Buffer
{
    public:
    Buffer()=default;
    Buffer(const Device &device);

    void setBuffer(const VkDeviceSize &bufferSize);
    void updateBuffer(const void *srcBuffer);

    void setIndexBuffer(
        const void *indices,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        Commands &commands
    );
    void setVertexBuffer(
        Device &device,
        const void *vertices,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        Commands &commands
    );

    void destroy();

    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    size_t m_numElements;

    private:
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkDeviceSize m_bufferSize;
    VkQueue m_queue;
    size_t m_numThreads;

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