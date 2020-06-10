#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include "util.h"
#include "flags.h"
#include "bench.h"
#include "threadpool.h"

#include "evulkan_util.h"

DECLARE_bool(enable_validation);

struct EVkCreateWindow
{
    bool resizeable;
    std::string title;
    int width;
    int height;
};
void evkCreateWindow(
    const EVkCreateWindow params,
    GLFWwindow *&window
);

struct EVkCreateInstance
{
    const char* appTitle;
    std::vector<const char*> extensions;
    std::vector<const char*> validationLayers;
};
void evkCreateInstance(
    const EVkCreateInstance *pCreateInfo,
    VkInstance *instance
);

void evkSetupDebugMessenger(
    VkInstance instance,
    VkDebugUtilsMessengerEXT *pDebugMessenger
);

struct EVkSurfaceCreate
{
    GLFWwindow *window;
};
void evkCreateSurface(
    VkInstance instance,
    const EVkSurfaceCreate *pCreateInfo,
    VkSurfaceKHR *surface
);

struct EVkPickPhysicalDevice
{
    VkSurfaceKHR surface;
    std::vector<const char *> deviceExtensions;
};
void evkPickPhysicalDevice(
    VkInstance instance,
    const EVkPickPhysicalDevice *pPickInfo,
    VkPhysicalDevice *physicalDevice
);

struct EVkDeviceCreateInfo
{
    VkSurfaceKHR surface;
    std::vector<const char *> deviceExtensions;
    std::vector<const char *> validationLayers;
};

struct EVkSwapchainCreateInfo
{
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    GLFWwindow* window;
    uint32_t numImages;
};

void evkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const EVkDeviceCreateInfo *pCreateInfo,
    VkDevice *pDevice,
    VkQueue *pGraphicsQueue,
    VkQueue *pPresentQueue);

void evkCreateSwapchain(
    VkDevice device,
    const EVkSwapchainCreateInfo *pCreateInfo,
    VkSwapchainKHR *pSwapchain,
    std::vector<VkImage> *pSwapchainImages,
    VkFormat *pSwapchainImageFormat,
    VkExtent2D *pSwapchainExtent);

struct EVkImageViewsCreateInfo
{
    std::vector<VkImage> images;
    VkFormat swapChainImageFormat;
};

void evkCreateImageViews
(
    VkDevice device,
    const EVkImageViewsCreateInfo *pCreateInfo,
    std::vector<VkImageView> *pSwapChainImageViews
);
VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

struct EVkRenderPassCreateInfo
{
    VkFormat swapChainImageFormat;
    VkPhysicalDevice physicalDevice;
};
void evkCreateRenderPass(
    VkDevice device,
    const EVkRenderPassCreateInfo *pCreateInfo,
    VkRenderPass *pRenderPass);
VkFormat findDepthFormat(const EVkRenderPassCreateInfo *pCreateInfo);
VkFormat findSupportedFormat(
    const EVkRenderPassCreateInfo *pCreateInfo,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features);

struct EVkDescriptorSetLayoutCreateInfo
{

};
void evkCreateDescriptorSetLayout(
    VkDevice device,
    const EVkDescriptorSetLayoutCreateInfo *pCreateInfo,
    VkDescriptorSetLayout *pDescriptorSetLayout
);

struct EVkGraphicsPipelineCreateInfo
{
    std::string vertShaderFile;
    std::string fragShaderFile;
    VkExtent2D swapchainExtent;
    VkDescriptorSetLayout *pDescriptorSetLayout;
    VkRenderPass renderPass;
};
void evkCreateGraphicsPipeline(
    VkDevice device,
    const EVkGraphicsPipelineCreateInfo *pCreateInfo,
    VkPipelineLayout *pPipelineLayout,
    VkPipeline *pPipeline);

