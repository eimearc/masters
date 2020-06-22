#ifndef EVK_CORE
#define EVK_CORE

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <optional>
#include "threadpool.h"
#include "evulkan_util.h"
#include "vertex.h"
#include <array>
#include <fstream>
#include <map>

#define GLFW_INCLUDE_VULKAN

namespace evk
{

typedef uint32_t Index;

struct InstanceCreateInfo
{
    std::vector<const char*> validationLayers;
    GLFWwindow *window;
    std::vector<const char *> deviceExtensions;
};

struct SwapChainCreateInfo
{
    uint8_t maxFramesInFlight;
};

struct GraphicsPipelineCreateInfo
{
    std::string vertShaderFile;
    std::string fragShaderFile;
};

struct EVkRenderPassCreateInfo
{
    VkFormat swapChainImageFormat;
    VkPhysicalDevice physicalDevice;
};

struct ImageCreateInfo
{
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags properties;
};

struct ImageViewCreateInfo
{
    VkImage image;
    VkFormat format;
    VkImageAspectFlags aspectFlags;
};

struct VertexAttributeInfo
{
    uint32_t location;
    uint32_t offset;
};

void loadOBJ(const std::string &fileName, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

class Instance
{
    public:
    Instance(size_t numThreads, const InstanceCreateInfo *pCreateInfo)
    {
        m_threadPool.setThreadCount(numThreads);
        m_numThreads=numThreads;

        m_deviceExtensions=pCreateInfo->deviceExtensions;
        m_validationLayers=pCreateInfo->validationLayers;
        m_window=pCreateInfo->window;

        createInstance(pCreateInfo->validationLayers);
        createSurface(pCreateInfo->window);
        pickPhysicalDevice(pCreateInfo->deviceExtensions);
        createDevice(true);
    }
    Instance()=default;

    void updateBuffer(const std::string &name);

    void createSwapChain(const SwapChainCreateInfo *pCreateInfo);
    void createRenderPass();
    void registerVertexShader(const std::string &vertShader);
    void registerFragmentShader(const std::string &fragShader);
    void addVertexAttributeVec2(const uint32_t &location, const uint32_t &offset);
    void addVertexAttributeVec3(const uint32_t &location, const uint32_t &offset);
    void setBindingDescription(uint32_t stride);
    void createGraphicsPipeline();
    void createDepthResources();
    void createBufferObject(std::string name, VkDeviceSize bufferSize);
    void updateBufferObject(std::string name, VkDeviceSize bufferSize, void *srcBuffer, size_t imageIndex);
    void createDescriptorSets();
    void createSyncObjects();
    void createFramebuffers();
    void createCommandPools();
    void createIndexBuffer(const std::vector<Index> &indices);
    void createVertexBuffer(const std::vector<Vertex> &vertices);
    void createDrawCommands();
    void draw();
    void cleanup();

    void loadTexture(const std::string &fileName);

    ThreadPool m_threadPool;
    size_t m_numThreads;

    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkDevice m_device;

    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    VkRenderPass m_renderPass;

    std::vector<VkShaderModule> m_shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
    VkPipelineLayout m_graphicsPipelineLayout;
    VkPipeline m_graphicsPipeline;

    VkImage m_depthImage;
    VkImageView m_depthImageView;
    VkDeviceMemory m_depthImageMemory;

    VkImage m_backBufferImage;
    VkImageView m_backBufferView;
    VkDeviceMemory m_backBufferMemory;

    // std::vector<VkBuffer> m_uniformBuffers;
    // std::vector<VkDeviceMemory> m_uniformBuffersMemory;

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_fencesInFlight;
    std::vector<VkFence> m_imagesInFlight;

    std::vector<VkFramebuffer> m_framebuffers;

    std::vector<VkCommandPool> m_commandPools;
    std::vector<VkCommandBuffer> m_primaryCommandBuffers;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffers;

    struct BufferInfo
    {
        size_t index;
        size_t size;
    };
    std::map<std::string, BufferInfo> m_bufferMap;
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;

    std::vector<Index> m_indices;

    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    VkVertexInputBindingDescription m_bindingDescription;

    VkImage m_textureImage=VK_NULL_HANDLE;
    VkImageView m_textureImageView=VK_NULL_HANDLE;
    VkSampler m_textureSampler=VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory=VK_NULL_HANDLE;
    VkDescriptorImageInfo m_textureDescriptor;

    std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
    std::vector<VkDescriptorSetLayoutBinding> m_descriptorSetBindings;
    std::vector<std::vector<VkWriteDescriptorSet>> m_writeDescriptorSet;
    std::vector<VkDescriptorBufferInfo> m_descriptorBufferInfo;
    std::vector<VkDescriptorImageInfo> m_descriptorTextureSamplerInfo;

    private:
    void createInstance(std::vector<const char*> validationLayers);
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice(std::vector<const char *> deviceExtensions);
    void createDevice(bool enableValidation);
    VkResult createDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
    VkFormat findDepthFormat(const EVkRenderPassCreateInfo *pCreateInfo);
    VkFormat findSupportedFormat(
        const EVkRenderPassCreateInfo *pCreateInfo,
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features);
    void createImageView(
        const ImageViewCreateInfo *pCreateInfo,
        VkImageView *pImageView);
    void createImage(
        const ImageCreateInfo *pCreateInfo,
        VkImage *pImage,
        VkDeviceMemory *pImageMemory);
    struct EVkUniformBufferUpdateInfo
    {
        uint32_t currentImage;
        VkExtent2D swapchainExtent;
        std::vector<VkDeviceMemory> *pUniformBufferMemory;
    };
    std::vector<char> readFile(const std::string& filename);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void addDescriptorPoolSize(const VkDescriptorType type);
    void addDescriptorSetBinding(const VkDescriptorType type, uint32_t binding, VkShaderStageFlagBits stage);
    void addWriteDescriptorSetBuffer(
        std::vector<VkBuffer> buffers, VkDeviceSize offset, VkDeviceSize range,
        uint32_t binding, VkDescriptorType type, size_t startIndex);
    void addWriteDescriptorSetTextureSampler(VkImageView textureView, VkSampler textureSampler, uint32_t binding);

    std::vector<const char *> m_deviceExtensions;
    std::vector<const char *> m_validationLayers;
    uint8_t m_maxFramesInFlight;
    GLFWwindow *m_window;
};

} // namespace evk
#endif