#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

class SyncTest : public  ::testing::Test
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

TEST_F(SyncTest,ctor)
{
    device = {1, deviceExtensions, 2, validationLayers};
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

    auto &sync = device.m_sync;
    EXPECT_TRUE(sync->m_device);
    for (const auto &f : sync->m_fencesInFlight) EXPECT_TRUE(f);
    for (const auto &s : sync->m_imageAvailableSemaphores) EXPECT_TRUE(s);
    for (const auto &f : sync->m_imagesInFlight) EXPECT_TRUE(f==VK_NULL_HANDLE);
    for (const auto &s : sync->m_renderFinishedSemaphores) EXPECT_TRUE(s);

    EXPECT_TRUE(sync==sync);
    EXPECT_FALSE(sync!=sync);
}

TEST_F(SyncTest,move)
{
    device = {1, deviceExtensions, 2, validationLayers};
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

    Device device1 = {1, deviceExtensions, 2, validationLayers};
    auto surfaceFunc1 = [&](){
        glfwCreateWindowSurface(
            device1.instance(), window, nullptr, &device1.surface()
        );
    };
    device1.createSurface(surfaceFunc1,800,600,surfaceExtensions);

    auto &sync = device.m_sync;
    sync = std::move(device1.m_sync);

    EXPECT_TRUE(sync->m_device);
    for (const auto &f : sync->m_fencesInFlight) EXPECT_TRUE(f);
    for (const auto &s : sync->m_imageAvailableSemaphores) EXPECT_TRUE(s);
    for (const auto &f : sync->m_imagesInFlight) EXPECT_TRUE(f==VK_NULL_HANDLE);
    for (const auto &s : sync->m_renderFinishedSemaphores) EXPECT_TRUE(s);

    device1.m_sync = std::move(sync);    
}

} // namespace evk