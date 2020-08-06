#ifndef EVK_DEVICE_H_
#define EVK_DEVICE_H_

#include "threadpool.h"
#include "util.h"
#include <vulkan/vulkan.h>

namespace evk {
    
class Buffer;
class Pipeline;
class Renderpass;

/**
 * @class Device
 * @brief A Device is the basic component used to generate other components.
 * 
 * A Device encaspluates basic Vulkan objects, including a VkInstance,
 * VkDevice, VkCommandBuffers, VkFences, VkSemaphores, VkFramebuffer,
 * and VkSwapchain. Is is the first Vulkan object that is created in
 * the program and is used in the generation of most other objects.
 * It contains the following objects:
 * 
 * Commands: represents the VkCommandBuffers that are used to record and submit
 *  items of work to the GPU.
 * Swapchain: holds all the VkImages required by the program and the VkImages
 *  used to blit images to the screen.
 * Sync: generates the synchronization objects needed by the Vulkan program,
 *  including the VkFences and VkSemaphores, ensuring that host-device
 *  synchronization is properly set up.
 * Framebuffer: holds the VkFramebuffer required to blit images to the screen.
 * 
 * @example
 * Device device(
 *  1, window, extensions, 2, layers
 * );
 **/ 
class Device
{
    public:
    Device()=default;
    Device(const Device&)=delete; // Class Device is non-copyable.
    Device& operator=(const Device&)=delete; // Class Device is non-copyable.
    Device(Device&&) noexcept;
    Device& operator=(Device&&) noexcept;
    ~Device()=default;

    /**
     * Constructs a Device without validation layers.
     * @param[in] numThreads the number of threads the Device should use.
     * @param[in] window a pointer to the window used to display images.
     * @param[in] deviceExtensions the extensions required to create the device.
     * @param[in] swapchainSize the number of images used by the swapchain,
     *  generally between two and three.
     */
    Device(
        uint32_t numThreads,
        GLFWwindow *window,
        const std::vector<const char *> &deviceExtensions,
        const uint32_t swapchainSize
    );

    /**
     * Constructs a Device with validation layers turned on. This is useful for
     *  developing programs.
     * @param[in] numThreads the number of threads the Device should use.
     * @param[in] window a pointer to the window used to display images.
     * @param[in] deviceExtensions the extensions required to create the device.
     * @param[in] swapchainSize the number of images used by the swapchain,
     *  generally between two and three.
     * @param[in] validationLayers the layers to use for validation.
     */
    Device(
        uint32_t numThreads,
        GLFWwindow *window,
        const std::vector<const char *> &deviceExtensions,
        uint32_t swapchainSize,
        const std::vector<const char*> &validationLayers
    );

    bool operator==(const Device& other);

    /**
     * Finalize the device. This is the last function that is called before
     * draw().
     * @param[in] indexBuffer the index buffer.
     * @param[in] vertexBuffer the vertex buffer.
     * @param[in] pipelines the set of pipelines used for drawing.
     **/
    void finalize(
        Buffer &indexBuffer,
        Buffer &vertexBuffer,
        std::vector<Pipeline*> &pipelines
    );

    /**
     * Draw.
     **/
    void draw();

    private:
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
    std::vector<std::unique_ptr<Thread>>& threads() { return m_threadPool.threads; };

    void record();
    void resizeScreen();
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
    std::vector<VkFence>& frameFences() const { return m_sync->m_fencesInFlight; };
    std::vector<VkFence>& imageFences() const { return m_sync->m_imagesInFlight; };
    std::vector<VkSemaphore>& imageSempahores() const { return m_sync->m_imageAvailableSemaphores; };
    std::vector<VkSemaphore>& renderSempahores() const { return m_sync->m_renderFinishedSemaphores; };

    // Framebuffers.
    std::vector<VkFramebuffer> framebuffers() const { return m_framebuffer->m_framebuffers; };

    class _Device
    {
        public:
        _Device()=default;
        _Device(const _Device&)=delete; // Class _Device is non-copyable.
        _Device& operator=(const _Device&)=delete; // Class _Device is non-copyable.
        _Device(_Device&&) noexcept;
        _Device& operator=(_Device&&) noexcept;
        ~_Device() noexcept;

        _Device(
            uint32_t num_threads,
            const std::vector<const char*> &validation_layers,
            GLFWwindow *window,
            const std::vector<const char *> &device_extensions
        );

        bool operator==(const _Device &other);

        VkDebugUtilsMessengerEXT m_debugMessenger=VK_NULL_HANDLE;
        VkFormat m_depthFormat;
        VkDevice m_device=VK_NULL_HANDLE;
        VkQueue m_graphicsQueue=VK_NULL_HANDLE;
        VkInstance m_instance=VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice=VK_NULL_HANDLE;
        VkQueue m_presentQueue=VK_NULL_HANDLE;
        VkSurfaceKHR m_surface=VK_NULL_HANDLE;
        GLFWwindow *m_window=nullptr;

        private:
        void createInstance(
            const std::vector<const char*> &validationLayers
        );
        void createSurface(
            GLFWwindow *window
        );
        void pickPhysicalDevice(
            const std::vector<const char *> &deviceExtensions
        );
        void createDevice(
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
        bool isDeviceSuitable(
            VkPhysicalDevice device,
            VkSurfaceKHR surface,
            std::vector<const char *> deviceExtensions
        );
        bool checkDeviceExtensionSupport(
            VkPhysicalDevice device,
            std::vector<const char *> deviceExtensions
        );
        void destroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        );
        void debugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT& createInfo
        );
        internal::QueueFamilyIndices getQueueFamilies(
            VkPhysicalDevice device,
            VkSurfaceKHR surface
        );
    };

