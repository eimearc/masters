#ifndef DEVICE
#define DEVICE

#include <vulkan/vulkan.h>
#include "threadpool.h"
#include "util.h"

class Device
{
    public:
    Device()=default;
    Device(
        uint32_t numThreads,
        std::vector<const char*> validationLayers,
        GLFWwindow *window,
        std::vector<const char *> deviceExtensions
    );
    void destroy();

    std::vector<const char *> m_deviceExtensions;
    std::vector<const char *> m_validationLayers;
    GLFWwindow *m_window;
    
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkDevice m_device;
    ThreadPool m_threadPool;
    size_t m_numThreads;
    VkFormat m_depthFormat;

    private:
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