#include "app.h"

void App::initVulkan()
{
    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    const uint32_t swapchainSize = MAX_FRAMES_IN_FLIGHT;

    device = {numThreads, validationLayers, window, deviceExtensions};

    commands = {device, swapchainSize, numThreads};

    Attachment framebuffer;
    swapchain = {
        swapchainSize,
        framebuffer,
        device
    };

    sync = {device, swapchain};
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    Descriptor descriptor(device, MAX_FRAMES_IN_FLIGHT,1);
    evk::loadOBJ("models/viking_room.obj", v, in);

    texture = { // Must be before createDescriptorSets.
        "tex/viking_room.png",
        device,
        commands
    };
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

    attachments = {framebuffer, depthAttachment};
    std::vector<Subpass> subpasses = {subpass};
    renderpass = {
        attachments,
        subpasses,
        device
    };

    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    // Set up UBO.
    buffer = Buffer(device);
    buffer.setBuffer(sizeof(UniformBufferObject));
    descriptor.addUniformBuffer(0, buffer, ShaderStage::VERTEX, sizeof(UniformBufferObject));

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    vertexInput.setBindingDescription(sizeof(Vertex));

    indexBuffer = Buffer(device);
    indexBuffer.setIndexBuffer(in.data(), sizeof(in[0]), in.size(), commands);

    vertexBuffer = Buffer(device);
    vertexBuffer.setVertexBuffer(device, v.data(), sizeof(v[0]), v.size(), commands);

    Shader vertexShader("shaders/vert.spv", ShaderStage::VERTEX, device);
    Shader fragmentShader("shaders/frag.spv", ShaderStage::FRAGMENT, device);
    shaders = {vertexShader,fragmentShader};

    Pipeline pipeline(
        device,
        subpass,
        &descriptor,
        vertexInput,
        swapchain,
        renderpass,
        shaders
    );

    descriptors = {descriptor};
    pipelines = {pipeline};

    recordDrawCommands(
        device, indexBuffer, vertexBuffer,
        descriptors, pipelines, renderpass,
        swapchain, framebuffers, commands);
}

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program demonstrating how to use OBJs and textures in Vulkan.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    App app;
    app.run();
}
