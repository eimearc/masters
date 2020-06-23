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

    evkInstance.createBufferObject("UBO", sizeof(UniformBufferObject));
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    evk::loadOBJ("obj/viking_room.obj", v, in);
    evkInstance.loadTexture("tex/viking_room.png"); // Must be before createDescriptorSets.

    evkInstance.registerVertexShader("vert", "shaders/vert.spv");
    evkInstance.registerFragmentShader("frag", "shaders/frag.spv");

    evkInstance.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    evkInstance.addVertexAttributeVec3(1,offsetof(Vertex,color));
    evkInstance.addVertexAttributeVec2(2,offsetof(Vertex,texCoord));
    evkInstance.setBindingDescription(sizeof(Vertex));

    evkInstance.createDescriptorSets();
    evkInstance.createGraphicsPipeline();

    evkInstance.createFramebuffers();

    evkInstance.createIndexBuffer(in);
    evkInstance.createVertexBuffer(v);
    
    evkInstance.createDrawCommands();
}

void App::initMultipassVulkan()
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
    multipassInstance=evk::Instance(FLAGS_num_threads, &instanceCreateInfo);

    multipassInstance.createCommandPools();
    evk::SwapChainCreateInfo swapChainCreateInfo{
        static_cast<uint8_t>(MAX_FRAMES_IN_FLIGHT)
    };
    multipassInstance.createSwapChain(&swapChainCreateInfo);

    multipassInstance.createSyncObjects();

    /** Pipeline 0 / Subpass 0 **/
    multipassInstance.addColorAttachment();
    multipassInstance.addDepthAttachment();
    std::vector<VkAttachmentReference> colorAttachments = {{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }};
    std::vector<VkAttachmentReference> inputAttachments;
    inputAttachments.push_back({ 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    inputAttachments.push_back({ 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    multipassInstance.addDependency(0,1); // Add dependency between stage 0 and stage 1.
    multipassInstance.addSubpass(colorAttachments,inputAttachments);
    multipassInstance.createRenderPass();

    multipassInstance.createBufferObject("UBO", sizeof(UniformBufferObject));
    multipassInstance.registerVertexShader("vert", "shaders/multipass_vert.spv");
    multipassInstance.registerFragmentShader("frag", "shaders/multipass_frag.spv");
    multipassInstance.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    multipassInstance.addVertexAttributeVec3(1,offsetof(Vertex,color));
    multipassInstance.setBindingDescription(sizeof(Vertex));
    multipassInstance.createDescriptorSets();

    multipassInstance.addPipeline({"vert", "frag"},0);
    multipassInstance.addPipeline({"vert", "frag"},1);

    multipassInstance.createGraphicsPipeline();
    /** End of Pipeline **/

    multipassInstance.createFramebuffers();
    multipassInstance.createIndexBuffer(indices);
    multipassInstance.createVertexBuffer(vertices);
    
    multipassInstance.createDrawCommands();
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