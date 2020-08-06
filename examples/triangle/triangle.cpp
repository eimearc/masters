#include "evulkan.h"

using namespace evk; // TODO: Remove.

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

    Device device(
        numThreads, deviceExtensions, swapchainSize, validationLayers
    );

    auto surfaceFunc = [&](){
        glfwCreateWindowSurface(
            device.instance(), window, nullptr, &device.surface()
        );
    };

    // int width, height;
    // glfwGetFramebufferSize(window, &width, &height);

    device.setWindowFunc(surfaceFunc,800,600);
    
    std::vector<Vertex> vertices=setupVerts();
    std::vector<uint32_t> indices={0,1,2};

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment depthAttachment(device, 1, Attachment::Type::DEPTH);

    std::vector<Attachment*> colorAttachments = {&framebufferAttachment};
    std::vector<Attachment*> depthAttachments = {&depthAttachment};
    std::vector<Attachment*> inputAttachments;
    std::vector<Subpass::Dependency> dependencies;
    
    Subpass subpass(
        0, dependencies, colorAttachments, depthAttachments, inputAttachments
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

    // Main loop.
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        device.draw();
    }
}
