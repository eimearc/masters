#include "evulkan.h"
#include "flags.h"

const uint32_t MAX_FRAMES_IN_FLIGHT=2;
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

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program demonstrating how to use OBJs and textures in Vulkan.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    const uint32_t swapchainSize = MAX_FRAMES_IN_FLIGHT;

    Device device = Device(numThreads, validationLayers, window, deviceExtensions);

    Commands commands = Commands(device, swapchainSize, numThreads);

    Attachment framebuffer;
    Swapchain swapchain = Swapchain(device,swapchainSize,framebuffer);

    Sync sync = Sync(device, swapchain);
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    Descriptor descriptor(device, MAX_FRAMES_IN_FLIGHT,1);
    evk::loadOBJ("viking_room.obj", v, in);

    Texture texture = Texture("viking_room.png", device, commands);
    descriptor.addTextureSampler(1, texture, ShaderStage::FRAGMENT);

    Attachment depthAttachment(device, 1);
    depthAttachment.setDepthAttachment(swapchain.m_extent, device);

    std::vector<Attachment> colorAttachments = {framebuffer};
    std::vector<Attachment> depthAttachments = {depthAttachment};
    std::vector<Attachment> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    
    Subpass subpass(
        0,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    std::vector<Attachment> attachments = {framebuffer, depthAttachment};
    std::vector<Subpass> subpasses = {subpass};
    Renderpass renderpass = {
        attachments,
        subpasses,
        device
    };

    Buffer ubo = Buffer(device);
    ubo.setBuffer(sizeof(UniformBufferObject));
    descriptor.addUniformBuffer(0, ubo, ShaderStage::VERTEX, sizeof(UniformBufferObject));

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    vertexInput.setBindingDescription(sizeof(Vertex));

    Buffer indexBuffer = Buffer(device, in.data(), sizeof(in[0]), in.size());
    Buffer vertexBuffer = Buffer(device, v.data(), sizeof(v[0]), v.size());

    Shader vertexShader("shader_vert.spv", ShaderStage::VERTEX, device);
    Shader fragmentShader("shader_frag.spv", ShaderStage::FRAGMENT, device);
    std::vector<Shader> shaders = {vertexShader,fragmentShader};

    Pipeline pipeline(
        subpass,
        &descriptor,
        vertexInput,
        renderpass,
        shaders
    );

    std::vector<Pipeline> pipelines = {pipeline};
    Framebuffer framebuffers;
    recordDrawCommands(
        device, indexBuffer, vertexBuffer,
        pipelines, renderpass,
        swapchain, framebuffers, commands);

    // Main loop.
    size_t frameIndex=0;
    size_t counter=0;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        UniformBufferObject uboUpdate = {};
        uboUpdate.model=glm::mat4(1.0f);
        uboUpdate.model=glm::rotate(glm::mat4(1.0f), 0.001f * glm::radians(90.0f)*counter, glm::vec3(0.0f,0.0f,1.0f));
        uboUpdate.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        uboUpdate.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
        uboUpdate.proj[1][1] *= -1;

        ubo.updateBuffer(&uboUpdate);

        executeDrawCommands(device, pipelines, swapchain, commands, sync);

        frameIndex=(frameIndex+1)%MAX_FRAMES_IN_FLIGHT;
        counter++;
    }

    // Tidy.
    ubo.destroy();
    indexBuffer.destroy();
    vertexBuffer.destroy();
    texture.destroy();
    for (auto &a : attachments) a.destroy();
    framebuffers.destroy();
    swapchain.destroy();
    commands.destroy();
    descriptor.destroy();
    for (auto &p : pipelines) p.destroy();
    for (auto &s : shaders) s.destroy();
    renderpass.destroy();
    sync.destroy();
    device.destroy();
}
