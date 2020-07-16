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
    Device(const Device&)=default;
    Device& operator=(const Device&)=default;
    Device(Device&&)=default;
    Device& operator=(Device&&)=default;
    ~Device() noexcept;

    Device(
        const uint32_t &num_threads,
        const std::vector<const char*> &validation_layers,
        GLFWwindow *window,
        const std::vector<const char *> &device_extensions,
        const uint32_t &swapchain_size,
        const bool &enable_validation
    );

    GLFWwindow* window() const { return m_window; }
    VkInstance instance() const { return m_instance; }
    VkSurfaceKHR surface() const { return m_surface; }
    VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
    VkDevice device() const { return m_device; }
    
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    ThreadPool m_threadPool;
    size_t m_numThreads;
    VkFormat m_depthFormat;

    // Need some sort of internal device for dtor.
    // _Device m_device;
    Swapchain m_swapchain;
    Sync m_sync;
    Commands m_commands;

    private:
    class _Device
    {
        public:
        ~_Device(){print();};
        void print(){std::cout << "Hello from _Device.\n";};
    };
    
    _Device m_innerDevice;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    GLFWwindow *m_window;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    void createInstance(
        std::vector<const char*> validationLayers
    );
    void createSurface(
        GLFWwindow *window
    );
    void pickPhysicalDevice(
        std::vector<const char *> deviceExtensions
    );
    void createDevice(
        bool enableValidation,
        const std::vector<const char *> &device_extensions,
        const std::vector<const char*> &validation_layers
    );
    VkResult createDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    );
    void setDepthFormat();
};

#endif