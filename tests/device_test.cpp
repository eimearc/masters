#include "evulkan.h"

#include <gtest/gtest.h>

class DeviceTest : public  ::testing::Test
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

    std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    std::vector<const char*> validationLayers =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };
    GLFWwindow *window;
};


TEST_F(DeviceTest, ctor)
{
    const uint32_t numThreads = 2;
    const uint32_t swapchainSize = 2;
    Device device(
        numThreads, window, deviceExtensions, swapchainSize, validationLayers
    );

    ASSERT_NE(device.m_device.get(), nullptr);
    ASSERT_NE(device.m_commands.get(), nullptr);
    ASSERT_NE(device.m_swapchain.get(), nullptr);
    ASSERT_NE(device.m_sync.get(), nullptr);

    ASSERT_EQ(device.m_framebuffer.get(), nullptr);
    ASSERT_EQ(device.m_numThreads, numThreads);
}

TEST_F(DeviceTest, move)
{
    const uint32_t numThreads = 1;
    const uint32_t swapchainSize = 2;

    Device device1(
        numThreads, window, deviceExtensions, swapchainSize, validationLayers
    );

    auto device = std::move(device1);
    device1 = std::move(device);
    device = std::move(device1);
    device = std::move(device);

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment colorAttachment(device, 1, Attachment::Type::COLOR);
    Attachment depthAttachment(device, 2, Attachment::Type::DEPTH);
}