#include "evulkan.h"

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
        numThreads, window, deviceExtensions,
        swapchainSize, validationLayers
    };

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

TEST_F(CommandTest, move)
{
    const uint32_t numThreads = 2;
    const uint32_t swapchainSize = 2;
    Device device1 = {
        numThreads, window, deviceExtensions,
        swapchainSize, validationLayers
    };

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