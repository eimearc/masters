#include "evulkan.h"

#include <gtest/gtest.h>

namespace evk {

class ShaderTest : public  ::testing::Test
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
        std::vector<Subpass::Dependency> dependencies;
        
        Subpass subpass(
            0, dependencies, colorAttachments, depthAttachments,
            inputAttachments
        );

        std::vector<Subpass*> subpasses = {&subpass};
        renderpass = {device, subpasses};

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

        vertexShader={device, "shader_vert.spv", Shader::Stage::VERTEX};
        fragmentShader={
            device, "shader_frag.spv", Shader::Stage::FRAGMENT
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
    Shader fragmentShader;
    Shader vertexShader;
};

TEST_F(ShaderTest,ctor)
{
    if (vertexShader.m_device==VK_NULL_HANDLE) FAIL();
    if (vertexShader.m_module==VK_NULL_HANDLE) FAIL();
    EXPECT_EQ(
        vertexShader.m_createInfo.sType,
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    );
    EXPECT_EQ(vertexShader.m_createInfo.stage, VK_SHADER_STAGE_VERTEX_BIT);
    EXPECT_EQ(vertexShader.m_createInfo.module, vertexShader.m_module);
    std::string name(vertexShader.m_createInfo.pName);
    EXPECT_EQ(name, "main");
    EXPECT_EQ(vertexShader.m_createInfo.pNext, nullptr);
    EXPECT_EQ(vertexShader.m_createInfo.flags, 0);
    EXPECT_EQ(vertexShader.m_createInfo.pSpecializationInfo, nullptr);
    if (vertexShader!=vertexShader) FAIL();
    EXPECT_TRUE(vertexShader.m_device);
    EXPECT_TRUE(vertexShader.m_module);

    if (fragmentShader.m_device==VK_NULL_HANDLE) FAIL();
    if (fragmentShader.m_module==VK_NULL_HANDLE) FAIL();
    EXPECT_EQ(
        fragmentShader.m_createInfo.sType,
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    );
    EXPECT_EQ(fragmentShader.m_createInfo.stage, VK_SHADER_STAGE_FRAGMENT_BIT);
    EXPECT_EQ(fragmentShader.m_createInfo.module, fragmentShader.m_module);
    name = fragmentShader.m_createInfo.pName;
    EXPECT_EQ(name, "main");
    EXPECT_EQ(fragmentShader.m_createInfo.pNext, nullptr);
    EXPECT_EQ(fragmentShader.m_createInfo.flags, 0);
    EXPECT_EQ(fragmentShader.m_createInfo.pSpecializationInfo, nullptr);
    if (fragmentShader!=fragmentShader) FAIL();  
    EXPECT_TRUE(fragmentShader.m_device);
    EXPECT_TRUE(fragmentShader.m_module);  
}

} // namespace evk