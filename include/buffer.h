#ifndef EVK_BUFFER_H_
#define EVK_BUFFER_H_

#include "command.h"
#include "device.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

class Buffer
{
    public:
    // enum Type{INDEX,VERTEX};
    void destroy();

    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    size_t m_numElements;

    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkDeviceSize m_bufferSize;
    VkQueue m_queue;
    size_t m_numThreads;

    void *m_data;
    VkDeviceSize m_elementSize;

    void copyBuffer(
        VkCommandPool commandPool,
        VkQueue queue,
        VkBuffer srcBuffer,
        VkBuffer dstBuffer);

    uint32_t findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties);
};

class DynamicBuffer : public Buffer
{
    public:
    DynamicBuffer()=default;
    DynamicBuffer(const Device &device, const VkDeviceSize &bufferSize);

    void update(const void *srcBuffer);
};

class StaticBuffer : public Buffer
{
    public:
    StaticBuffer()=default;
    StaticBuffer(
        const Device &device,
        void *data,
        const VkDeviceSize &elementSize,
        const size_t numElements
    );

    void finalizeIndex(Device &device,Commands &commands);
    void finalizeVertex(Device &device,Commands &commands);
};

#endif