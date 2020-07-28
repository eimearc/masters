#include "evulkan.h"

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
std::vector<const char*> validationLayers =
{
    "VK_LAYER_LUNARG_standard_validation"
};
std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<Vertex> setupVerts()
{
    std::vector<Vertex> verts;
    Vertex v;
    v.pos={0,-0.5,0};
    v.color={1,0,0};
    verts.push_back(v);
    v.pos={-0.5,0.5,0};
    v.color={0,0,1};
    verts.push_back(v);
    v.pos={0.5,0.5,0};
    v.color={0,1,0};
    verts.push_back(v);
    return verts;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    const uint32_t numThreads = 1;
    const uint32_t swapchainSize = 2;

    Device device(numThreads, validationLayers, window, deviceExtensions, swapchainSize, true);
    
    std::vector<Vertex> v=setupVerts();
    std::vector<uint32_t> in={0,1,2};
    Descriptor descriptor(device, swapchainSize, 1);

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment depthAttachment(device, 1, Attachment::Type::DEPTH);

    std::vector<Attachment*> colorAttachments = {&framebufferAttachment};
    std::vector<Attachment*> depthAttachments = {&depthAttachment};
    std::vector<Attachment*> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    
    Subpass subpass(
        0,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    std::vector<Attachment*> attachments = {&framebufferAttachment, &depthAttachment};
    std::vector<Subpass*> subpasses = {&subpass};
    Renderpass renderpass(device,attachments,subpasses);

    // UniformBufferObject uboUpdate = {};
    // uboUpdate.model=glm::mat4(1.0f);
    // uboUpdate.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // uboUpdate.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
    // StaticBuffer ubo(device, &uboUpdate, sizeof(uboUpdate), 1, Buffer::UBO);
    // descriptor.addUniformBuffer(0, ubo, Shader::Stage::VERTEX, sizeof(uboUpdate));

    VertexInput vertexInput(sizeof(Vertex));
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));

    StaticBuffer indexBuffer(device, in.data(), sizeof(in[0]), in.size(), Buffer::INDEX);
    StaticBuffer vertexBuffer(device, v.data(), sizeof(v[0]), v.size(), Buffer::VERTEX);

    Shader vertexShader(device, "shader_vert.spv", Shader::Stage::VERTEX);
    Shader fragmentShader(device, "shader_frag.spv", Shader::Stage::FRAGMENT);
    std::vector<Shader*> shaders = {&vertexShader,&fragmentShader};

    Pipeline pipeline(
        device,
        subpass,
        &descriptor,
        vertexInput,
        &renderpass,
        shaders,
        true
    );

    std::vector<Pipeline*> pipelines = {&pipeline};
    device.finalize(indexBuffer,vertexBuffer,pipelines);

    // Main loop.
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        device.draw();
    }
}
