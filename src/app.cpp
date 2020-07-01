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

    VkAttachmentDescription description;
    description.format = VK_FORMAT_B8G8R8A8_SRGB;
    description.flags = 0;
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    evk::Attachment attachment;
    attachment.name=evk::FRAMEBUFFER_ATTACHMENT;
    attachment.description=description;
    instance.addAttachment(attachment);

    instance.createCommandPools();

    std::cout << instance.m_commandPools.size() << std::endl;
    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    instance.createSwapChain(&swapChainCreateInfo);

    instance.createSyncObjects();
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    Descriptor descriptor(MAX_FRAMES_IN_FLIGHT,1);
    evk::loadOBJ("obj/viking_room.obj", v, in);
    instance.loadTexture("tex/viking_room.png"); // Must be before createDescriptorSets.
    descriptor.addTextureSampler(1, instance.m_textureImageView, instance.m_textureSampler, VK_SHADER_STAGE_FRAGMENT_BIT);

    const std::string DEPTH_ATTACHMENT = "depth";
    instance.addDepthAttachment(DEPTH_ATTACHMENT);
    std::vector<std::string> colorAttachments = {evk::FRAMEBUFFER_ATTACHMENT};
    std::vector<std::string> depthAttachments = {DEPTH_ATTACHMENT};
    std::vector<std::string> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    instance.addSubpass(
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments);
    instance.createRenderPass();

    const std::string VERTEX_SHADER="vert";
    const std::string FRAGMENT_SHADER="frag";
    const std::string UBO="ubo";

    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    buffer = Buffer(MAX_FRAMES_IN_FLIGHT, instance.m_device, instance.m_physicalDevice);
    buffer.setBuffer(sizeof(UniformBufferObject));
    descriptor.addUniformBuffer(0, buffer.m_buffers, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject));

    instance.registerVertexShader(VERTEX_SHADER, "shaders/vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER, "shaders/frag.spv");

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    vertexInput.setBindingDescription(sizeof(Vertex));

    std::vector<Pipeline> pipelines;
    Pipeline pipeline({VERTEX_SHADER, FRAGMENT_SHADER},&descriptor,vertexInput,0);
    pipelines.push_back(pipeline);

    indexBuffer = Buffer(MAX_FRAMES_IN_FLIGHT, instance.m_device, instance.m_physicalDevice);
    indexBuffer.setIndexBuffer(sizeof(in[0]), in.data(), in.size(), instance.m_commandPools[0], instance.m_graphicsQueue);

    instance.createVertexBuffer(v);
    
    std::vector<Descriptor*> descriptors;
    descriptors.push_back(&descriptor);

    instance.createFramebuffers();
    instance.createGraphicsPipeline(pipelines);
    instance.createDrawCommands(indexBuffer, in.size(), descriptors);
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

    VkAttachmentDescription description;
    description.format = VK_FORMAT_B8G8R8A8_SRGB;
    description.flags = 0;
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    evk::Attachment attachment;
    attachment.name=evk::FRAMEBUFFER_ATTACHMENT;
    attachment.description=description;
    instance.addAttachment(attachment);

    instance.createCommandPools();
    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    instance.createSwapChain(&swapChainCreateInfo);

    instance.createSyncObjects();

    const std::string COLOR_ATTACHMENT = "color";
    const std::string DEPTH_ATTACHMENT = "depth";

    instance.addColorAttachment(COLOR_ATTACHMENT);
    instance.addDepthAttachment(DEPTH_ATTACHMENT);

    std::vector<std::string> colorAttachments = {COLOR_ATTACHMENT};
    std::vector<std::string> depthAttachments = {DEPTH_ATTACHMENT};
    std::vector<std::string> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;
    instance.addSubpass(
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments);

    colorAttachments = {evk::FRAMEBUFFER_ATTACHMENT};
    depthAttachments.resize(0);
    inputAttachments = {COLOR_ATTACHMENT, DEPTH_ATTACHMENT};
    dependencies = {{0,1}};
    instance.addSubpass(
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments);
    instance.createRenderPass();

    auto colorImageViews = instance.m_evkattachments[COLOR_ATTACHMENT].imageViews;
    auto depthImageViews = instance.m_evkattachments[DEPTH_ATTACHMENT].imageViews;

    const std::string VERTEX_SHADER_0="vert0";
    const std::string FRAGMENT_SHADER_0="frag0";
    const std::string VERTEX_SHADER_1="vert1";
    const std::string FRAGMENT_SHADER_1="frag1";
    const std::string UBO="ubo";

    Descriptor descriptor0(MAX_FRAMES_IN_FLIGHT,1);
    Descriptor descriptor1(MAX_FRAMES_IN_FLIGHT,2);

    buffer = Buffer(MAX_FRAMES_IN_FLIGHT, instance.m_device, instance.m_physicalDevice);
    buffer.setBuffer(sizeof(UniformBufferObject));
    descriptor0.addUniformBuffer(0, buffer.m_buffers, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject));

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

    std::vector<Pipeline> pipelines;
    pipelines.push_back({{VERTEX_SHADER_0,FRAGMENT_SHADER_0},&descriptor0,vertexInput0,0});
    pipelines.push_back({{VERTEX_SHADER_1,FRAGMENT_SHADER_1},&descriptor1,vertexInput1,1});

    indexBuffer = Buffer(MAX_FRAMES_IN_FLIGHT, instance.m_device, instance.m_physicalDevice);
    indexBuffer.setIndexBuffer(sizeof(uint32_t), indices.data(), indices.size(), instance.m_commandPools[0], instance.m_graphicsQueue);

    instance.createVertexBuffer(vertices);

    std::vector<Descriptor*> descriptors = {&descriptor0, &descriptor1};

    instance.createFramebuffers();
    instance.createGraphicsPipeline(pipelines);
    instance.createDrawCommands(indexBuffer, indices.size(), descriptors);
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