#include "device.h"

#include <gtest/gtest.h>

class DescriptorTest : public  ::testing::Test
{
    protected:
    virtual void SetUp() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

        std::vector<const char*> validationLayers =
        {
            "VK_LAYER_LUNARG_standard_validation"
        };
        std::vector<const char*> deviceExtensions = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        const uint32_t numThreads = 1;
        const uint32_t swapchainSize = 2;
        device = {
            numThreads, validationLayers, window, deviceExtensions,
            swapchainSize, true
        };
    }

    virtual void TearDown() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow *window;
    Device device;
};

TEST_F(DescriptorTest, firstTest)
{
    std::cout << "Hello from first test.\n";
}