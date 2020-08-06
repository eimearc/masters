#include "evulkan.h"

#include <gtest/gtest.h>

namespace evk {

class FramebufferTest : public  ::testing::Test
{
    protected:
    virtual void SetUp() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

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

    Device device;
    GLFWwindow *window;
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
};

TEST_F(FramebufferTest, ctor)
{
    ASSERT_EQ(device.m_framebuffer,nullptr);

    std::vector<Vertex> vertices;
    Vertex v;
    v.pos={0,-0.5,0};
    v.color={1,0,0};
    vertices.push_back(v);
    v.pos={-0.5,0.5,0};
    v.color={0,0,1};
    vertices.push_back(v);
    v.pos={0.5,0.5,0};
    v.color={0,1,0};
    vertices.push_back(v);
    std::vector<uint32_t> indices={0,1,2};

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment depthAttachment(device, 1, Attachment::Type::DEPTH);

    std::vector<Attachment*> colorAttachments = {&framebufferAttachment};
    std::vector<Attachment*> depthAttachments = {&depthAttachment};
    std::vector<Attachment*> inputAttachments;
    std::vector<Subpass::Dependency> dependencies;
    
    Subpass subpass(
        0,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    std::vector<Subpass*> subpasses = {&subpass};
    Renderpass renderpass(device, subpasses);

    VertexInput vertexInput(sizeof(Vertex));
    vertexInput.setVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.setVertexAttributeVec3(1,offsetof(Vertex,color));

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

    Pipeline pipeline(device, &subpass, vertexInput, &renderpass, shaders);
    std::vector<Pipeline*> pipelines = {&pipeline};
    
    device.finalize(indexBuffer,vertexBuffer,pipelines);

    const auto &framebuffer = device.m_framebuffer;
    ASSERT_NE(framebuffer,nullptr);
    ASSERT_EQ(framebuffer->m_device->device(), device.device());
    ASSERT_EQ(framebuffer->m_framebuffers.size(), swapchainSize);
}

} // namespace evk