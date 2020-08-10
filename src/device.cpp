#include "device.h"

#include <set>

namespace evk {

Device::Device(
    uint32_t num_threads,
    const std::vector<const char *> &deviceExtensions,
    uint32_t swapchainSize)
{
    m_threadPool.setThreadCount(num_threads);
    m_numThreads=num_threads;
    m_swapchainSize=swapchainSize;

    const std::vector<const char*> validationLayers; // TODO: Remove.
    m_device=std::make_unique<_Device>(
        validationLayers, deviceExtensions
    );
}

Device::Device(
    uint32_t num_threads,
    const std::vector<const char *> &deviceExtensions,
    uint32_t swapchainSize,
    const std::vector<const char*> &validationLayers)
{
    m_threadPool.setThreadCount(num_threads);
    m_numThreads = num_threads;
    m_swapchainSize = swapchainSize;

    m_device=std::make_unique<_Device>(
        validationLayers, deviceExtensions
    );
}

void Device::finishSetup(
    std::function<void()> windowFunc,
    const std::vector<const char*> &windowExtensions
)
{
    m_device->finishSetup(windowFunc, windowExtensions);
    m_swapchain=std::make_unique<Swapchain>(
        m_device->m_device, m_device->m_physicalDevice, m_device->m_surface,
        m_windowExtent, m_swapchainSize
    );
    m_sync=std::make_unique<Sync>(m_device->m_device, m_swapchainSize);
    m_commands=std::make_unique<Commands>(m_device->m_device,
        m_device->m_physicalDevice, m_device->m_surface, m_swapchainSize,
        m_numThreads
    );
}

void Device::createSurface(
    std::function<void()> surfaceFunc,
    uint32_t width,
    uint32_t height,
    const std::vector<const char*> &windowExtensions
)
{
    m_windowExtent = {width,height};
    finishSetup(surfaceFunc, windowExtensions);
}

bool Device::operator==(const Device& other) const noexcept
{
    if ((m_commands!=nullptr) && (other.m_commands!=nullptr))
        if (*m_commands.get() != *other.m_commands.get()) return false;

    if ((m_commands==nullptr) != (other.m_commands==nullptr))
        return false;

    if ((m_device!=nullptr) && (other.m_device!=nullptr))
        if (*m_device.get() != *other.m_device.get()) return false;

    if ((m_device==nullptr) != (other.m_device==nullptr)) return false;

    if ((m_framebuffer!=nullptr) && other.m_framebuffer!=nullptr)
        if (*m_framebuffer.get() != *other.m_framebuffer.get()) return false;

    if ((m_framebuffer==nullptr) != (other.m_framebuffer==nullptr))
        return false;
    
    if (m_numThreads != other.m_numThreads) return false;

    if ((m_swapchain!=nullptr) && (other.m_swapchain!=nullptr))
        if (*m_swapchain.get() != *other.m_swapchain.get()) return false;

    if ((m_swapchain==nullptr) != (other.m_swapchain==nullptr)) return false;

    if (m_swapchainSize != other.m_swapchainSize) return false;

    if ((m_sync!=nullptr) && (other.m_sync!=nullptr))
        if (*m_sync.get() != *other.m_sync.get()) return false;

    if ((m_sync==nullptr) != (other.m_sync==nullptr)) return false;

    return true;
}

bool Device::operator!=(const Device& other) const noexcept
{
    return !(*this==other);
}

Device::Device(Device&& other) noexcept
{
    *this=std::move(other);
}

Device& Device::operator=(Device&& other) noexcept
{
    if (*this == other) return *this;
    m_device = std::move(other.m_device);
    m_commands = std::move(other.m_commands);
    m_framebuffer = std::move(other.m_framebuffer);
    m_numThreads = other.m_numThreads;
    other.m_numThreads=1;
    m_swapchain = std::move(other.m_swapchain);
    m_swapchainSize=other.m_swapchainSize;
    m_sync = std::move(other.m_sync);
    m_threadPool = std::move(other.m_threadPool);
    return *this;
}

Device::_Device::_Device(
    const std::vector<const char*> &validationLayers,
    const std::vector<const char *> &deviceExtensions
)
{
    m_deviceExtensions=deviceExtensions;
    m_validationLayers=validationLayers;
}

void Device::_Device::finishSetup(
    std::function<void()> windowFunc,
    const std::vector<const char *> &windowExtensions
)
{
    m_windowExtensions=windowExtensions;
    createInstance();
    windowFunc();
    pickPhysicalDevice();
    setDepthFormat();
    createDevice();
}

// TODO: Check is this needed.
Device::_Device::_Device(_Device&& other) noexcept
{
    *this=std::move(other);
}

Device::_Device& Device::_Device::operator=(_Device&& other) noexcept
{
    if (*this == other) return *this;
    m_debugMessenger=other.m_debugMessenger;
    other.m_debugMessenger=VK_NULL_HANDLE;
    m_depthFormat=other.m_depthFormat;
    m_device=other.m_device;
    other.m_device=VK_NULL_HANDLE;
    m_graphicsQueue=other.m_graphicsQueue;
    m_instance=other.m_instance;
    other.m_instance=VK_NULL_HANDLE;
    m_physicalDevice=other.m_physicalDevice;
    m_presentQueue=other.m_presentQueue;
    m_surface=other.m_surface;
    other.m_surface=VK_NULL_HANDLE;
    return *this;
}

Device::_Device::~_Device() noexcept
{
    // TODO: Should wait for idle?
    if (m_device!=VK_NULL_HANDLE) vkDestroyDevice(m_device, nullptr);
    if (m_debugMessenger!=VK_NULL_HANDLE)
        destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    if (m_surface!=VK_NULL_HANDLE)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    if (m_instance!=VK_NULL_HANDLE) vkDestroyInstance(m_instance, nullptr);
}

bool Device::_Device::operator==(const _Device &other) const noexcept
{
    if (m_debugMessenger!=other.m_debugMessenger)return false;
    if (m_depthFormat!=other.m_depthFormat)return false;
    if (m_device!=other.m_device)return false;
    if (m_graphicsQueue!=other.m_graphicsQueue)return false;
    if (m_instance!=other.m_instance)return false;
    if (m_physicalDevice!=other.m_physicalDevice)return false;
    if (m_presentQueue!=other.m_presentQueue)return false;
    if (m_surface!=other.m_surface)return false;
    return true;
}

bool Device::_Device::operator!=(const _Device &other) const noexcept
{
    return !(*this==other);
}

void Device::_Device::createInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;

