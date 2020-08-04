#include "evulkan.h"

#include <gtest/gtest.h>

namespace evk {

class DeviceTest : public  ::testing::Test
{
    protected:
    virtual void SetUp() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
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
};


TEST_F(DeviceTest, ctor)
{
    const uint32_t numThreads = 2;
    const uint32_t swapchainSize = 2;
    Device device(
        numThreads, window, deviceExtensions, swapchainSize, validationLayers
    );

    EXPECT_NE(device.m_device.get(), nullptr);
    EXPECT_NE(device.m_commands.get(), nullptr);
    EXPECT_NE(device.m_swapchain.get(), nullptr);
    EXPECT_NE(device.m_sync.get(), nullptr);

    EXPECT_EQ(device.m_framebuffer.get(), nullptr);
    EXPECT_EQ(device.m_numThreads, numThreads);

    Device device1(
        numThreads, window, deviceExtensions, swapchainSize
    );

    EXPECT_NE(device1.m_device.get(), nullptr);
    EXPECT_NE(device1.m_commands.get(), nullptr);
    EXPECT_NE(device1.m_swapchain.get(), nullptr);
    EXPECT_NE(device1.m_sync.get(), nullptr);

    EXPECT_EQ(device1.m_framebuffer.get(), nullptr);
    EXPECT_EQ(device1.m_numThreads, numThreads);

    Device::_Device d(2,validationLayers, window, deviceExtensions);
    EXPECT_TRUE(d.m_device);
    EXPECT_TRUE(d.m_debugMessenger);
    EXPECT_TRUE(d.m_graphicsQueue);
    EXPECT_TRUE(d.m_instance);
    EXPECT_TRUE(d.m_physicalDevice);
    EXPECT_TRUE(d.m_presentQueue);
    EXPECT_TRUE(d.m_surface);

    const std::vector<const char*> validationLayers1;
    Device::_Device d1(3,validationLayers1, window, deviceExtensions);
    EXPECT_TRUE(d1.m_device);
    EXPECT_FALSE(d1.m_debugMessenger);
    EXPECT_TRUE(d1.m_graphicsQueue);
    EXPECT_TRUE(d1.m_instance);
    EXPECT_TRUE(d1.m_physicalDevice);
    EXPECT_TRUE(d1.m_presentQueue);
    EXPECT_TRUE(d1.m_surface);
}

TEST_F(DeviceTest, move)
{
    const uint32_t numThreads = 1;
    const uint32_t swapchainSize = 2;

    Device device1(
        numThreads, window, deviceExtensions, swapchainSize, validationLayers
    );

    auto device = std::move(device1);
    device1 = std::move(device);
    device = std::move(device1);
    device = std::move(device);

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment colorAttachment(device, 1, Attachment::Type::COLOR);
    Attachment depthAttachment(device, 2, Attachment::Type::DEPTH);
}

TEST_F(DeviceTest, draw)
{
    const uint32_t numThreads = 1;
    const uint32_t swapchainSize = 2;
    Device device(
        numThreads, window, deviceExtensions, swapchainSize, validationLayers
    );

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

    std::vector<Attachment*> attachments = {
        &framebufferAttachment, &depthAttachment
    };
    std::vector<Subpass*> subpasses = {&subpass};
    Renderpass renderpass(device, attachments, subpasses);

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
    device.draw();
}

} // namespace evk