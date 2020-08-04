#include "evulkan.h"

#include <gtest/gtest.h>

namespace evk {

class SwapchainTest : public  ::testing::Test
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
    Device device;
};

TEST_F(SwapchainTest,ctor)
{
    device = {1, window, deviceExtensions, 2, validationLayers};

    auto &swapchain = device.m_swapchain;
    if (swapchain->m_device==VK_NULL_HANDLE) FAIL();
    if (swapchain->m_swapchain==VK_NULL_HANDLE) FAIL();
    EXPECT_EQ(swapchain->m_imageViews.size(),2);
    EXPECT_TRUE(swapchain->m_device);
    EXPECT_TRUE(swapchain->m_swapchain);
    for (const auto &i : swapchain->m_images)
        EXPECT_TRUE(i);
    for (const auto &iv : swapchain->m_imageViews)
        EXPECT_TRUE(iv);
    EXPECT_TRUE(swapchain->m_format);

    if (swapchain!=swapchain) FAIL();
}

TEST_F(SwapchainTest,move)
{
    device = {1, window, deviceExtensions, 2, validationLayers};
    Device device1 = {1, window, deviceExtensions, 2, validationLayers};

    auto &swapchain = device.m_swapchain;
    swapchain = std::move(device1.m_swapchain);

    device1.m_swapchain = std::move(swapchain);    
}

} // namespace evk