#include "app.h"

void App::initVulkan()
{
    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    device = {numThreads, validationLayers, window, deviceExtensions};

    const uint32_t swapchainSize = MAX_FRAMES_IN_FLIGHT;

    commands = {device, swapchainSize, numThreads};

    Attachment framebufferAttachment;
    swapchain = {swapchainSize, framebufferAttachment, device};

    sync = {device, swapchain};

    Attachment colorAttachment(device, 1);
    colorAttachment.setColorAttachment(swapchain.m_extent, device);

    Attachment depthAttachment(device, 2);
    depthAttachment.setDepthAttachment(swapchain.m_extent, device);

    std::vector<Attachment> colorAttachments = {colorAttachment};
    std::vector<Attachment> depthAttachments = {depthAttachment};
    std::vector<Attachment> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;

    Subpass subpass0(
        0,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    colorAttachments = {framebufferAttachment};
    depthAttachments.resize(0);
    inputAttachments = {colorAttachment, depthAttachment};
    dependencies = {{0,1}};

    Subpass subpass1(
        1,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    attachments = {framebufferAttachment, colorAttachment, depthAttachment};
    std::vector<Subpass> subpasses = {subpass0, subpass1};

    renderpass = {
        attachments,
        subpasses,
        device
    };

    // Set up UBO.
    buffer = Buffer(device);
    buffer.setBuffer(sizeof(UniformBufferObject));
    Descriptor descriptor0(device, MAX_FRAMES_IN_FLIGHT,1);
    descriptor0.addUniformBuffer(0, buffer, ShaderStage::VERTEX, sizeof(UniformBufferObject));

    Descriptor descriptor1(device, MAX_FRAMES_IN_FLIGHT, 3);
    descriptor1.addUniformBuffer(0, buffer, ShaderStage::VERTEX, sizeof(UniformBufferObject));
    descriptor1.addInputAttachment(0, colorAttachment, ShaderStage::FRAGMENT);
    descriptor1.addInputAttachment(1, depthAttachment, ShaderStage::FRAGMENT);

    VertexInput vertexInput0;
    vertexInput0.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput0.setBindingDescription(sizeof(Vertex));

    VertexInput vertexInput1;
    vertexInput1.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput1.setBindingDescription(sizeof(Vertex));

    indexBuffer = Buffer(device);
    indexBuffer.setIndexBuffer(indices.data(), sizeof(indices[0]), indices.size(), commands);

    vertexBuffer = Buffer(device);
    vertexBuffer.setVertexBuffer(device, vertices.data(), sizeof(vertices[0]), vertices.size(), commands);

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.setBindingDescription(sizeof(Vertex));

    std::vector<Shader> shaders0 = {
        {"shaders/multipass_0_vert.spv", ShaderStage::VERTEX, device},
        {"shaders/multipass_0_frag.spv", ShaderStage::FRAGMENT, device}
    };
    Pipeline pipeline0(
        device,
        subpass0,
        &descriptor0,
        vertexInput0,
        swapchain,
        renderpass,
        shaders0
    );

    std::vector<Shader> shaders1 = {
        {"shaders/multipass_1_vert.spv", ShaderStage::VERTEX, device},
        {"shaders/multipass_1_frag.spv", ShaderStage::FRAGMENT, device},
    };
    Pipeline pipeline1(
        device,
        subpass1,
        &descriptor1,
        vertexInput1,
        swapchain,
        renderpass,
        shaders1
    );

    pipelines = {pipeline0, pipeline1};
    descriptors = {descriptor0, descriptor1};
    for (const auto &s : shaders0) shaders.push_back(s);
    for (const auto &s : shaders1) shaders.push_back(s);
    
    recordDrawCommands(
        device, indexBuffer, vertexBuffer,
        descriptors, pipelines, renderpass,
        swapchain, framebuffers, commands);
}

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for using multipass Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    App app;
    app.run();
}
