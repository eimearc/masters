#include "app.h"

void App::createGrid()
{
    float gridSize = 2.0f;
    float cubeSize = (gridSize/NUM_CUBES)*0.5;
    grid = Grid(gridSize, cubeSize, NUM_CUBES);
    int i=0;
    const size_t numVerts = 8;
    Vertex vertex = {{}, {1,0,0}};
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint32_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color={0,0,1};
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
    auto &instance = evkInstance;

    Device device(FLAGS_num_threads, validationLayers, window, deviceExtensions);

    instance.m_threadPool.setThreadCount(FLAGS_num_threads);
    instance.m_physicalDevice=device.m_physicalDevice;
    instance.m_debugMessenger=device.m_debugMessenger;
    instance.m_surface=device.m_surface;
    instance.m_graphicsQueue=device.m_graphicsQueue;
    instance.m_presentQueue=device.m_presentQueue;
    instance.m_device=device.m_device;
    instance.m_numThreads=device.m_numThreads;

    instance.createCommandPools();

    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };

    Attachment framebuffer(0,MAX_FRAMES_IN_FLIGHT);
    framebuffer.setFramebufferAttachment(); // Must be before createSwapChain.

    instance.createSwapChain(&swapChainCreateInfo,framebuffer);

    instance.createSyncObjects();
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    Descriptor descriptor(MAX_FRAMES_IN_FLIGHT,1);
    evk::loadOBJ("obj/viking_room.obj", v, in);
    instance.loadTexture("tex/viking_room.png"); // Must be before createDescriptorSets.
    descriptor.addTextureSampler(1, instance.m_textureImageView, instance.m_textureSampler, VK_SHADER_STAGE_FRAGMENT_BIT);

    Attachment depthAttachment(1,MAX_FRAMES_IN_FLIGHT);
    evk::EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = instance.m_swapChainImageFormat;
    renderPassInfo.physicalDevice = instance.m_physicalDevice;
    VkFormat depthFormat = instance.findDepthFormat(&renderPassInfo);
    depthAttachment.setDepthAttachment(instance.m_swapChainExtent, depthFormat, device);

    std::vector<Attachment> colorAttachments = {framebuffer};
    std::vector<Attachment> depthAttachments = {depthAttachment};
    std::vector<Attachment> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    instance.addSubpass(
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments);
    std::vector<Attachment> attachments = {framebuffer, depthAttachment};
    instance.createRenderPass(attachments);

    const std::string VERTEX_SHADER="vert";
    const std::string FRAGMENT_SHADER="frag";
    const std::string UBO="ubo";

    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    buffer = Buffer(MAX_FRAMES_IN_FLIGHT, device);
    buffer.setBuffer(sizeof(UniformBufferObject));

    descriptor.addUniformBuffer(0, buffer.m_buffers, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject));

    instance.registerVertexShader(VERTEX_SHADER, "shaders/vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER, "shaders/frag.spv");

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    vertexInput.setBindingDescription(sizeof(Vertex));

    indexBuffer = Buffer(MAX_FRAMES_IN_FLIGHT, device);
    indexBuffer.setIndexBuffer(in.data(), sizeof(in[0]), in.size(), instance.m_commandPools[0]);

    vertexBuffer = Buffer(MAX_FRAMES_IN_FLIGHT, device);
    vertexBuffer.setVertexBuffer(v.data(), sizeof(v[0]), v.size(), device, instance.m_commandPools);
    
    std::vector<Descriptor*> descriptors = {&descriptor};

    instance.createFramebuffers(attachments);

    std::vector<Pipeline> pipelines = {
        {{VERTEX_SHADER, FRAGMENT_SHADER},&descriptor,vertexInput,0}
    };
    instance.createGraphicsPipeline(pipelines);

    instance.createDrawCommands(indexBuffer, vertexBuffer, descriptors);
}

