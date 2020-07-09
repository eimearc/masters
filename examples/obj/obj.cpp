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
    evk::loadOBJ("viking_room.obj", v, in);

    texture = { // Must be before createDescriptorSets.
        "viking_room.png",
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

    // Set up UBO.
    ubo = Buffer(device);
    ubo.setBuffer(sizeof(UniformBufferObject));
    descriptor.addUniformBuffer(0, ubo, ShaderStage::VERTEX, sizeof(UniformBufferObject));

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    vertexInput.setBindingDescription(sizeof(Vertex));

    indexBuffer = Buffer(device);
    indexBuffer.setIndexBuffer(in.data(), sizeof(in[0]), in.size(), commands);

    vertexBuffer = Buffer(device);
    vertexBuffer.setVertexBuffer(device, v.data(), sizeof(v[0]), v.size(), commands);

    Shader vertexShader("shader_vert.spv", ShaderStage::VERTEX, device);
    Shader fragmentShader("shader_frag.spv", ShaderStage::FRAGMENT, device);
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
