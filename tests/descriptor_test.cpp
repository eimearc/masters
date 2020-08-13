#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

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

        descriptor = {device, swapchainSize};
    }

    virtual void TearDown() override
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    Descriptor descriptor;
    Device device;
    GLFWwindow *window;
};

TEST_F(DescriptorTest, ctor)
{
    if (descriptor.m_device==VK_NULL_HANDLE) FAIL();
    EXPECT_TRUE(descriptor.m_device);
    EXPECT_EQ(descriptor.m_swapchainSize,2);
    EXPECT_EQ(descriptor.m_writeSetVertex.size(),0);
    EXPECT_EQ(descriptor.m_writeSetFragment.size(),0);
    EXPECT_EQ(descriptor.m_poolSizes.size(),3);
    auto types = {
        Descriptor::Type::INPUT_ATTACHMENT,
        Descriptor::Type::TEXTURE_SAMPLER,
        Descriptor::Type::UNIFORM_BUFFER
    };
    for (const auto &t : types)
    {
        auto index = static_cast<uint32_t>(t);
        EXPECT_EQ(descriptor.m_poolSizes[index].descriptorCount, 0);
    }
    EXPECT_TRUE(descriptor==descriptor);
    EXPECT_FALSE(descriptor!=descriptor);
}

TEST_F(DescriptorTest, multipleUniformBuffers)
{
    struct UniformBufferA {char a;};
    struct UniformBufferB {double b;};
    UniformBufferA a{'a'};
    UniformBufferB b{3.94};

    DynamicBuffer uboA(device, &a, sizeof(a), 1, Buffer::Type::UBO);
    DynamicBuffer uboB(device, &b, sizeof(b), 1, Buffer::Type::UBO);

    descriptor.addUniformBuffer(0, uboA, Shader::Stage::VERTEX);
    descriptor.addUniformBuffer(1, uboB, Shader::Stage::VERTEX);

    EXPECT_EQ(descriptor.m_bufferInfo.size(), 2);
}

} // namespace evk