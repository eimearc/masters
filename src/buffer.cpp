#include "buffer.h"

#include "evk_assert.h"

namespace evk {

Buffer::Buffer(Buffer &&other) noexcept
{
    *this=std::move(other);
}

Buffer::~Buffer() noexcept
{
    if (m_bufferData!=nullptr) free(m_bufferData);
    if (m_buffer!=VK_NULL_HANDLE)
        vkDestroyBuffer(m_device, m_buffer, nullptr);
    if (m_bufferMemory!=VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_bufferMemory, nullptr);
}

Buffer& Buffer::operator=(Buffer &&other) noexcept
{
    if (*this==other) return *this;
    m_buffer=other.m_buffer;
    m_bufferData=other.m_bufferData;
    m_bufferMemory=other.m_bufferMemory;
    m_bufferSize=other.m_bufferSize;
    m_device=other.m_device;
    m_elementSize=other.m_elementSize;
    m_physicalDevice=other.m_physicalDevice;
    m_numElements=other.m_numElements;
    m_numThreads=other.m_numThreads;
    m_queue=other.m_queue;
    other.reset();
    return *this;
}

void Buffer::reset() noexcept
{
    m_buffer=VK_NULL_HANDLE;
    m_bufferData=nullptr;
    m_bufferMemory=VK_NULL_HANDLE;
    m_bufferSize=0;
    m_device=VK_NULL_HANDLE;
    m_elementSize=0;
    m_numElements=0;
    m_numThreads=1;
    m_physicalDevice=VK_NULL_HANDLE;
    m_queue=VK_NULL_HANDLE;
}

bool Buffer::operator==(const Buffer &other) const noexcept
{
    if (m_buffer!=other.m_buffer) return false;
    if (m_bufferMemory!=other.m_bufferMemory) return false;
    if (m_numElements!=other.m_numElements) return false;
    if (m_device!=other.m_device) return false;
    if (m_physicalDevice!=other.m_physicalDevice) return false;
    if (m_bufferSize!=other.m_bufferSize) return false;
    if (m_queue!=other.m_queue) return false;
    if (m_numThreads!=other.m_numThreads) return false;
    if (m_elementSize!=other.m_elementSize) return false;
    return true;
}

bool Buffer::operator!=(const Buffer &other) const noexcept
{
    return !(*this==other);
}

VkBufferUsageFlags Buffer::typeToFlag(const Type &type) const noexcept
{
    switch(type)
    {
        case Type::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case Type::INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case Type::UBO:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
}

StaticBuffer::StaticBuffer(
    Device &device,
    const void *data,
    const VkDeviceSize &elementSize,
    const size_t numElements,
    const Type &type
) noexcept
{
    m_elementSize=elementSize;
    m_numElements=numElements;

    m_bufferSize = m_numElements * m_elementSize;
    m_bufferData = malloc(m_bufferSize);
    memcpy(m_bufferData,data,m_bufferSize);

    m_device = device.device();
    m_numThreads = device.numThreads();
    m_physicalDevice = device.physicalDevice();
    m_queue = device.graphicsQueue();

    finalize(device, type);
}

void StaticBuffer::finalize(
    Device &device,
    const Type &type
) noexcept
{
    const std::vector<VkCommandPool> &commandPools = device.commandPools();

    size_t numThreadsLocal = m_numThreads;
    if (m_numElements<numThreadsLocal) numThreadsLocal = m_numElements;

    const int num_elements_each = m_numElements/numThreadsLocal;
    m_bufferSize = m_elementSize*m_numElements;

    std::vector<VkCommandBuffer> commandBuffers(numThreadsLocal);
    std::vector<VkBuffer> buffers(numThreadsLocal);
    std::vector<VkDeviceMemory> bufferMemory(numThreadsLocal);

    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    usageFlags |= typeToFlag(type);

    // Create the device-local buffer.
    internal::createBuffer(
        m_device, m_physicalDevice, m_bufferSize, usageFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &m_buffer, &m_bufferMemory
    );

    auto setupCopyFunction = [&](int thread)
    {
        int element_offset = num_elements_each*thread;

        int num_elements=num_elements_each;
        if (thread==(numThreadsLocal-1)) 
            num_elements = m_numElements-(thread*num_elements_each);

        auto &staging_buffer = buffers[thread];
        auto &staging_buffer_memory = bufferMemory[thread];

        copyData(
            m_device, m_physicalDevice, commandPools[thread],
            commandBuffers[thread], m_buffer, staging_buffer,
            staging_buffer_memory, num_elements, m_elementSize,
            element_offset
        );
    };

    int counter = 0;
    for (auto &t: device.threads())
    {
        if (counter < numThreadsLocal)
            t->addJob(std::bind(setupCopyFunction,counter++));
    }
    device.wait();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);

    for (size_t i = 0; i<numThreadsLocal; ++i)
    {
        vkFreeCommandBuffers(m_device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(m_device, buffers[i], nullptr);
        vkFreeMemory(m_device, bufferMemory[i], nullptr);
    }
}

void StaticBuffer::copyData(
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
) const noexcept
{
    const size_t buffer_offset = element_offset*element_size;
    const size_t buffer_size = num_elements*element_size;

    EVK_ASSERT_TRUE(buffer_size>0, "buffer size is 0\n");

    // Use a host visible buffer as a staging buffer.
    internal::createBuffer(
        device, physicalDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_memory
    );

    const unsigned char* bytePtr = reinterpret_cast<const unsigned char*>(m_bufferData);
    auto offset = element_offset*element_size;

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *mappedData;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &mappedData);
    memcpy(mappedData, bytePtr+offset, buffer_size); // Offset vertices.
    vkUnmapMemory(device, staging_buffer_memory);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &allocInfo, &command_buffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = buffer_size;
    copyRegion.dstOffset = buffer_offset;
    vkCmdCopyBuffer(command_buffer, staging_buffer, device_buffer, 1, &copyRegion);

    vkEndCommandBuffer(command_buffer);
}

DynamicBuffer::DynamicBuffer(
    const Device &device,
    const VkDeviceSize &bufferSize,
    const Type &type
) noexcept
{
    m_device = device.device();
    m_bufferSize=bufferSize;
    m_numElements=1;

    internal::createBuffer(
        m_device, device.physicalDevice(), bufferSize, typeToFlag(type),
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &m_buffer, &m_bufferMemory);
}

void DynamicBuffer::update(const void *srcBuffer) noexcept
{
    if (m_bufferData!=nullptr) free(m_bufferData);
    m_bufferData = malloc(m_bufferSize);
    memcpy(m_bufferData,srcBuffer,m_bufferSize);

    void* dstBuffer;
    vkMapMemory(m_device, m_bufferMemory, 0, m_bufferSize, 0, &dstBuffer);
    memcpy(dstBuffer, srcBuffer, m_bufferSize);
    vkUnmapMemory(m_device, m_bufferMemory);
}

DynamicBuffer::DynamicBuffer(
    Device &device,
    const void *data,
    const VkDeviceSize &elementSize,
    const size_t numElements,
    const Type &type
) noexcept
{
    m_device = device.device();
    m_physicalDevice=device.physicalDevice();
    m_queue=device.graphicsQueue();
    
    m_bufferSize=elementSize*numElements;
    m_numElements=numElements;
    m_bufferData = malloc(m_bufferSize);
    
    memcpy(m_bufferData,data,m_bufferSize);

    VkBufferUsageFlags usageFlags = typeToFlag(type);

    internal::createBuffer(
        m_device, device.physicalDevice(), m_bufferSize, usageFlags,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &m_buffer, &m_bufferMemory);

    update(data);
}

void Buffer::copyBuffer(
    VkCommandPool commandPool,
    VkQueue queue,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer) const noexcept
{
    VkCommandBuffer commandBuffer;
    internal::beginSingleTimeCommands(m_device, commandPool, &commandBuffer);

    VkBufferCopy copyRegion = {};
    copyRegion.size = m_bufferSize;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    internal::endSingleTimeCommands(m_device, queue, commandPool, commandBuffer);
}

} // namespace evk