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
            vertex.color=cube.color;
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

    evk::InstanceCreateInfo instanceCreateInfo{
        validationLayers,
        window,
        deviceExtensions
    };
    instance=evk::Instance(FLAGS_num_threads, &instanceCreateInfo);

    instance.createCommandPools();
    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    instance.createSwapChain(&swapChainCreateInfo);

    instance.createSyncObjects();
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    Descriptor descriptor(MAX_FRAMES_IN_FLIGHT);
    evk::loadOBJ("obj/viking_room.obj", v, in);
    instance.loadTexture("tex/viking_room.png"); // Must be before createDescriptorSets.
    descriptor.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptor.addWriteDescriptorSetTextureSampler(instance.m_textureImageView, instance.m_textureSampler, 1);

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
    instance.createBufferObject(UBO, sizeof(UniformBufferObject));
    descriptor.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
    descriptor.addWriteDescriptorSetBuffer(instance.m_buffers, 0, sizeof(UniformBufferObject), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);

    instance.registerVertexShader(VERTEX_SHADER, "shaders/vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER, "shaders/frag.spv");

    VertexInput vertexInput;
    vertexInput.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    vertexInput.setBindingDescription(sizeof(Vertex));

    instance.addPipeline({VERTEX_SHADER, FRAGMENT_SHADER},descriptor,vertexInput,0);

    instance.createIndexBuffer(in);
    instance.createVertexBuffer(v);
    
    instance.createDescriptorSets();
    instance.createFramebuffers();
    instance.createGraphicsPipeline();
    instance.createDrawCommands();
}

void App::initMultipassVulkan()
{
    auto &instance = multipassInstance;

    evk::InstanceCreateInfo instanceCreateInfo{
        validationLayers,
        window,
        deviceExtensions
    };
    instance=evk::Instance(FLAGS_num_threads, &instanceCreateInfo);

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

    const std::string VERTEX_SHADER_0="vert0";
    const std::string FRAGMENT_SHADER_0="frag0";
    const std::string VERTEX_SHADER_1="vert1";
    const std::string FRAGMENT_SHADER_1="frag1";
    const std::string UBO="ubo";

    Descriptor descriptor0(MAX_FRAMES_IN_FLIGHT);
    Descriptor descriptor1(MAX_FRAMES_IN_FLIGHT);

    instance.createBufferObject(UBO, sizeof(UniformBufferObject));
    descriptor0.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT);
    descriptor0.addWriteDescriptorSetBuffer(instance.m_buffers, 0, sizeof(UniformBufferObject), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);

    auto colorImageViews = instance.m_evkattachments[COLOR_ATTACHMENT].imageViews;
    auto depthImageViews = instance.m_evkattachments[DEPTH_ATTACHMENT].imageViews;

    descriptor1.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptor1.addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptor1.addWriteDescriptorSetInputAttachment(colorImageViews, 0);
    descriptor1.addWriteDescriptorSetInputAttachment(depthImageViews, 1);

    instance.registerVertexShader(VERTEX_SHADER_0, "shaders/multipass_0_vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER_0, "shaders/multipass_0_frag.spv");
    instance.registerVertexShader(VERTEX_SHADER_1, "shaders/multipass_1_vert.spv");
    instance.registerFragmentShader(FRAGMENT_SHADER_1, "shaders/multipass_1_frag.spv");

    VertexInput vertexInput0;
    vertexInput0.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput0.addVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput0.setBindingDescription(sizeof(Vertex));

    VertexInput vertexInput1;
    vertexInput1.setBindingDescription(0);

    instance.addPipeline({VERTEX_SHADER_0, FRAGMENT_SHADER_0},descriptor0,vertexInput0,0);
    instance.addPipeline({VERTEX_SHADER_1, FRAGMENT_SHADER_1},descriptor1,vertexInput1,1);

    instance.createIndexBuffer(indices);
    instance.createVertexBuffer(vertices);
    
    instance.createDescriptorSets();
    instance.createFramebuffers();
    instance.createGraphicsPipeline();
    instance.createDrawCommands();
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

        instance.updateBufferObject("UBO",sizeof(ubo), &ubo, frameIndex);

        instance.draw();

        frameIndex=(frameIndex+1)%MAX_FRAMES_IN_FLIGHT;
        counter++;
    }
}