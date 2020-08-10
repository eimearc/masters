#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

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
            numThreads, deviceExtensions, swapchainSize,
            validationLayers
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

        std::vector<Subpass*> subpasses = {&subpass};
        renderpass={device, subpasses};
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
    std::vector<Subpass::Dependency> dependencies;
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

    EXPECT_TRUE(subpass==subpass);
    EXPECT_FALSE(subpass!=subpass);

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
    EXPECT_FALSE(renderpass.m_renderPass==VK_NULL_HANDLE);
    EXPECT_EQ(renderpass.m_device, device.m_device->m_device);
    EXPECT_EQ(renderpass.m_attachments.size(),2);
    EXPECT_EQ(renderpass.m_clearValues.size(),2);
    EXPECT_EQ(renderpass.m_subpasses.size(),1);

    EXPECT_TRUE(renderpass==renderpass);
    EXPECT_FALSE(renderpass!=renderpass);
}

bool descriptionEqual(
    const VkAttachmentDescription &a,
    const VkAttachmentDescription &b
)
{
    if (a.finalLayout!=b.finalLayout) return false;
    if (a.flags!=b.flags) return false;
    if (a.format!=b.format) return false;
    if (a.initialLayout!=b.initialLayout) return false;
    if (a.loadOp!=b.loadOp) return false;
    if (a.samples!=b.samples) return false;
    if (a.stencilLoadOp!=b.stencilLoadOp) return false;
    if (a.stencilStoreOp!=b.stencilStoreOp) return false;
    if (a.storeOp!=b.storeOp) return false;
    return true;
}

bool clearValueEqual(
    const VkClearValue &a,
    const VkClearValue &b
)
{
    float e=0.00001;
    if (abs((*a.color.float32)-(*b.color.float32))>e) return false;
    if (*a.color.int32!=*b.color.int32) return false;
    if (*a.color.uint32!=*b.color.uint32) return false;
    if ((a.depthStencil.depth)!=(b.depthStencil.depth)) return false;
    if (a.depthStencil.stencil!=b.depthStencil.stencil) return false;
    return true;
}

TEST_F(PassTest, constructDescriptions)
{
    Attachment a0(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment a1(device, 1, Attachment::Type::DEPTH);
    Attachment a2(device, 2, Attachment::Type::COLOR);
    std::vector<Subpass::Dependency> dep = {};

    std::vector<Attachment*> c;
    std::vector<Attachment*> d;
    std::vector<Attachment*> i;

    c = {&a2};
    d = {&a1};
    Subpass s0(0, dep, c, d, i);
    i = {&a1,&a2};
    c = {&a0};
    d = {};
    Subpass s1(1, dep, c, d, i);

    std::vector<Subpass*> subpasses = {&s0,&s1};
    auto got = (Renderpass::attachmentInfo(subpasses));
    std::vector<VkAttachmentDescription> expectedDesc = {
        a0.description(), a1.description(), a2.description()
    };
    std::vector<VkClearValue> expectedClear = {
        a0.clearValue(), a1.clearValue(), a2.clearValue()
    };

    EXPECT_EQ(got.clearValues.size(), expectedClear.size());
    EXPECT_TRUE(
        std::equal(
            got.clearValues.begin(), got.clearValues.end(),
            expectedClear.begin(), clearValueEqual
        )
    );

    std::vector<Attachment*> expectedAttachments = {
        &a0, &a1, &a2
    };
    EXPECT_EQ(got.attachments.size(), expectedAttachments.size());
    EXPECT_TRUE(
        std::equal(
            got.attachments.begin(), got.attachments.end(),
            expectedAttachments.begin()
        )
    ); 
}

} // namespace evk