#include "evulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace evk {

class PipelineTest : public  ::testing::Test
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
        std::vector<Subpass::Dependency> dependencies;
        
        subpass = {
            0, dependencies, colorAttachments, depthAttachments,
            inputAttachments
        };

        std::vector<Subpass*> subpasses = {&subpass};
        renderpass = {device, subpasses};

        vertexInput = {sizeof(Vertex)};
        vertexInput.setVertexAttributeVec3(0,offsetof(Vertex,pos));
        vertexInput.setVertexAttributeVec3(1,offsetof(Vertex,color));

        StaticBuffer indexBuffer(
            device, indices.data(), sizeof(indices[0]), indices.size(),
            Buffer::Type::INDEX
        );
        StaticBuffer vertexBuffer(
            device, vertices.data(), sizeof(vertices[0]), vertices.size(),
            Buffer::Type::VERTEX
        );

        vertexShader = Shader(device, "shader_vert.spv", Shader::Stage::VERTEX);
        fragmentShader = Shader(device, "shader_frag.spv", Shader::Stage::FRAGMENT);
        shaders = {&vertexShader,&fragmentShader};
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
    std::vector<Attachment*> inputAttachments;
    Pipeline pipeline0;
    Pipeline pipeline1;
    Subpass subpass;
    Shader vertexShader;
    Shader fragmentShader;
    std::vector<Shader*> shaders;
    VertexInput vertexInput;
    Renderpass renderpass;
};

TEST_F(PipelineTest,ctor)
{
    pipeline0 = {
        device, subpass, vertexInput, renderpass, shaders
    };
    EXPECT_EQ(pipeline0.m_descriptor, nullptr);
    if (pipeline0.m_device==VK_NULL_HANDLE) FAIL();
    if (pipeline0.m_layout==VK_NULL_HANDLE) FAIL();
    if (pipeline0.m_pipeline==VK_NULL_HANDLE) FAIL();
    EXPECT_NE(pipeline0.m_renderpass,nullptr);
    EXPECT_EQ(pipeline0.m_shaders.size(), 2);
    EXPECT_NE(pipeline0.m_subpass, nullptr);

    EXPECT_FALSE(pipeline0.m_descriptor);
    EXPECT_TRUE(pipeline0.m_device);
    EXPECT_TRUE(pipeline0.m_layout);
    EXPECT_TRUE(pipeline0.m_pipeline);

    EXPECT_TRUE(pipeline0==pipeline0);
    EXPECT_FALSE(pipeline0!=pipeline0);

    VertexInput empty;
    EXPECT_NE(pipeline0.m_vertexInput, empty);

    DynamicBuffer buffer(device, 10, Buffer::Type::UBO);
    Descriptor descriptor(device, 2);
    descriptor.addUniformBuffer(0, buffer, Shader::Stage::VERTEX);

    pipeline1 = {
        device, subpass, descriptor, vertexInput, renderpass, shaders
    };
    EXPECT_NE(pipeline1.m_descriptor, nullptr);
    if (pipeline1.m_device==VK_NULL_HANDLE) FAIL();
    if (pipeline1.m_layout==VK_NULL_HANDLE) FAIL();
    if (pipeline1.m_pipeline==VK_NULL_HANDLE) FAIL();
    EXPECT_NE(pipeline1.m_renderpass,nullptr);
    EXPECT_EQ(pipeline1.m_shaders.size(), 2);
    EXPECT_NE(pipeline1.m_subpass, nullptr);

    EXPECT_TRUE(pipeline1.m_descriptor);
    EXPECT_TRUE(pipeline1.m_device);
    EXPECT_TRUE(pipeline1.m_layout);
    EXPECT_TRUE(pipeline1.m_pipeline);
    EXPECT_NE(pipeline1.m_vertexInput, empty);

    EXPECT_TRUE(pipeline1==pipeline1);
    EXPECT_FALSE(pipeline1!=pipeline1);

    EXPECT_FALSE(pipeline0==pipeline1);
    EXPECT_TRUE(pipeline0!=pipeline1);
}

} // namespace evk