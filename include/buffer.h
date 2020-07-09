#ifndef EVK_BUFFER
#define EVK_BUFFER

#include "command.h"
#include "device.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

class Buffer
{
    public:
    Buffer()=default;
    Buffer(const Device &device);
    Buffer(
        const Device &device,
        void *data,
        const VkDeviceSize &elementSize,
        const size_t numElements
    );

    enum Type{INDEX,VERTEX};

    void setBuffer(const VkDeviceSize &bufferSize);
    void updateBuffer(const void *srcBuffer);

    void finalizeVertex(Device &device, Commands &commands);
    void finalizeIndex(Device &device, Commands &commands);

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

#endif