void App::initMultipassVulkan()
{
    auto &instance = multipassInstance;

    Device device(FLAGS_num_threads, validationLayers, window, deviceExtensions);

    instance.m_threadPool.setThreadCount(FLAGS_num_threads);
    instance.m_physicalDevice=device.m_physicalDevice;
    instance.m_debugMessenger=device.m_debugMessenger;
    instance.m_surface=device.m_surface;
    instance.m_graphicsQueue=device.m_graphicsQueue;
    instance.m_presentQueue=device.m_presentQueue;
    instance.m_device=device.m_device;
    instance.m_numThreads=device.m_numThreads;

    instance.createCommandPools();
    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };

    Attachment framebuffer(0,MAX_FRAMES_IN_FLIGHT);
    framebuffer.setFramebufferAttachment();

    instance.createSwapChain(&swapChainCreateInfo, framebuffer);

    instance.createSyncObjects();

    Attachment colorAttachment(1,MAX_FRAMES_IN_FLIGHT);
    colorAttachment.setColorAttachment(instance.m_swapChainExtent, device);

    Attachment depthAttachment(2,MAX_FRAMES_IN_FLIGHT);
    evk::EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = instance.m_swapChainImageFormat;
    renderPassInfo.physicalDevice = instance.m_physicalDevice;
    VkFormat depthFormat = instance.findDepthFormat(&renderPassInfo);
    depthAttachment.setDepthAttachment(instance.m_swapChainExtent, depthFormat, device);

    std::vector<Attachment> colorAttachments = {colorAttachment};
    std::vector<Attachment> depthAttachments = {depthAttachment};
    std::vector<Attachment> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    instance.addSubpass(
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments);

    colorAttachments = {framebuffer};
    depthAttachments.resize(0);
    inputAttachments = {colorAttachment, depthAttachment};
    dependencies = {{0,1}};
    instance.addSubpass(
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments);
    std::vector<Attachment> attachments = {framebuffer, colorAttachment, depthAttachment};
    instance.createRenderPass(attachments);

    auto colorImageViews = colorAttachment.m_imageViews;
    auto depthImageViews = depthAttachment.m_imageViews;

    const std::string VERTEX_SHADER_0="vert0";
    const std::string FRAGMENT_SHADER_0="frag0";
    const std::string VERTEX_SHADER_1="vert1";
    const std::string FRAGMENT_SHADER_1="frag1";
    const std::string UBO="ubo";

    buffer = Buffer(MAX_FRAMES_IN_FLIGHT, device);
    buffer.setBuffer(sizeof(UniformBufferObject));

    Descriptor descriptor0(MAX_FRAMES_IN_FLIGHT,1);
    descriptor0.addUniformBuffer(0, buffer.m_buffers, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject));
    Descriptor descriptor1(MAX_FRAMES_IN_FLIGHT,2);
    descriptor1.addInputAttachment(0, colorImageViews, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptor1.addInputAttachment(1, depthImageViews, VK_SHADER_STAGE_FRAGMENT_BIT);

    instance.registerVertexShader(VERTEX_SHADER_0, "shaders/multipass_0_vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER_0, "shaders/multipass_0_frag.spv");
    instance.registerVertexShader(VERTEX_SHADER_1, "shaders/multipass_1_vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER_1, "shaders/multipass_1_frag.spv");

    VertexInput vertexInput0;
    vertexInput0.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput0.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput0.setBindingDescription(sizeof(Vertex));

    VertexInput vertexInput1;
    vertexInput1.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput1.setBindingDescription(sizeof(Vertex));

    indexBuffer = Buffer(MAX_FRAMES_IN_FLIGHT, device);
    indexBuffer.setIndexBuffer(indices.data(), sizeof(indices[0]), indices.size(), instance.m_commandPools[0]);

    vertexBuffer = Buffer(MAX_FRAMES_IN_FLIGHT, device);
    vertexBuffer.setVertexBuffer(vertices.data(), sizeof(vertices[0]), vertices.size(), device, instance.m_commandPools);

    std::vector<Descriptor*> descriptors = {&descriptor0, &descriptor1};

    instance.createFramebuffers(attachments);

    std::vector<Pipeline> pipelines = {
        {{VERTEX_SHADER_0,FRAGMENT_SHADER_0},&descriptor0,vertexInput0,0},
        {{VERTEX_SHADER_1,FRAGMENT_SHADER_1},&descriptor1,vertexInput1,1}
    };
    instance.createGraphicsPipeline(pipelines); // Pipeline is next.

    instance.createDrawCommands(indexBuffer, vertexBuffer, descriptors);
}

void App::mainLoop(evk::Instance &instance)
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

        instance.draw();

        frameIndex=(frameIndex+1)%MAX_FRAMES_IN_FLIGHT;
        counter++;
    }
}