#ifndef EVK_DEVICE
#define EVK_DEVICE

#include "command.h"
#include "swapchain.h"
#include "sync.h"
#include "threadpool.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Device
{
    public:
    Device()=default;
    Device(
        uint32_t numThreads,
        std::vector<const char*> validationLayers,
        GLFWwindow *window,
        std::vector<const char *> deviceExtensions,
        uint32_t swapchainSize
    );
    void destroy();

    GLFWwindow* window() const { return m_window; }
    VkInstance instance() const { return m_instance; }
    VkSurfaceKHR surface() const { return m_surface; }
    VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
    VkDevice device() const { return m_device; }
    
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    ThreadPool m_threadPool;
    size_t m_numThreads;
    VkFormat m_depthFormat;

    Swapchain m_swapchain;
    Sync m_sync;
    Commands m_commands;

    private:
    std::vector<const char *> m_deviceExtensions;
    std::vector<const char *> m_validationLayers;
    GLFWwindow *m_window;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;

    void createInstance(std::vector<const char*> validationLayers);
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice(std::vector<const char *> deviceExtensions);
    void createDevice(bool enableValidation);
    VkResult createDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
    void setDepthFormat();
    VkFormat getSupportedFormat(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features);
};

#endif