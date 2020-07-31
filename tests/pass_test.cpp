#include "evulkan.h"

#include <gtest/gtest.h>

class PassTest : public  ::testing::Test
{
    protected:
    virtual void SetUp() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

        const uint32_t numThreads = 1;
        const uint32_t swapchainSize = 2;
        device = {
            numThreads, window, deviceExtensions, swapchainSize,
            validationLayers
        };

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        evk::loadOBJ("tri.obj",vertices,indices);

        Attachment framebufferAttachment(
            device, 0, Attachment::Type::FRAMEBUFFER
        );
        Attachment depthAttachment(device, 1, Attachment::Type::DEPTH);

        colorAttachments = {&framebufferAttachment};
        depthAttachments = {&depthAttachment};
        
        subpass = {
            0, dependencies, colorAttachments, depthAttachments,
            inputAttachments
        };

        std::vector<Attachment*> attachments = {
            &framebufferAttachment, &depthAttachment
        };
        std::vector<Subpass*> subpasses = {&subpass};
        renderpass={device, attachments, subpasses};

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
    std::vector<Attachment*> colorAttachments;
    std::vector<Attachment*> depthAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    std::vector<Attachment*> inputAttachments;
    Subpass subpass;
    Renderpass renderpass;
};

TEST_F(PassTest,ctor)
{
    // Subpass ctor.
    if (!std::equal(
        subpass.m_colorReferences.begin(), subpass.m_colorReferences.end(),
        subpass.m_colorReferences.begin(),Subpass::referenceEqual))
    FAIL();

    EXPECT_EQ(subpass.m_colorAttachments, colorAttachments);

    std::vector<VkAttachmentReference> colorReferences = 
    {
        {0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    if (!std::equal(
            subpass.m_colorReferences.begin(), subpass.m_colorReferences.end(),
            colorReferences.begin(),Subpass::referenceEqual))
        FAIL();
    EXPECT_EQ(subpass.m_depthAttachments, depthAttachments);

    std::vector<VkAttachmentReference> depthReferences = 
    {
        {1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
    };
    if (!std::equal(
            subpass.m_depthReferences.begin(), subpass.m_depthReferences.end(),
            depthReferences.begin(),Subpass::referenceEqual))
        FAIL();
    EXPECT_EQ(subpass.m_inputAttachments, inputAttachments);
    EXPECT_EQ(subpass.m_dependencies.size(), 0);
    EXPECT_EQ(subpass.m_index, 0);

    EXPECT_EQ(subpass,subpass);

    // Renderpass ctor.
    if (renderpass.m_device==VK_NULL_HANDLE) FAIL();
    EXPECT_EQ(renderpass.m_device, device.m_device->m_device);
}