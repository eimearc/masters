#include "evulkan.h"
#include "flags.h"
#include "grid.h"

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

void createGrid(
    uint32_t numCubes,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices)
{
    numCubes = sqrt(numCubes);
    float gridSize = 2.0f;
    float cubeSize = (gridSize/numCubes)*0.5;
    Grid grid = Grid(gridSize, cubeSize, numCubes);
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

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for using multipass Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    auto window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    createGrid(FLAGS_num_cubes, vertices, indices);

    Device device = {numThreads, validationLayers, window, deviceExtensions};

    const uint32_t swapchainSize = MAX_FRAMES_IN_FLIGHT;

    Commands commands = {device, swapchainSize, numThreads};

    Swapchain swapchain = {device, swapchainSize};

    Sync sync = {device, swapchain};

    Attachment framebufferAttachment(device, 0);
    framebufferAttachment.setFramebufferAttachment();

    Attachment colorAttachment(device, 1);
    colorAttachment.setColorAttachment(device, swapchain);

    Attachment depthAttachment(device, 2);
    depthAttachment.setDepthAttachment(device, swapchain);

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

    std::vector<Attachment> attachments = {framebufferAttachment, colorAttachment, depthAttachment};
    std::vector<Subpass> subpasses = {subpass0, subpass1};
    Renderpass renderpass = {device,attachments,subpasses};

    // Set up UBO.
    DynamicBuffer ubo = DynamicBuffer(device, sizeof(UniformBufferObject));

    Descriptor descriptor0(device, MAX_FRAMES_IN_FLIGHT, 1);
    descriptor0.addUniformBuffer(0, ubo, ShaderStage::VERTEX, sizeof(UniformBufferObject));

    Descriptor descriptor1(device, MAX_FRAMES_IN_FLIGHT, 3);
    descriptor1.addUniformBuffer(0, ubo, ShaderStage::VERTEX, sizeof(UniformBufferObject));
    descriptor1.addInputAttachment(0, colorAttachment, ShaderStage::FRAGMENT);
    descriptor1.addInputAttachment(1, depthAttachment, ShaderStage::FRAGMENT);

    VertexInput vertexInput0(sizeof(Vertex));
    vertexInput0.addVertexAttributeVec3(0,offsetof(Vertex,pos));

    VertexInput vertexInput1(sizeof(Vertex));
    vertexInput1.addVertexAttributeVec3(0,offsetof(Vertex,pos));

    StaticBuffer indexBuffer = StaticBuffer(device, commands, indices.data(), sizeof(indices[0]), indices.size(), Buffer::INDEX);
    StaticBuffer vertexBuffer = StaticBuffer(device, commands, vertices.data(), sizeof(vertices[0]), vertices.size(), Buffer::VERTEX);
    vertexBuffer.finalizeVertex(device, commands);
    indexBuffer.finalizeIndex(device, commands);

    std::vector<Shader> shaders0 = {
        {"pass_0_vert.spv", ShaderStage::VERTEX, device},
        {"pass_0_frag.spv", ShaderStage::FRAGMENT, device}
    };
    Pipeline pipeline0(
        subpass0,
        &descriptor0,
        vertexInput0,
        renderpass,
        shaders0
    );

    std::vector<Shader> shaders1 = {
        {"pass_1_vert.spv", ShaderStage::VERTEX, device},
        {"pass_1_frag.spv", ShaderStage::FRAGMENT, device},
    };
    Pipeline pipeline1(
        subpass1,
        &descriptor1,
        vertexInput1,
        renderpass,
        shaders1
    );

    std::vector<Pipeline> pipelines = {pipeline0, pipeline1};
    std::vector<Shader> shaders;
    for (const auto &s : shaders0) shaders.push_back(s);
    for (const auto &s : shaders1) shaders.push_back(s);
    
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
        uboUpdate.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f)*counter, glm::vec3(0.0f,0.0f,1.0f));
        uboUpdate.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        uboUpdate.proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
        uboUpdate.proj[1][1] *= -1;

        ubo.update(&uboUpdate);

        executeDrawCommands(device, pipelines, swapchain, commands, sync);

        frameIndex=(frameIndex+1)%MAX_FRAMES_IN_FLIGHT;
        counter++;
    }

    // Tidy up.
    ubo.destroy();
    indexBuffer.destroy();
    vertexBuffer.destroy();
    for (auto &a : attachments) a.destroy();
    framebuffers.destroy();
    swapchain.destroy();
    commands.destroy();
    descriptor0.destroy();
    descriptor1.destroy();
    for (auto &p : pipelines) p.destroy();
    for (auto &s : shaders) s.destroy();
    renderpass.destroy();
    sync.destroy();
    device.destroy(); 
}
