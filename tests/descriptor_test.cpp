#include "evulkan.h"

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
            numThreads, window, deviceExtensions,
            swapchainSize, validationLayers
        };

        descriptor = {device, swapchainSize};
    }

    virtual void TearDown() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    std::vector<uint32_t> indices;
    Descriptor descriptor;
    Device device;
    std::vector<Vertex> vertices;
    GLFWwindow *window;
};

TEST_F(DescriptorTest, firstTest)
{
    std::cout << "Hello from first test.\n";
}

TEST_F(DescriptorTest, multipleUniformBuffers)
{
    struct UniformBufferA {char a;};
    struct UniformBufferB {double b;};
    UniformBufferA a{'a'};
    UniformBufferB b{3.94};

    DynamicBuffer uboA(device, &a, sizeof(a), 1, Buffer::UBO);
    DynamicBuffer uboB(device, &b, sizeof(b), 1, Buffer::UBO);

    descriptor.addUniformBuffer(0, uboA, Shader::Stage::VERTEX);
    descriptor.addUniformBuffer(1, uboB, Shader::Stage::VERTEX);

    // ASSERT_EQ(descriptor.m_device, VK_NULL_HANDLE);
}