#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

class BufferTest : public  ::testing::Test
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
    }

    virtual void TearDown() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    Device device;
    GLFWwindow *window;
};

TEST_F(BufferTest, ctor)
{
    struct Data{int a;};
    Data data{0};
    DynamicBuffer dynamicBuffer(device, &data, sizeof(data), 1, Buffer::UBO);
    EXPECT_TRUE(dynamicBuffer.buffer());
    EXPECT_TRUE(dynamicBuffer.m_bufferMemory);
    EXPECT_EQ(dynamicBuffer.m_bufferSize, sizeof(data));
    EXPECT_TRUE(dynamicBuffer.m_device);
    EXPECT_EQ(dynamicBuffer.m_numElements, 1);
    EXPECT_EQ(dynamicBuffer.m_numThreads, 1);
    EXPECT_TRUE(dynamicBuffer.m_physicalDevice);
    EXPECT_TRUE(dynamicBuffer.m_queue);
    EXPECT_TRUE(dynamicBuffer==dynamicBuffer);
    EXPECT_FALSE(dynamicBuffer!=dynamicBuffer);

    StaticBuffer staticBuffer(device, &data, sizeof(data), 1, Buffer::VERTEX);
    EXPECT_TRUE(staticBuffer.buffer());
    EXPECT_TRUE(staticBuffer.m_bufferMemory);
    EXPECT_EQ(staticBuffer.m_bufferSize, sizeof(data));
    EXPECT_TRUE(staticBuffer.m_device);
    EXPECT_EQ(staticBuffer.m_numElements, 1);
    EXPECT_EQ(staticBuffer.m_numThreads, 1);
    EXPECT_TRUE(staticBuffer.m_physicalDevice);
    EXPECT_TRUE(staticBuffer.m_queue);
    EXPECT_TRUE(staticBuffer==staticBuffer);
    EXPECT_FALSE(staticBuffer!=staticBuffer);
}

TEST_F(BufferTest, update)
{
    struct Data{int a;};
    Data data{0};
    DynamicBuffer dynamic(device, &data, sizeof(data), 1, Buffer::UBO);
    ASSERT_EQ(static_cast<Data*>(dynamic.m_bufferData)->a, data.a);
    data.a=1;
    EXPECT_EQ(static_cast<Data*>(dynamic.m_bufferData)->a, 0);
    dynamic.update(&data);
    EXPECT_EQ(static_cast<Data*>(dynamic.m_bufferData)->a, 1);

    data.a=0;
    StaticBuffer staticBuffer(device, &data, sizeof(data), 1, Buffer::UBO);
    ASSERT_EQ(static_cast<Data*>(staticBuffer.m_bufferData)->a, data.a);
    data.a=1;
    EXPECT_EQ(static_cast<Data*>(staticBuffer.m_bufferData)->a, 0);
}

} // namespace evk