struct EVkDepthResourcesCreateInfo
{
    VkFormat swapchainImageFormat;
    VkPhysicalDevice physicalDevice;
    VkExtent2D swapchainExtent;
};
void evkCreateDepthResources(
    VkDevice device,
    const EVkDepthResourcesCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkImageView *pImageView,
    VkDeviceMemory *pImageMemory
);

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
void evkCreateImage(
    VkDevice device,
    const EVkImageCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory);

struct EVkFramebuffersCreateInfo
{
    std::vector<VkImageView> swapchainImageViews;
    VkExtent2D swapchainExtent;
    VkImageView depthImageView;
    VkRenderPass renderPass;
};
void evkCreateFramebuffers
(
    VkDevice device,
    const EVkFramebuffersCreateInfo *pCreateInfo,
    std::vector<VkFramebuffer> *pFramebuffers
);

struct EVkCommandPoolCreateInfo
{
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkCommandPoolCreateFlags flags;
};
void evkCreateCommandPool(
    VkDevice device,
    const EVkCommandPoolCreateInfo *pCreateInfo,
    VkCommandPool *pCommandPool);

struct EVkIndexBufferCreateInfo
{
    std::vector<uint32_t> indices;
    VkPhysicalDevice physicalDevice;
    VkQueue queue;
    VkCommandPool commandPool;
};
void evkCreateIndexBuffer(
    VkDevice device,
    const EVkIndexBufferCreateInfo *pCreateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
);
// void createBuffer(
//     VkDevice device,
//     VkPhysicalDevice physicalDevice,
//     VkDeviceSize size,
//     VkBufferUsageFlags usage,
//     VkMemoryPropertyFlags properties,
//     VkBuffer *pBuffer,
//     VkDeviceMemory *pBufferMemory);
// void copyBuffer(
//     VkDevice device,
//     VkCommandPool commandPool,
//     VkQueue queue,
//     VkBuffer srcBuffer,
//     VkBuffer dstBuffer,
//     VkDeviceSize size);
// void beginSingleTimeCommands(
//     VkDevice device,
//     VkCommandPool commandPool,
//     VkCommandBuffer *pCommandBuffer);
// void endSingleTimeCommands(
//     VkDevice device,
//     VkQueue queue,
//     VkCommandPool commandPool,
//     VkCommandBuffer commandBuffer);

struct EVkUniformBufferCreateInfo
{
    VkPhysicalDevice physicalDevice;
    std::vector<VkImage> swapchainImages;  
};
void evkCreateUniformBuffers(
    VkDevice device,
    const EVkUniformBufferCreateInfo *pCreateInfo,
    std::vector<VkBuffer> *pBuffer,
    std::vector<VkDeviceMemory> *pBufferMemory
);

struct EVkDescriptorPoolCreateInfo
{
    std::vector<VkImage> swapchainImages;
};
void evkCreateDescriptorPool(
    VkDevice device,
    const EVkDescriptorPoolCreateInfo *pCreateInfo,
    VkDescriptorPool *pDescriptorPool
);

struct EVkDescriptorSetCreateInfo
{
    std::vector<VkImage> swapchainImages;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkBuffer> uniformBuffers;
};
void evkCreateDescriptorSets(
    VkDevice device,
    const EVkDescriptorSetCreateInfo *pCreateInfo,
    std::vector<VkDescriptorSet> *pDescriptorSets
);

struct EVkCommandBuffersCreateInfo
{
    VkFramebuffer framebuffer;
    VkCommandPool commandPool;
    EVkCommandPoolCreateInfo poolCreateInfo;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    VkExtent2D swapchainExtent;
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    std::vector<uint32_t> indices;
};

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<VkCommandBuffer> *pCommandBuffers,
    const std::vector<VkCommandPool> *pCommandPools,
    ThreadPool &threadpool
);

struct EVkSyncObjectsCreateInfo
{
    size_t maxFramesInFlight;
    size_t swapchainSize;
};
void evkCreateSyncObjects(
    VkDevice device,
    const EVkSyncObjectsCreateInfo *pCreateInfo,
    std::vector<VkSemaphore> *pImageAvailableSemaphores,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    std::vector<VkFence> *pFencesInFlight,
    std::vector<VkFence> *pImagesInFlight
);

