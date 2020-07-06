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

#include "attachment.h"
#include "buffer.h"
#include "command.h"
#include "descriptor.h"
#include "device.h"
#include "pass.h"
#include "pipeline.h"
#include "shader.h"
#include "swapchain.h"
#include "texture.h"

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

struct EVkRenderPassCreateInfo
{
    VkFormat swapChainImageFormat;
    VkPhysicalDevice physicalDevice;
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

    void createSyncObjects(const Swapchain &swapchain);
    void createFramebuffers(
        const std::vector<Attachment> &attachments,
        const Renderpass &renderpass,
        const Swapchain &swapchain
    );
    void createCommandPools();

    void createDrawCommands(
        const Buffer &indexBuffer,
        const Buffer &vertexBuffer,
        const std::vector<Descriptor> &descriptors,
        const std::vector<Pipeline> &pipelines,
        const Renderpass &renderpass,
        const Swapchain &swapchain,
        Commands &commands
    );
    void draw(
        const std::vector<Pipeline> &pipelines,
        const Swapchain &swapchain,
        const Commands &commands);
    void cleanup();

    ThreadPool m_threadPool;
    size_t m_numThreads;

    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkDevice m_device;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_fencesInFlight;
    std::vector<VkFence> m_imagesInFlight;

    std::vector<VkFramebuffer> m_framebuffers;

    // std::vector<VkCommandPool> m_commandPools;
    // std::vector<VkCommandBuffer> m_primaryCommandBuffers;
    // std::vector<VkCommandBuffer> m_secondaryCommandBuffers;

    uint32_t m_maxFramesInFlight;

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
    VkFormat findSupportedFormat(
        const EVkRenderPassCreateInfo *pCreateInfo,
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features);

    std::vector<const char *> m_deviceExtensions;
    std::vector<const char *> m_validationLayers;
    GLFWwindow *m_window;
};

} // namespace evk
#endif