    auto extensions=m_windowExtensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(m_validationLayers.size() > 0)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
        debugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // TODO: Is this needed?
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance->");
    }

    if (m_validationLayers.size() > 0)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        debugMessengerCreateInfo(createInfo); //TODO: Why is this duplicate of above?

        if (createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger.");
        }
    }
}

void Device::_Device::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, m_surface, m_deviceExtensions))
        {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find suitable GPU.");
    }
}

void Device::_Device::createDevice()
{
    internal::QueueFamilyIndices indices = getQueueFamilies(m_physicalDevice, m_surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        static_cast<uint32_t>(indices.graphicsFamily),
        static_cast<uint32_t>(indices.presentFamily)
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy=VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if (m_validationLayers.size()>0) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device.");
    }

    vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
}

VkResult Device::_Device::createDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Device::_Device::setDepthFormat()
{
    std::vector<VkFormat> candidates = {
        VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormatProperties props;
    for (VkFormat format : candidates)
    {
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);
        if ((props.optimalTilingFeatures & features) == features)
        {
            m_depthFormat = format;
            break;
        }
    }
}

bool Device::_Device::isDeviceSuitable(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    std::vector<const char *> deviceExtensions
)
{
    internal::QueueFamilyIndices indices = internal::findQueueFamilies(
        device, surface
    );

    bool extensionsSupported = checkDeviceExtensionSupport(
        device, deviceExtensions
    );

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        internal::SwapChainSupportDetails swapChainSupport =
            internal::querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty()
            && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported
        && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Device::_Device::checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> deviceExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Device::_Device::destroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void Device::_Device::debugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

internal::QueueFamilyIndices Device::_Device::getQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
)
{
    internal::QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
        }
        if (indices.isComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

} // namespace evk