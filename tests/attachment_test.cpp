#include "evulkan.h"

#include <gtest/gtest.h>

class AttachmentTest : public  ::testing::Test
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

    Attachment attachment;
    Device device;
    GLFWwindow *window;
};

TEST_F(AttachmentTest, move)
{
    Attachment a(device, 1, Attachment::Type::DEPTH);
    a=std::move(a);
    ASSERT_EQ(a.m_colorReference.attachment,1);

    Attachment b(device, 2, Attachment::Type::COLOR);
    a=std::move(b);
    ASSERT_EQ(a.index(),2);
    ASSERT_EQ(a.description().format, VK_FORMAT_R8G8B8A8_UNORM);
    VkClearColorValue got = a.clearValue().color;
    VkClearColorValue want = {0.0f,0.0f,0.0f,1.0f};
    ASSERT_EQ(*got.float32, *want.float32);
    ASSERT_EQ(a.m_colorReference.attachment,2);
    ASSERT_EQ(a.m_depthReference.attachment,2);
    ASSERT_EQ(b.index(),0);
    if (b.m_image!=VK_NULL_HANDLE) FAIL();
    if (b.m_imageMemory!=VK_NULL_HANDLE) FAIL();
    if (b.m_imageView!=VK_NULL_HANDLE) FAIL();
}