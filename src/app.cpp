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
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    evk::InstanceCreateInfo instanceCreateInfo{
        validationLayers,
        window,
        deviceExtensions
    };
    evkInstance=evk::Instance(FLAGS_num_threads, &instanceCreateInfo);

    evkInstance.createCommandPools();
    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    evkInstance.createSwapChain(&swapChainCreateInfo);

    evkInstance.createSyncObjects();
    evkInstance.createRenderPass();

    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    evk::loadOBJ("obj/viking_room.obj", v, in);
    evkInstance.loadTexture("tex/viking_room.png"); // Must be before createDescriptorSets.

    evkInstance.createUniformBufferObject();
    evkInstance.createDescriptorSets();

    evkInstance.registerVertexShader("shaders/vert.spv");
    evkInstance.registerFragmentShader("shaders/frag.spv");

    evkInstance.addVertexAttribute(0,offsetof(Vertex,pos));
    evkInstance.addVertexAttribute(1,offsetof(Vertex,color));
    evkInstance.setBindingDescription(sizeof(Vertex));

    evkInstance.createGraphicsPipeline();

    evkInstance.createDepthResources();
    evkInstance.createFramebuffers();

    evkInstance.createIndexBuffer(in);
    evkInstance.createVertexBuffer(v);
    
    evkInstance.createDrawCommands(in);
}

void App::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        evkInstance.draw();
    }
}