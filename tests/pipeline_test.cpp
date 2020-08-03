#include "evulkan.h"

#include <gtest/gtest.h>

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
        std::vector<evk::SubpassDependency> dependencies;
        
        Subpass subpass(
            0, dependencies, colorAttachments, depthAttachments,
            inputAttachments
        );

        std::vector<Attachment*> attachments = {
            &framebufferAttachment, &depthAttachment
        };
        std::vector<Subpass*> subpasses = {&subpass};
        renderpass = {device, attachments, subpasses};

        VertexInput vertexInput(sizeof(Vertex));
        vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
        vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));

        StaticBuffer indexBuffer(
            device, indices.data(), sizeof(indices[0]), indices.size(),
            Buffer::INDEX
        );
        StaticBuffer vertexBuffer(
            device, vertices.data(), sizeof(vertices[0]), vertices.size(),
            Buffer::VERTEX
        );

        Shader vertexShader(device, "shader_vert.spv", Shader::Stage::VERTEX);
        Shader fragmentShader(device, "shader_frag.spv", Shader::Stage::FRAGMENT);
        std::vector<Shader*> shaders = {&vertexShader,&fragmentShader};

        pipeline = {
            device, &subpass, vertexInput, &renderpass, shaders
        };
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
    Pipeline pipeline;
    Renderpass renderpass;
};

TEST_F(PipelineTest,ctor)
{
    EXPECT_EQ(pipeline.m_descriptor, nullptr);
    if (pipeline.m_device==VK_NULL_HANDLE) FAIL();
    if (pipeline.m_layout==VK_NULL_HANDLE) FAIL();
    if (pipeline.m_pipeline==VK_NULL_HANDLE) FAIL();
    EXPECT_NE(pipeline.m_renderpass,nullptr);
    EXPECT_EQ(pipeline.m_shaders.size(), 2);
    EXPECT_NE(pipeline.m_subpass, nullptr);
    VertexInput empty;
    EXPECT_NE(pipeline.m_vertexInput, empty);
}