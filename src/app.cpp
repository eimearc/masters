#include "app.h"

void App::createGrid()
{
    float gridSize = 2.0f;
    float cubeSize = (gridSize/NUM_CUBES)*0.5;
    grid = Grid(gridSize, cubeSize, NUM_CUBES);
    int i=0;
    const size_t numVerts = 8;
    Vertex vertex = {{}, {1,1,1}};
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint32_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color={1,0,1};
            vertices.push_back(vertex);
        }
        for(size_t j = 0; j<ind.size(); ++j)
        {
            indices.push_back(ind[j]+i*numVerts);
        }
        ++i;
    }
}

void App::initVulkan()
{
    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    const uint32_t swapchainSize = MAX_FRAMES_IN_FLIGHT;

    device = {numThreads, validationLayers, window, deviceExtensions};

    commands = {device, swapchainSize, FLAGS_num_threads};

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
    evk::loadOBJ("obj/viking_room.obj", v, in);

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

void App::initMultipassVulkan()
{
    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    device = {numThreads, validationLayers, window, deviceExtensions};

    const uint32_t swapchainSize = MAX_FRAMES_IN_FLIGHT;

    commands = {device, swapchainSize, FLAGS_num_threads};

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

void App::mainLoop()
{
    size_t frameIndex=0;
    size_t counter=0;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        UniformBufferObject ubo = {};
        ubo.model=glm::mat4(1.0f);
        ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f)*counter, glm::vec3(0.0f,0.0f,1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        buffer.updateBuffer(&ubo);

        executeDrawCommands(device, pipelines, swapchain, commands, sync);

        frameIndex=(frameIndex+1)%MAX_FRAMES_IN_FLIGHT;
        counter++;
    }
}