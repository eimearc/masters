#ifndef EVK_DEVICE
#define EVK_DEVICE

#include "command.h"
#include "sync.h"
#include "threadpool.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Device
{
    public:
    Device()=default;
    Device(const Device&)=delete;
    Device& operator=(const Device&)=delete;
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

    // Device.
    GLFWwindow* window() const { return m_innerDevice->m_window; }
    VkInstance instance() const { return m_innerDevice->m_instance; }
    VkSurfaceKHR surface() const { return m_innerDevice->m_surface; }
    VkPhysicalDevice physicalDevice() const { return m_innerDevice->m_physicalDevice; }
    VkDevice device() const { return m_innerDevice->m_device; }
    VkFormat depthFormat() const { return m_innerDevice->m_depthFormat; };
    VkQueue graphicsQueue() const { return m_innerDevice->m_graphicsQueue; };
    VkQueue presentQueue() const { return m_innerDevice->m_presentQueue; };

    // Swapchain.
    VkExtent2D extent() const { return m_swapchain->m_extent; };
    VkSwapchainKHR swapchain() const { return m_swapchain->m_swapchain; };
    uint32_t swapchainSize() const { return m_swapchain->m_images.size(); };
    std::vector<VkImageView> swapchainImageViews() const { return m_swapchain->m_imageViews; };
    
    ThreadPool m_threadPool;
    size_t m_numThreads;

    Sync m_sync;
    Commands m_commands;

    private:
    class _Device
    {
        public:
        _Device()=default;
        _Device(const _Device&)=default;
        _Device& operator=(const _Device&)=default;
        _Device(_Device&&)=default;
        _Device& operator=(_Device&&)=default;
        ~_Device() noexcept;

        _Device(
            const uint32_t &num_threads,
            const std::vector<const char*> &validation_layers,
            GLFWwindow *window,
            const std::vector<const char *> &device_extensions,
            const bool &enable_validation
        );

        VkInstance m_instance;
        VkSurfaceKHR m_surface;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;
        GLFWwindow *m_window;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkFormat m_depthFormat;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;

        private:
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

    class Swapchain
    {
        public:
        Swapchain()=default;
        Swapchain(const Swapchain&)=default;
        Swapchain& operator=(const Swapchain&)=default;
        Swapchain(Swapchain&&)=default;
        Swapchain& operator=(Swapchain&&)=default;
        ~Swapchain() noexcept;

        Swapchain(
            const VkDevice &device,
            const VkPhysicalDevice &physicalDevice,
            const VkSurfaceKHR &surface,
            GLFWwindow *window, //TODO: Make const.
            const uint32_t swapchainSize
        );

        VkDevice m_device;
        VkSwapchainKHR m_swapchain;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        VkFormat m_format;
        VkExtent2D m_extent;
    };
    
    std::unique_ptr<_Device> m_innerDevice;
    std::unique_ptr<Swapchain> m_swapchain;
};

#endif