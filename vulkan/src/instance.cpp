#include "evulkan_core.h"

#include <set>
#include <vector>
#include <string>
#include <iostream>

std::vector<const char*> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
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

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
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

void evkCreateWindow(const EVkCreateWindow params, GLFWwindow *&window)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

void evkCreateInstance(const EVkCreateInstance *pCreateInfo, VkInstance *instance)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = pCreateInfo->appTitle;
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    auto extensions = getRequiredExtensions();
    for (const auto &e : pCreateInfo->extensions)
    {
        extensions.push_back(e);
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(FLAGS_enable_validation)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(pCreateInfo->validationLayers.size());
        createInfo.ppEnabledLayerNames = pCreateInfo->validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance->");
    }
}

void evkSetupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    if (!FLAGS_enable_validation) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, pDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger.");
    }
}

void evkCreateSurface(VkInstance instance, const EVkSurfaceCreate *pCreateInfo, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, pCreateInfo->window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window instance->surface.");
    }
}

void evkPickPhysicalDevice(VkInstance instance, const EVkPickPhysicalDevice *pPickInfo, VkPhysicalDevice *physicalDevice)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, pPickInfo->surface, pPickInfo->deviceExtensions))
        {
            *physicalDevice = device;
            break;
        }
    }

    if (*physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find suitable GPU.");
    }
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, const EVkPickPhysicalDevice *pPickInfo)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, pPickInfo->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, pPickInfo->surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, pPickInfo->surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, pPickInfo->surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, pPickInfo->surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const EVkPickPhysicalDevice *pPickInfo)
{
    QueueFamilyIndices indices;

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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, pPickInfo->surface, &presentSupport);
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