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
#include "descriptor.h"
#include "buffer.h"
#include "device.h"

#define GLFW_INCLUDE_VULKAN

namespace evk
{

const std::string FRAMEBUFFER_ATTACHMENT = "framebuffer";

struct Attachment
{
    std::string name;
    uint32_t index;
    VkAttachmentDescription description;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkDeviceMemory> imageMemories;
};

struct BufferInfo
{
    size_t index;
    size_t size;
};

struct Pipeline
{
    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<std::string> m_shaders;
    Descriptor m_descriptor;
    VertexInput m_vertexInput;
    uint32_t m_subpass;
};

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

struct SubpassDependency
{
    uint32_t srcSubpass;
    uint32_t dstSubpass;
};

struct SubpassDescription
{
    std::vector<VkAttachmentReference> colorAttachments;
    std::vector<VkAttachmentReference> depthAttachments;
    std::vector<VkAttachmentReference> inputAttachments;
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

        VkAttachmentDescription description;
        description.format = VK_FORMAT_B8G8R8A8_SRGB;
        description.flags = 0;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        Attachment attachment;
        attachment.name=evk::FRAMEBUFFER_ATTACHMENT;
        attachment.description=description;
        addAttachment(attachment);

        createInstance(pCreateInfo->validationLayers);
        createSurface(pCreateInfo->window);
        pickPhysicalDevice(pCreateInfo->deviceExtensions);
        createDevice(true);
    }
    Instance()=default;

    void updateBuffer(const std::string &name);

    void createSwapChain(const SwapChainCreateInfo *pCreateInfo);
    void createRenderPass();
    void registerVertexShader(const std::string &name, const std::string &vertShader);
    void registerFragmentShader(const std::string &name, const std::string &fragShader);

    void addColorAttachment(const std::string &name);
    void addDepthAttachment(const std::string &name);

    void addSubpass(
        const std::vector<SubpassDependency> &dependencies,
        const std::vector<std::string> &c,
        const std::vector<std::string> &d,
        const std::vector<std::string> &i);

    void addPipeline(
        const std::vector<std::string> &shaders,
        Descriptor &descriptor,
        VertexInput &vertexInput,
        uint32_t subpass);
    void createGraphicsPipeline();

    void createSyncObjects();
    void createFramebuffers();
    void createCommandPools();
    void createIndexBuffer(const std::vector<Index> &indices);
    void createVertexBuffer(const std::vector<Vertex> &vertices);
    void createDrawCommands(const Buffer &indexBuffer, const size_t numIndicesOuter);
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
    std::map<std::string, VkPipelineShaderStageCreateInfo> m_shaders;
    std::vector<VkPipelineLayout> m_pipelineLayouts;
    std::vector<VkPipeline> m_pipelines;

    std::vector<Pipeline> m_evkpipelines;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkDeviceMemory> m_imageMemories;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_fencesInFlight;
    std::vector<VkFence> m_imagesInFlight;

    std::vector<VkFramebuffer> m_framebuffers;

    std::vector<VkCommandPool> m_commandPools;
    std::vector<VkCommandBuffer> m_primaryCommandBuffers;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffers;

    std::map<std::string, BufferInfo> m_bufferMap;
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;

    std::vector<SubpassDescription> m_subpasses;
    std::vector<VkSubpassDependency> m_dependencies;

    std::map<std::string,Attachment> m_evkattachments;

    std::vector<Index> m_indices;

    VkImage m_textureImage=VK_NULL_HANDLE;
    VkImageView m_textureImageView=VK_NULL_HANDLE;
    VkSampler m_textureSampler=VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory=VK_NULL_HANDLE;
    VkDescriptorImageInfo m_textureDescriptor;

    void addAttachment(Attachment attachment);

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

    void addDependency(uint32_t srcSubpass, uint32_t dstSubpass);

    std::vector<const char *> m_deviceExtensions;
    std::vector<const char *> m_validationLayers;
    uint8_t m_maxFramesInFlight;
    GLFWwindow *m_window;
};

} // namespace evk
#endif