struct EVkDrawFrameInfo
{
    // VkPhysicalDevice physicalDevice;
    std::vector<VkFence> *pInFlightFences;
    std::vector<VkSemaphore> *pImageAvailableSemaphores;
    size_t maxFramesInFlight;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    VkExtent2D swapchainExtent;
    // std::vector<VkFramebuffer> framebuffers;
    std::vector<VkDeviceMemory> *pUniformBufferMemory;
    // std::vector<VkCommandPool> commandPools;
};
void evkDrawFrame(
    VkDevice device,
    const EVkDrawFrameInfo *pDrawInfo,
    size_t *pCurrentFrame,
    std::vector<VkFence> *pImagesInFlight,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    VkCommandBuffer *pPrimaryCommandBuffer
);

struct EVkSwapchainRecreateInfo
{
    GLFWwindow *pWindow;
    VkSwapchainKHR *pSwapchain;
    std::vector<VkImage> *pSwapchainImages;
    std::vector<VkImageView> *pSwapchainImageViews;
    VkFormat *pSwapchainImageFormats;
    VkExtent2D *pSwapchainExtent;
    VkRenderPass *pRenderPass;
    VkPipelineLayout *pPipelineLayout;
    VkPipeline *pPipeline;
    VkImage *pDepthImage;
    VkImageView *pDepthImageView;
    VkDeviceMemory *pDepthImageMemory;
    std::vector<VkFramebuffer> *pSwapchainFramebuffers;
    std::vector<VkBuffer> *pUniformBuffers;
    std::vector<VkDeviceMemory> *pUniformBuffersMemory;
    VkDescriptorPool *pDescriptorPool;
    std::vector<VkDescriptorSet> *pDescriptorSets;
    std::vector<VkCommandBuffer> *pCommandBuffers;
    VkCommandBuffer *pPrimaryCommandBuffer;

    EVkSwapchainCreateInfo swapchainCreateInfo;
    EVkImageViewsCreateInfo imageViewsCreateInfo;
    EVkRenderPassCreateInfo renderPassCreateInfo;
    EVkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    EVkDepthResourcesCreateInfo depthResourcesCreateInfo;
    EVkFramebuffersCreateInfo framebuffersCreateInfo;
    EVkUniformBufferCreateInfo uniformBuffersCreateInfo;
    EVkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
    EVkDescriptorSetCreateInfo EVkDescriptorSetCreateInfo;
    EVkCommandBuffersCreateInfo commandBuffersCreateInfo;
};
void evkRecreateSwapchain(
    VkDevice device,
    const EVkSwapchainRecreateInfo *pCreateInfo,
    ThreadPool &threadpool
);

struct EVkSwapchainCleanupInfo
{
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> *pCommandBuffers;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    std::vector<VkImageView> swapchainImageViews;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
};
void evkCleanupSwapchain(VkDevice device, const EVkSwapchainCleanupInfo *pCleanupInfo);

struct EVkUniformBufferUpdateInfo
{
    uint32_t currentImage;
    VkExtent2D swapchainExtent;
    std::vector<VkDeviceMemory> *pUniformBufferMemory;
};
void evkUpdateUniformBuffer(VkDevice device, const EVkUniformBufferUpdateInfo *pUpdateInfo);

struct EVkVertexBufferCreateInfo
{
   std::vector<Vertex> *pVertices;
   VkPhysicalDevice physicalDevice;
   VkCommandPool commandPool;
   std::vector<VkCommandPool> commandPools;
   VkQueue graphicsQueue;
   VkBuffer vertexBuffer;
};
void evkCreateVertexBuffer(
    VkDevice device,
    const EVkVertexBufferCreateInfo *pUpdateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory,  
    ThreadPool &threadpool);