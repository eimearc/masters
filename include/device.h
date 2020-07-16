#ifndef EVK_DEVICE
#define EVK_DEVICE

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
    ~Device()=default;

    Device(
        const uint32_t &num_threads,
        const std::vector<const char*> &validation_layers,
        GLFWwindow *window,
        const std::vector<const char *> &device_extensions,
        const uint32_t &swapchain_size,
        const bool &enable_validation
    );

    // Device.
    GLFWwindow* window() const { return m_device->m_window; }
    VkInstance instance() const { return m_device->m_instance; }
    VkSurfaceKHR surface() const { return m_device->m_surface; }
    VkPhysicalDevice physicalDevice() const { return m_device->m_physicalDevice; }
    VkDevice device() const { return m_device->m_device; }
    VkFormat depthFormat() const { return m_device->m_depthFormat; };
    VkQueue graphicsQueue() const { return m_device->m_graphicsQueue; };
    VkQueue presentQueue() const { return m_device->m_presentQueue; };
    uint32_t numThreads() const { return m_numThreads; };
    std::vector<std::unique_ptr<Thread>>& threads() { return m_threadPool.threads; }; // TODO: Check if this is right.
    void wait() { m_threadPool.wait(); };

    // Swapchain.
    VkExtent2D extent() const { return m_swapchain->m_extent; };
    VkSwapchainKHR swapchain() const { return m_swapchain->m_swapchain; };
    uint32_t swapchainSize() const { return m_swapchain->m_images.size(); };
    std::vector<VkImageView> swapchainImageViews() const { return m_swapchain->m_imageViews; };

    // Commands.
    std::vector<VkCommandPool> commandPools() const { return m_commands->m_commandPools; };
    std::vector<VkCommandBuffer> primaryCommandBuffers() const { return m_commands->m_primaryCommandBuffers; };
    std::vector<VkCommandBuffer> secondaryCommandBuffers() const { return m_commands->m_secondaryCommandBuffers; };

    // Sync.
    std::vector<VkFence> frameFences() const { return m_sync->m_fencesInFlight; };
    std::vector<VkFence> imageFences() const { return m_sync->m_imagesInFlight; };
    std::vector<VkSemaphore> imageSempahores() const { return m_sync->m_imageAvailableSemaphores; };
    std::vector<VkSemaphore> renderSempahores() const { return m_sync->m_renderFinishedSemaphores; };

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
            GLFWwindow *window,
            const uint32_t swapchainSize
        );

        VkDevice m_device;
        VkSwapchainKHR m_swapchain;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        VkFormat m_format;
        VkExtent2D m_extent;
    };

    class Commands
    {
        public:
        Commands()=default;
        Commands(const Commands&)=default;
        Commands& operator=(const Commands&)=default;
        Commands(Commands&&)=default;
        Commands& operator=(Commands&&)=default;
        ~Commands() noexcept;

        Commands(
            const VkDevice &device,
            const VkPhysicalDevice &physicalDevice,
            const VkSurfaceKHR &surface,
            const uint32_t &swapchainSize,
            const uint32_t &numThreads
        );

        VkDevice m_device;
        std::vector<VkCommandPool> m_commandPools;
        std::vector<VkCommandBuffer> m_primaryCommandBuffers;
        std::vector<VkCommandBuffer> m_secondaryCommandBuffers;
    };

    class Sync
    {
        public:
        Sync()=default;
        Sync(const Sync&)=default;
        Sync& operator=(const Sync&)=default;
        Sync(Sync&&)=default;
        Sync& operator=(Sync&&)=default;
        ~Sync() noexcept;

        Sync(const VkDevice &device, const uint32_t &swapchainSize);

        VkDevice m_device;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_fencesInFlight;
        std::vector<VkFence> m_imagesInFlight;
    };
    
    std::unique_ptr<_Device> m_device;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Commands> m_commands;
    std::unique_ptr<Sync> m_sync;
    ThreadPool m_threadPool;
    size_t m_numThreads;
};

#endif