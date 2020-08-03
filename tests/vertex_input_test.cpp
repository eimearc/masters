#include "evulkan.h"

#include <gtest/gtest.h>

class VertexInputTest : public  ::testing::Test
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
};

TEST_F(VertexInputTest, ctor)
{
    VertexInput v(100);
    EXPECT_EQ(v.m_bindingDescription.stride, 100);

    auto &attributeDescriptions = v.m_attributeDescriptions;
    auto &bindingDescription = v.m_bindingDescription;

    v.addVertexAttributeVec2(0,0);
    EXPECT_EQ(attributeDescriptions.size(),1);
    EXPECT_EQ(attributeDescriptions[0].location,0);
    EXPECT_EQ(attributeDescriptions[0].format,VK_FORMAT_R32G32_SFLOAT);
    EXPECT_EQ(attributeDescriptions[0].offset,0);

    v.addVertexAttributeVec3(1,5);
    EXPECT_EQ(attributeDescriptions.size(),2);
    EXPECT_EQ(attributeDescriptions[1].location,1);
    EXPECT_EQ(attributeDescriptions[1].format,VK_FORMAT_R32G32B32_SFLOAT);
    EXPECT_EQ(attributeDescriptions[1].offset,5);

    v.addVertexAttributeVec2(0,2); // Check that location 0 overwrites old one.
    EXPECT_EQ(attributeDescriptions.size(),2);
    EXPECT_EQ(attributeDescriptions[0].location,0);
    EXPECT_EQ(attributeDescriptions[0].format,VK_FORMAT_R32G32_SFLOAT);
    EXPECT_EQ(attributeDescriptions[0].offset,2);
}