#include "evulkan.h"

#include <gtest/gtest.h>

class TextureTest : public  ::testing::Test
{
    protected:
    virtual void SetUp() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
        device = {1, window, deviceExtensions, 2, validationLayers};
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
    Texture texture;
};

TEST_F(TextureTest,ctor)
{
    texture=Texture(device, "viking_room.png");

    EXPECT_TRUE(texture.m_device);
    EXPECT_TRUE(texture.m_image);
    EXPECT_TRUE(texture.m_imageSampler);
    EXPECT_TRUE(texture.m_imageView);
    EXPECT_TRUE(texture.m_memory);

    EXPECT_TRUE(texture==texture);
    EXPECT_FALSE(texture!=texture);
}

TEST_F(TextureTest, get)
{
    texture=Texture(device, "viking_room.png");
    EXPECT_EQ(texture.sampler(), texture.m_imageSampler);
    EXPECT_EQ(texture.view(), texture.m_imageView);
}

TEST_F(TextureTest,move)
{
    texture=Texture(device, "viking_room.png");
    Texture texture1=std::move(texture);
    EXPECT_TRUE(texture1.m_device);
    EXPECT_TRUE(texture1.m_image);
    EXPECT_TRUE(texture1.m_imageSampler);
    EXPECT_TRUE(texture1.m_imageView);
    EXPECT_TRUE(texture1.m_memory);

    texture=std::move(texture1);
    EXPECT_TRUE(texture.m_device);
    EXPECT_TRUE(texture.m_image);
    EXPECT_TRUE(texture.m_imageSampler);
    EXPECT_TRUE(texture.m_imageView);
    EXPECT_TRUE(texture.m_memory);
}