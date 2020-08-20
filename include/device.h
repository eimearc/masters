#ifndef EVK_DEVICE_H_
#define EVK_DEVICE_H_

#include <functional>
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
 *  1, extensions, 2, layers
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
     * @param[in] deviceExtensions the extensions required to create the device.
     * @param[in] swapchainSize the number of images used by the swapchain. A
     *  graphics card has a minimum number of images it needs to function, in
     *  addition to a maximum. The capabilities of a graphics card are
     *  available to view by running the `vulkaninfo` command.
     */
    Device(
        uint32_t numThreads,
        const std::vector<const char *> &deviceExtensions,
        const uint32_t swapchainSize
    ) noexcept;

    /**
     * Constructs a Device with validation layers turned on. This is useful for
     *  developing programs.
     * @param[in] numThreads the number of threads the Device should use.
     * @param[in] deviceExtensions the extensions required to create the device.
     * @param[in] swapchainSize the number of images used by the swapchain. A
     *  graphics card has a minimum number of images it needs to function, in
     *  addition to a maximum. The capabilities of a graphics card are
     *  available to view by running the `vulkaninfo` command.
     * @param[in] validationLayers the layers to use for validation.
     */
    Device(
        uint32_t numThreads,
        const std::vector<const char *> &deviceExtensions,
        uint32_t swapchainSize,
        const std::vector<const char*> &validationLayers
    ) noexcept;

    bool operator==(const Device&) const noexcept;
    bool operator!=(const Device&) const noexcept;

    /**
     * Creates a surface object and finishes Device setup.
     * @param[in] surfaceFunc the function which will be called to generate
     *  a surface object.
     * @param[in] width the window width in pixels.
     * @param[in] height the window height in pixels.
     * @param[in] windowExtensions the set of extentsions required for Vulkan
     *  to interface with the windowing system.
     **/
    void createSurface(
        std::function<void()> surfaceFunc,
        uint32_t width,
        uint32_t height,
        const std::vector<const char *> &windowExtensions
    ) noexcept;

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
    ) noexcept;

    /**
     * Draw.
     **/
    void draw() noexcept;

    /**
     * Resize the surface and associated resources during the next draw command.
     **/
    void resizeRequired() noexcept;

    VkInstance instance() const noexcept { return m_device->m_instance; }
    VkSurfaceKHR& surface() const noexcept { return m_device->m_surface; }

    private:
    // Device.
    VkPhysicalDevice physicalDevice() const noexcept
    {
        return m_device->m_physicalDevice;
    }
    VkDevice device() const noexcept { return m_device->m_device; }
    VkFormat depthFormat() const noexcept { return m_device->m_depthFormat; };
    VkQueue graphicsQueue() const noexcept
    {
        return m_device->m_graphicsQueue;
    };
    VkQueue presentQueue() const noexcept { return m_device->m_presentQueue; };
    uint32_t numThreads() const noexcept { return m_numThreads; };
    std::vector<std::unique_ptr<Thread>>& threads() noexcept
    {
        return m_threadPool.threads;
    };

    void finishSetup(
        std::function<void()> windowFunc,
        const std::vector<const char*> &windowExtensions
    ) noexcept;
    void record() noexcept;
    void reset() noexcept;
    void resizeWindow() noexcept;
    void wait() noexcept { m_threadPool.wait(); };

    // Swapchain.
    VkExtent2D extent() const noexcept { return m_swapchain->m_extent; };
    VkSwapchainKHR swapchain() const noexcept
    {
        return m_swapchain->m_swapchain;
    };
    uint32_t swapchainSize() const noexcept
    {
        return m_swapchain->m_images.size();
    };
    std::vector<VkImageView> swapchainImageViews() const noexcept
    {
        return m_swapchain->m_imageViews;
    };

    // Commands.
    std::vector<VkCommandPool> commandPools() const noexcept
    {
        return m_commands->m_commandPools;
    };
    std::vector<VkCommandBuffer> primaryCommandBuffers() const noexcept
    {
        return m_commands->m_primaryCommandBuffers;
    };
    std::vector<VkCommandBuffer> secondaryCommandBuffers() const noexcept
    {
        return m_commands->m_secondaryCommandBuffers;
    };

    // Sync.
    std::vector<VkFence>& frameFences() const noexcept
    {
        return m_sync->m_fencesInFlight;
    };
    std::vector<VkFence>& imageFences() const noexcept
    {
        return m_sync->m_imagesInFlight;
    };
    std::vector<VkSemaphore>& imageSempahores() const noexcept
    {
        return m_sync->m_imageAvailableSemaphores;
    };
    std::vector<VkSemaphore>& renderSempahores() const noexcept
    {
        return m_sync->m_renderFinishedSemaphores;
    };

    // Framebuffers.
    std::vector<VkFramebuffer> framebuffers() const noexcept
    {
        return m_framebuffer->m_framebuffers;
    };

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
            const std::vector<const char*> &validationLayers,
            const std::vector<const char *> &deviceExtensions
        );

        bool operator==(const _Device &other) const noexcept;
        bool operator!=(const _Device &other) const noexcept;

        void finishSetup(
            std::function<void()> windowFunc,
            const std::vector<const char *> &windowExtensions
        ) noexcept;
        void reset() noexcept;

        VkDebugUtilsMessengerEXT m_debugMessenger=VK_NULL_HANDLE;
        VkFormat m_depthFormat;
        VkDevice m_device=VK_NULL_HANDLE;
        std::vector<const char *> m_deviceExtensions;
        VkQueue m_graphicsQueue=VK_NULL_HANDLE;
        VkInstance m_instance=VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice=VK_NULL_HANDLE;
        VkQueue m_presentQueue=VK_NULL_HANDLE;
        VkSurfaceKHR m_surface=VK_NULL_HANDLE;
        std::vector<const char*> m_validationLayers;
        std::vector<const char *> m_windowExtensions;

        private:
        void createInstance() noexcept;
        void pickPhysicalDevice() noexcept;
        void createDevice() noexcept;
        VkResult createDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger
        ) noexcept;
        void setDepthFormat() noexcept;
        bool isDeviceSuitable(
            VkPhysicalDevice device,
            VkSurfaceKHR surface,
            std::vector<const char *> deviceExtensions
        ) noexcept;
        bool checkDeviceExtensionSupport(
            VkPhysicalDevice device,
            std::vector<const char *> deviceExtensions
        ) noexcept;
        void destroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        ) noexcept;
        void debugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT& createInfo
        ) noexcept;
        internal::QueueFamilyIndices getQueueFamilies(
            VkPhysicalDevice device,
            VkSurfaceKHR surface
        ) noexcept;
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
            VkExtent2D windowExtent,
            const uint32_t swapchainSize
        );

        bool operator==(const Swapchain &other) const noexcept;
        bool operator!=(const Swapchain &other) const noexcept;

        void reset() noexcept;
        void recreate() noexcept;
        void setup() noexcept;

        VkExtent2D chooseSwapExtent(
            const VkSurfaceCapabilitiesKHR& capabilities
        ) const noexcept;
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes
        ) const noexcept;
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats
        ) const noexcept;
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
        VkExtent2D m_windowExtent;
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

        bool operator==(const Commands&) const noexcept;
        bool operator!=(const Commands&) const noexcept;

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

        bool operator==(const Sync &other) const noexcept;
        bool operator!=(const Sync &other) const noexcept;

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
            Renderpass &renderpass
        );

        bool operator==(const Framebuffer &other) const noexcept;
        bool operator!=(const Framebuffer &other) const noexcept;

        void recreate() noexcept;
        void reset() noexcept;
        void setup() noexcept;

        Device *m_device=nullptr;
        std::vector<VkFramebuffer> m_framebuffers;
        Renderpass *m_renderpass=nullptr;
        size_t m_swapchainSize;
    };
    
    std::unique_ptr<_Device> m_device=nullptr;
    std::unique_ptr<Commands> m_commands=nullptr;
    std::unique_ptr<Framebuffer> m_framebuffer=nullptr;
    Buffer *m_indexBuffer=nullptr;
    size_t m_numThreads=1;
    std::vector<Pipeline*> m_pipelines;
    bool m_resizeRequired=false;
    std::unique_ptr<Swapchain> m_swapchain=nullptr;
    uint32_t m_swapchainSize=1;
    std::unique_ptr<Sync> m_sync=nullptr;
    ThreadPool m_threadPool;
    VkExtent2D m_windowExtent;
    Buffer *m_vertexBuffer=nullptr;

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
    FRIEND_TEST(CommandTest,ctor);
    FRIEND_TEST(CommandTest,move);
    FRIEND_TEST(DeviceTest,ctor);
    FRIEND_TEST(FramebufferTest,ctor);
    FRIEND_TEST(PassTest,ctor);
    FRIEND_TEST(SwapchainTest,ctor);
    FRIEND_TEST(SwapchainTest,move);
    FRIEND_TEST(SyncTest,ctor);
    FRIEND_TEST(SyncTest,move);
    FRIEND_TEST(UtilTest,createImage);
    FRIEND_TEST(UtilTest,createImageView);
    FRIEND_TEST(UtilTest,createBuffer);
    FRIEND_TEST(UtilTest,findMemoryType);
    FRIEND_TEST(UtilTest,cmds);
    FRIEND_TEST(UtilTest,findQueueFamilies);
    FRIEND_TEST(UtilTest,querySwapChainSupport);
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) noexcept;

} // namespace evk

#endif