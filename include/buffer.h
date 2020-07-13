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
    enum Type{INDEX,VERTEX,CUSTOM};
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
        Device &device,
        Commands &commands,
        void *data,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        const Type &type
    );

    void finalize(Device &device,Commands &commands, const Type &type);

    void copyData(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool command_pool,
        VkCommandBuffer &command_buffer,
        VkBuffer device_buffer,
        VkBuffer &staging_buffer,
        VkDeviceMemory &staging_buffer_memory,
        const size_t num_elements,
        const VkDeviceSize element_size,
        const size_t element_offset
    );
};

#endif