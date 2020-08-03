#include "evulkan.h"

#include <gtest/gtest.h>

class UtilTest : public  ::testing::Test
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

TEST_F(UtilTest, createImage)
{
    VkImage image;
    VkDeviceMemory memory;

    // Test color attachment image creation.
    createImage(
        device.device(),device.physicalDevice(), device.extent(),
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &image, &memory
    );
    EXPECT_TRUE(image);
    EXPECT_TRUE(memory);
    vkDestroyImage(device.device(), image, nullptr);
    vkFreeMemory(device.device(), memory, nullptr);

    // Test depth stencil attachment image creation.
    createImage(
        device.device(),device.physicalDevice(), device.extent(),
        device.depthFormat(), VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &image, &memory
    );
    EXPECT_TRUE(image);
    EXPECT_TRUE(memory);
    vkDestroyImage(device.device(), image, nullptr);
    vkFreeMemory(device.device(), memory, nullptr);
}

TEST_F(UtilTest, createImageView)
{
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;

    // Test color attachment image view creation.
    createImage(
        device.device(),device.physicalDevice(), device.extent(),
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &image, &memory
    );
    createImageView(
        device.device(), image, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_ASPECT_COLOR_BIT, &imageView
    );
    EXPECT_TRUE(imageView);
    vkDestroyImageView(device.device(), imageView, nullptr);
    vkDestroyImage(device.device(), image, nullptr);
    vkFreeMemory(device.device(), memory, nullptr);

    // Test depth stencil attachment image view creation.
    createImage(
        device.device(),device.physicalDevice(), device.extent(),
        device.depthFormat(), VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &image, &memory
    );
    createImageView(
        device.device(), image, device.depthFormat(),
        VK_IMAGE_ASPECT_DEPTH_BIT, &imageView
    );
    EXPECT_TRUE(imageView);
    vkDestroyImageView(device.device(), imageView, nullptr);
    vkDestroyImage(device.device(), image, nullptr);
    vkFreeMemory(device.device(), memory, nullptr);
}

TEST_F(UtilTest, createBuffer)
{
    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    createBuffer(
        device.device(), device.physicalDevice(), 1, usage, properties, &buffer,
        &memory 
    );
    EXPECT_TRUE(buffer);
    EXPECT_TRUE(memory);
    vkDestroyBuffer(device.device(),buffer,nullptr);
    vkFreeMemory(device.device(), memory, nullptr);

    usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createBuffer(
        device.device(), device.physicalDevice(), 1, usage, properties, &buffer,
        &memory 
    );
    EXPECT_TRUE(buffer);
    EXPECT_TRUE(memory);
    vkDestroyBuffer(device.device(),buffer,nullptr);
    vkFreeMemory(device.device(), memory, nullptr);
}

TEST_F(UtilTest, findMemoryType)
{
    VkImage image;
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = 100;
    imageInfo.extent.height = 100;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    vkCreateImage(device.device(), &imageInfo, nullptr, &image);
    ASSERT_TRUE(image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.device(), image, &memRequirements);

    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    auto index = findMemoryType(
        device.physicalDevice(), memRequirements.memoryTypeBits, properties
    );

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(
        device.physicalDevice(), &memProperties
    );

    VkMemoryPropertyFlags expectedProperties =
        memProperties.memoryTypes[index].propertyFlags;
    EXPECT_EQ(expectedProperties&properties,properties);

    vkDestroyImage(device.device(), image, nullptr);
}