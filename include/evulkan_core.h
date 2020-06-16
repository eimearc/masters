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

#define GLFW_INCLUDE_VULKAN

namespace evk
{

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

struct EVkImageCreateInfo
{
    VkPhysicalDevice physicalDevice;
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags properties;
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

    void createSwapChain(const SwapChainCreateInfo *pCreateInfo);
    void createRenderPass();
    void registerVertexShader(const std::string &vertShader);
    void registerFragmentShader(const std::string &fragShader);
    void addVertexAttribute(const uint32_t &location, const uint32_t &offset);
    void setBindingDescription(uint32_t stride);
    void createGraphicsPipeline();
    void createDepthResources();
    void createUniformBufferObject();
    void createDescriptorSets();
    void createSyncObjects();
    void createFramebuffers();
    void createCommandPools();
    void createIndexBuffer(const std::vector<uint32_t> &indices);
    void createVertexBuffer(const std::vector<Vertex> &vertices);
    void createDrawCommands(const std::vector<uint32_t> &indices);
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

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

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

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    VkVertexInputBindingDescription m_bindingDescription;

    VkImage m_textureImage;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
    VkDeviceMemory m_textureImageMemory;

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
    VkImageView createImageView(
        VkImage image,
        VkFormat format,
        VkImageAspectFlags aspectFlags);
    void createImage(
        const EVkImageCreateInfo *pCreateInfo,
        VkImage *pImage,
        VkDeviceMemory *pImageMemory);
    struct EVkUniformBufferUpdateInfo
    {
        uint32_t currentImage;
        VkExtent2D swapchainExtent;
        std::vector<VkDeviceMemory> *pUniformBufferMemory;
    };
    void updateUniformBuffer(const EVkUniformBufferUpdateInfo *pUpdateInfo);
    std::vector<char> readFile(const std::string& filename);
    UniformBufferObject getUBO(const uint32_t &_width, const uint32_t &_height);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    std::vector<const char *> m_deviceExtensions;
    std::vector<const char *> m_validationLayers;
    uint8_t m_maxFramesInFlight;
    GLFWwindow *m_window;
};

} // namespace evk
#endif