#ifndef EVK_BUFFER_H_
#define EVK_BUFFER_H_

#include "device.h"
#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

/**
 * @class Buffer
 * @brief A Buffer is used to make data available to the GPU.
 * 
 * Buffers take data from the user and make that data available to the GPU. This
 * data can either be copied over to a device-local memory, allowing for slower
 * setup speeds with faster acess speeds, or the data can be made available to
 * the GPU with faster setup speeds with slower access speeds.
 * 
 * For data that will be updated during the program, it is recommended to use a
 * DynamicBuffer. Otherwise, a StaticBuffer is optimal.
 * 
 * Common usages of the StaticBuffer include an index buffer, or vertex buffer.
 * A common usage of the DynamicBuffer includes a Uniform Buffer Object (UBO)
 * which is updated every frame.
 * 
 * The index buffer and vertex buffer must be attached to the Device, using the
 * finalize() method. Other custom buffers must be attached to a Descriptor.
 * 
 * @example
 * StaticBuffer indexBuffer(
 *   device, indices.data(), sizeof(indices[0]), indices.size(), Buffer::INDEX
 * );
 * StaticBuffer vertexBuffer(
 *   device, vertices.data(), sizeof(vertices[0]), vertices.size(),
 *   Buffer::VERTEX
 * );
 * 
 * DynamicBuffer ubo(device, &ubo, sizeof(ubo), 1, Buffer::UBO);
 * descriptor.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);
 * 
 * device.finalize(indexBuffer,vertexBuffer,pipelines);
 **/
class Buffer
{
    public:
    enum Type{INDEX,VERTEX,UBO};

    Buffer()=default;
    Buffer(const Buffer&)=delete; // Class Buffer is not copyable.
    Buffer& operator=(const Buffer&)=delete; // Class Buffer is not copyable.
    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;
    ~Buffer() noexcept;

    bool operator==(const Buffer&) const;

    protected:
    VkBuffer buffer() const { return m_buffer; };
    size_t numElements() const { return m_numElements; };
    VkDeviceSize size() const { return m_bufferSize; }; 

    void copyBuffer(
        VkCommandPool commandPool,
        VkQueue queue,
        VkBuffer srcBuffer,
        VkBuffer dstBuffer
    ) const;
    void reset() noexcept;
    VkBufferUsageFlags typeToFlag(const Type &type) const;

    VkBuffer m_buffer=VK_NULL_HANDLE;
    void *m_bufferData=nullptr;
    VkDeviceMemory m_bufferMemory=VK_NULL_HANDLE;
    VkDeviceSize m_bufferSize=0;
    VkDevice m_device=VK_NULL_HANDLE;
    VkDeviceSize m_elementSize=0;
    size_t m_numElements=0;
    size_t m_numThreads=1;
    VkPhysicalDevice m_physicalDevice=VK_NULL_HANDLE;
    VkQueue m_queue=VK_NULL_HANDLE;

    friend class Descriptor;
    friend class Device;

    // Tests.
    friend class BufferTest_update_Test;
};

class DynamicBuffer : public Buffer
{
    public:
    DynamicBuffer(
        const Device &device,
        const VkDeviceSize &bufferSize
    );
    DynamicBuffer(
        Device &device,
        const void *data,
        const VkDeviceSize &element_size,
        const size_t num_elements,
        const Type &type
    );

    void update(const void *srcBuffer);
};

class StaticBuffer : public Buffer
{
    public:
    StaticBuffer(
        Device &device,
        const void *data,
        const VkDeviceSize &elementSize,
        const size_t numElements,
        const Type &type
    );

    private:
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
    ) const;
    void finalize(
        Device &device,
        const Type &type
    );
};

#endif