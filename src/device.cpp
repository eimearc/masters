#include "device.h"

#include <set>

namespace evk {

Device::Device(
    uint32_t num_threads,
    GLFWwindow *window,
    const std::vector<const char *> &device_extensions,
    uint32_t swapchain_size)
{
    m_threadPool.setThreadCount(num_threads);
    m_numThreads=num_threads;

    const std::vector<const char*> validationLayers; // TODO: Remove.
    m_device=std::make_unique<_Device>(
        num_threads, validationLayers, window, device_extensions
    );
    m_swapchain=std::make_unique<Swapchain>(
        m_device->m_device, m_device->m_physicalDevice, m_device->m_surface,
        window, swapchain_size
    );
    m_sync=std::make_unique<Sync>(m_device->m_device, swapchain_size);
    m_commands=std::make_unique<Commands>(m_device->m_device,
        m_device->m_physicalDevice, m_device->m_surface, swapchain_size,
        num_threads
    );
}

Device::Device(
    uint32_t num_threads,
    GLFWwindow *window,
    const std::vector<const char *> &device_extensions,
    uint32_t swapchain_size,
    const std::vector<const char*> &validation_layers)
{
    m_threadPool.setThreadCount(num_threads);
    m_numThreads=num_threads;

    m_device=std::make_unique<_Device>(
        num_threads, validation_layers, window, device_extensions
    );
    m_swapchain=std::make_unique<Swapchain>(
        m_device->m_device, m_device->m_physicalDevice, m_device->m_surface,
        window, swapchain_size
    );
    m_sync=std::make_unique<Sync>(m_device->m_device, swapchain_size);
    m_commands=std::make_unique<Commands>(m_device->m_device,
        m_device->m_physicalDevice, m_device->m_surface, swapchain_size,
        num_threads
    );
}

bool Device::operator==(const Device& other)
{
    bool result = true;
    if ((m_commands!=nullptr) && (other.m_commands!=nullptr))
        result &= (*m_commands.get() == *other.m_commands.get());
    else result &= ((m_commands==nullptr) && (other.m_commands==nullptr));

    if ((m_device!=nullptr) && (other.m_device!=nullptr))
        result &= (*m_device.get() == *other.m_device.get());
    else result &= ((m_device==nullptr) && (other.m_device==nullptr));

    if ((m_framebuffer!=nullptr) && other.m_framebuffer!=nullptr)
        result &= (*m_framebuffer.get() == *other.m_framebuffer.get());
    else result &= ((m_framebuffer==nullptr) && (other.m_framebuffer==nullptr));
    
    result &= (m_numThreads == other.m_numThreads);

    if ((m_swapchain!=nullptr) && (other.m_swapchain!=nullptr))
        result &= (*m_swapchain.get() == *other.m_swapchain.get());
    else result &= ((m_swapchain==nullptr) && (other.m_swapchain==nullptr));

    if ((m_sync!=nullptr) && (other.m_sync!=nullptr))
        result &= (*m_sync.get() == *other.m_sync.get());
    else result &= ((m_sync==nullptr) && (other.m_sync==nullptr));

    return result;
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
    m_sync = std::move(other.m_sync);
    m_threadPool = std::move(other.m_threadPool);
    return *this;
}

Device::_Device::_Device(
    uint32_t num_threads,
    const std::vector<const char*> &validation_layers,
    GLFWwindow *window,
    const std::vector<const char *> &device_extensions
)
{
    m_window=window;
    createInstance(validation_layers);
    createSurface(window);
    pickPhysicalDevice(device_extensions);
    setDepthFormat();

    createDevice(device_extensions, validation_layers);
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
    m_window=other.m_window;
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

bool Device::_Device::operator==(const _Device &other)
{
    bool result = true;
    result &= (m_debugMessenger==other.m_debugMessenger);
    result &= (m_depthFormat==other.m_depthFormat);
    result &= (m_device==other.m_device);
    result &= (m_graphicsQueue==other.m_graphicsQueue);
    result &= (m_instance==other.m_instance);
    result &= (m_physicalDevice==other.m_physicalDevice);
    result &= (m_presentQueue==other.m_presentQueue);
    result &= (m_surface==other.m_surface);
    result &= (m_window==other.m_window);
    return result;
}

void Device::_Device::createInstance(const std::vector<const char*> &validation_layers)
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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(validation_layers.size() > 0)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        createInfo.ppEnabledLayerNames = validation_layers.data();
        debugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance->");
    }

    if (validation_layers.size() > 0)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        debugMessengerCreateInfo(createInfo); //TODO: Why is this duplicate of above?

        if (createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger.");
        }
    }
}

void Device::_Device::createSurface(GLFWwindow *window)
{
    if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window instance->surface.");
    }
}

void Device::_Device::pickPhysicalDevice(const std::vector<const char *> &device_extensions)
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
        if (isDeviceSuitable(device, m_surface, device_extensions))
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

void Device::_Device::createDevice(
    const std::vector<const char*> &deviceExtensions,
    const std::vector<const char*> &validationLayers
)
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

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (validationLayers.size()>0) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
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

    for (const auto& extension : availableExtensions)
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
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); //Crashes here.
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