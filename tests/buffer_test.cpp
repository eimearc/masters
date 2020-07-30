#include "evulkan.h"

#include <gtest/gtest.h>

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
            numThreads, window, deviceExtensions,
            swapchainSize, validationLayers
        };
    }

    virtual void TearDown() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    std::vector<uint32_t> indices;
    Device device;
    std::vector<Vertex> vertices;
    GLFWwindow *window;
};

TEST_F(BufferTest, update)
{
    struct Data{int a;};
    Data data{0};
    DynamicBuffer dynamic(device, &data, sizeof(data), 1, Buffer::UBO);
    ASSERT_EQ(static_cast<Data*>(dynamic.m_bufferData)->a, data.a);
    data.a=1;
    ASSERT_EQ(static_cast<Data*>(dynamic.m_bufferData)->a, 0);
    dynamic.update(&data);
    ASSERT_EQ(static_cast<Data*>(dynamic.m_bufferData)->a, 1);

    data.a=0;
    StaticBuffer staticBuffer(device, &data, sizeof(data), 1, Buffer::UBO);
    ASSERT_EQ(static_cast<Data*>(staticBuffer.m_bufferData)->a, data.a);
    data.a=1;
    ASSERT_EQ(static_cast<Data*>(staticBuffer.m_bufferData)->a, 0);
}