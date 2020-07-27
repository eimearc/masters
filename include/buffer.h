#ifndef EVK_BUFFER_H_
#define EVK_BUFFER_H_

#include "device.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

class Buffer
{
    public:
    enum Type{INDEX,VERTEX,UBO};

    Buffer()=default;
    Buffer(const Buffer&)=delete;
    Buffer& operator=(const Buffer&)=delete;
    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;
    ~Buffer() noexcept;

    bool operator==(const Buffer&) const;

    // protected:
    VkBuffer m_buffer=VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory=VK_NULL_HANDLE;
    VkDeviceSize m_bufferSize;
    void *m_data;
    VkDevice m_device;
    VkDeviceSize m_elementSize;
    size_t m_numElements;
    size_t m_numThreads;
    VkPhysicalDevice m_physicalDevice;
    VkQueue m_queue;

    VkBufferUsageFlags typeToFlag(const Type &type);

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
    DynamicBuffer(
        Device &device,
        void *data,
        const VkDeviceSize &element_size,
        const size_t num_elements,
        const Type &type
    );

    void update(const void *srcBuffer);
};

class StaticBuffer : public Buffer
{
    public:
    StaticBuffer()=default;
    // StaticBuffer(const StaticBuffer&)=delete;
    // StaticBuffer& operator=(const StaticBuffer&)=delete;
    // StaticBuffer(StaticBuffer&&) noexcept;
    // StaticBuffer& operator=(StaticBuffer&&) noexcept;

    StaticBuffer(
        Device &device,
        void *data,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        const Type &type
    );

    void finalize(Device &device, const Type &type);

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