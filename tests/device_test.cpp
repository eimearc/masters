#include "device.h"

#include <gtest/gtest.h>

std::vector<const char*> validationLayers =
{
    "VK_LAYER_LUNARG_standard_validation"
};
std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
GLFWwindow *window=nullptr;

TEST(Device, firstTest)
{
    std::cout << "Hello from first test.\n";
}

TEST(Device, move)
{
    const uint32_t numThreads = 1;
    const uint32_t swapchainSize = 2;

    Device device1(numThreads, validationLayers, window, deviceExtensions, swapchainSize, true);
    auto device = std::move(device1);
    device1 = std::move(device);
    device = std::move(device1);
    device = std::move(device);
}

int main()
{
    testing::InitGoogleTest();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    auto result = RUN_ALL_TESTS();

    glfwDestroyWindow(window);
    glfwTerminate();

    return result;
}