    class Swapchain
    {
        public:
        Swapchain()=default;
        Swapchain(const Swapchain&)=delete; // Class Swapchain is non-copyable.
        Swapchain& operator=(const Swapchain&)=delete; // Class Swapchain is non-copyable.
        Swapchain(Swapchain&&) noexcept;
        Swapchain& operator=(Swapchain&&) noexcept;
        ~Swapchain() noexcept;

        Swapchain(
            const VkDevice &device,
            const VkPhysicalDevice &physicalDevice,
            const VkSurfaceKHR &surface,
            GLFWwindow *window,
            const uint32_t swapchainSize
        );

        bool operator==(const Swapchain &other) const;
        bool operator!=(const Swapchain &other) const;
        void reset() noexcept;
        void recreate();
        void setup() noexcept;

        VkExtent2D chooseSwapExtent(
            GLFWwindow* window,
            const VkSurfaceCapabilitiesKHR& capabilities
        ) const;
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes
        ) const;
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats
        ) const;
        void destroy() noexcept;

        VkDevice m_device=VK_NULL_HANDLE;
        VkExtent2D m_extent;
        VkFormat m_format;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        VkSwapchainKHR m_swapchain=VK_NULL_HANDLE;

        uint32_t m_swapchainSize;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

        GLFWwindow *m_window;
    };

    class Commands
    {
        public:
        Commands()=default;
        Commands(const Commands&)=delete; // Class Commands is non-copyable.
        Commands& operator=(const Commands&)=delete; // Class Commands is non-copyable.
        Commands(Commands&&) noexcept;
        Commands& operator=(Commands&&) noexcept;
        ~Commands() noexcept;

        Commands(
            const VkDevice &device,
            const VkPhysicalDevice &physicalDevice,
            const VkSurfaceKHR &surface,
            const uint32_t &swapchainSize,
            const uint32_t &numThreads
        );

        bool operator==(const Commands &other);
        void reset() noexcept;

        std::vector<VkCommandPool> m_commandPools;
        VkDevice m_device=VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_primaryCommandBuffers;
        std::vector<VkCommandBuffer> m_secondaryCommandBuffers;
    };

    class Sync
    {
        public:
        Sync()=default;
        Sync(const Sync&)=delete; // Class Sync is non-copyable.
        Sync& operator=(const Sync&)=delete; // Class Sync is non-copyable.
        Sync(Sync&&) noexcept;
        Sync& operator=(Sync&&) noexcept;
        ~Sync() noexcept;

        Sync(const VkDevice &device, const uint32_t &swapchainSize);

        bool operator==(const Sync &other);
        void reset() noexcept;

        VkDevice m_device=VK_NULL_HANDLE;
        std::vector<VkFence> m_fencesInFlight;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkFence> m_imagesInFlight;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
    };

    class Framebuffer
    {
        public:
        Framebuffer()=default;
        Framebuffer(const Framebuffer&)=delete; // Class Framebuffer is non-copyable.
        Framebuffer& operator=(const Framebuffer&)=delete; // Class Framebuffer is non-copyable.
        Framebuffer(Framebuffer&&) noexcept;
        Framebuffer& operator=(Framebuffer&&) noexcept;
        ~Framebuffer() noexcept;

        Framebuffer(
            Device &device,
            size_t swapchainSize,
            const std::vector<VkImageView> &swapchainImageViews,
            Renderpass &renderpass
        );

        bool operator==(const Framebuffer &other);
        void recreate(
            const std::vector<VkImageView> &swapchainImageViews
        );
        void setup(
            const std::vector<VkImageView> &swapchainImageViews
        );

        // VkDevice m_device=VK_NULL_HANDLE;
        Device *m_device=nullptr;
        std::vector<VkFramebuffer> m_framebuffers;
        Renderpass *m_renderpass=nullptr;
        size_t m_swapchainSize;
    };
    
    std::unique_ptr<_Device> m_device=nullptr;
    std::unique_ptr<Commands> m_commands=nullptr;
    std::unique_ptr<Framebuffer> m_framebuffer=nullptr;
    size_t m_numThreads=1;
    std::unique_ptr<Swapchain> m_swapchain=nullptr;
    std::unique_ptr<Sync> m_sync=nullptr;
    ThreadPool m_threadPool;

    Buffer *m_indexBuffer=nullptr;
    Buffer *m_vertexBuffer=nullptr;
    std::vector<Pipeline*> m_pipelines;

    friend class Attachment;
    friend class Buffer;
    friend class Descriptor;
    friend class DynamicBuffer;
    friend class Pipeline;
    friend class Renderpass;
    friend class Shader;
    friend class StaticBuffer;
    friend class Texture;

    // Tests.
    friend class CommandTest_ctor_Test;
    friend class CommandTest_move_Test;
    friend class DeviceTest_ctor_Test;
    friend class FramebufferTest_ctor_Test;
    friend class PassTest_ctor_Test;
    friend class SwapchainTest_ctor_Test;
    friend class SwapchainTest_move_Test;
    friend class SyncTest_ctor_Test;
    friend class SyncTest_move_Test;
    friend class UtilTest_createImage_Test;
    friend class UtilTest_createImageView_Test;
    friend class UtilTest_createBuffer_Test;
    friend class UtilTest_findMemoryType_Test;
    friend class UtilTest_cmds_Test;
    friend class UtilTest_findQueueFamilies_Test;
    friend class UtilTest_querySwapChainSupport_Test;
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
);

} // namespace evk

#endif