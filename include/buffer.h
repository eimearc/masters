#ifndef BUF
#define BUF

#include <vulkan/vulkan.h>
#include <vector>

class Buffer
{
    public:
    Buffer()=default;
    Buffer(size_t swapchainSize, VkDevice device, VkPhysicalDevice physicalDevice);

    void setBuffer(const VkDeviceSize &bufferSize);
    void updateBuffer(const void *srcBuffer);
    void destroy();

    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;

    private:
    size_t m_swapchainSize;
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkDeviceSize m_bufferSize;

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer *pBuffer,
        VkDeviceMemory *pBufferMemory);

    uint32_t findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties);
};

#endif