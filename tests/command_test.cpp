#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

class CommandTest : public  ::testing::Test
{
    protected:
    virtual void SetUp() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    }

    virtual void TearDown() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    Device device;
    GLFWwindow *window;
    std::vector<const char*> validationLayers =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };
    std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

TEST_F(CommandTest, ctor)
{
    const uint32_t numThreads = 1;
    const uint32_t swapchainSize = 2;
    device = {
        numThreads, deviceExtensions,
        swapchainSize, validationLayers
    };
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> surfaceExtensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount
    );
    auto surfaceFunc = [&](){
        glfwCreateWindowSurface(
            device.instance(), window, nullptr, &device.surface()
        );
    };
    device.createSurface(surfaceFunc,800,600,surfaceExtensions);

    const auto &commands = device.m_commands;

    EXPECT_EQ(commands->m_device, device.m_device->m_device);
    EXPECT_EQ(commands->m_commandPools.size(), numThreads);
    EXPECT_EQ(
        commands->m_primaryCommandBuffers.size(), swapchainSize
    );
    EXPECT_EQ(
        commands->m_secondaryCommandBuffers.size(), numThreads
    );

    EXPECT_TRUE(commands==commands);
    EXPECT_FALSE(commands!=commands);
}

TEST_F(CommandTest, move)
{
    const uint32_t numThreads = 2;
    const uint32_t swapchainSize = 2;
    Device device1 = {
        numThreads, deviceExtensions,
        swapchainSize, validationLayers
    };
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> surfaceExtensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount
    );
    auto surfaceFunc1 = [&](){
        glfwCreateWindowSurface(
            device1.instance(), window, nullptr, &device1.surface()
        );
    };
    device1.createSurface(surfaceFunc1,800,600,surfaceExtensions);

    device = std::move(device1);

    const auto &commands = device.m_commands;
    EXPECT_EQ(commands->m_device, device.m_device->m_device);
    EXPECT_EQ(commands->m_commandPools.size(), numThreads);
    EXPECT_EQ(
        commands->m_primaryCommandBuffers.size(), swapchainSize
    );
    EXPECT_EQ(
        commands->m_secondaryCommandBuffers.size(), numThreads
    );
}

} // namespace evk