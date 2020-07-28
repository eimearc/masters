#include "evulkan.h"
#include "flags.h"
#include "grid.h"

struct UniformBufferObject
{
    glm::mat4 MVP_model;
    glm::mat4 MVP_light;
    glm::mat4 MV;
};
std::vector<const char*> validationLayers =
{
    "VK_LAYER_LUNARG_standard_validation"
};
std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

GLFWwindow *window;

void createGrid(
    uint32_t numCubes,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices)
{
    constexpr size_t NUM_VERTS = 8;
    constexpr float GRID_SIZE = 2.0f;
    numCubes = sqrt(numCubes);
    float cubeSize = (GRID_SIZE/numCubes)*0.5;
    Grid grid = Grid(GRID_SIZE, cubeSize, numCubes);
    int i=0;
    Vertex vertex;
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint32_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color={1,0,1};
            vertex.normal=-vertex.pos;
            vertices.push_back(vertex);
        }
        for(size_t j = 0; j<ind.size(); ++j)
        {
            indices.push_back(ind[j]+i*NUM_VERTS);
        }
        ++i;
    }
}

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for using multipass Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    const uint32_t swapchainSize = 2;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    createGrid(FLAGS_num_cubes, vertices, indices);

    Device device1(numThreads, validationLayers, window, deviceExtensions, swapchainSize, true);
    auto device = std::move(device1);
    device1 = std::move(device);
    device = std::move(device1);
    device = std::move(device);

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment colorAttachment(device, 1, Attachment::Type::COLOR);
    Attachment depthAttachment(device, 2, Attachment::Type::DEPTH);

    std::vector<Attachment*> colorAttachments = {&colorAttachment};
    std::vector<Attachment*> depthAttachments = {&depthAttachment};
    std::vector<Attachment*> inputAttachments;
    std::vector<evk::SubpassDependency> dependencies;

    Subpass subpass0(
        0,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    colorAttachments = {&framebufferAttachment};
    depthAttachments.resize(0);
    inputAttachments = {&colorAttachment, &depthAttachment};
    dependencies = {{0,1}};

    Subpass subpass1(
        1,
        dependencies,
        colorAttachments,
        depthAttachments,
        inputAttachments
    );

    std::vector<Attachment*> attachments = {&framebufferAttachment, &colorAttachment, &depthAttachment};
    std::vector<Subpass*> subpasses = {&subpass0, &subpass1};
    Renderpass renderpass(device,attachments,subpasses);

    // Set up UBO.
    DynamicBuffer ubo(device, sizeof(UniformBufferObject));

    Descriptor descriptor0(device, swapchainSize, 1);
    descriptor0.addUniformBuffer(0, ubo, Shader::Stage::VERTEX, sizeof(UniformBufferObject));

    Descriptor descriptor1(device, swapchainSize, 3);
    descriptor1.addUniformBuffer(0, ubo, Shader::Stage::VERTEX, sizeof(UniformBufferObject));
    descriptor1.addInputAttachment(0, colorAttachment, Shader::Stage::FRAGMENT);
    descriptor1.addInputAttachment(1, depthAttachment, Shader::Stage::FRAGMENT); // TODO: If depth input attachment.

    VertexInput vertexInput0(sizeof(Vertex));
    vertexInput0.addVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput0.addVertexAttributeVec3(1,offsetof(Vertex,normal));

    VertexInput vertexInput1(sizeof(Vertex));
    vertexInput1.addVertexAttributeVec3(0,offsetof(Vertex,pos));

    StaticBuffer indexBuffer(device, indices.data(), sizeof(indices[0]), indices.size(), Buffer::INDEX);
    StaticBuffer vertexBuffer(device, vertices.data(), sizeof(vertices[0]), vertices.size(), Buffer::VERTEX);

    Shader vertexShader0(device, "pass_0_vert.spv", Shader::Stage::VERTEX);
    Shader fragmentShader0(device, "pass_0_frag.spv", Shader::Stage::FRAGMENT);
    std::vector<Shader*> shaders0 = {&vertexShader0, &fragmentShader0};
    Pipeline pipeline0(
        device,
        &subpass0,
        &descriptor0,
        vertexInput0,
        &renderpass,
        shaders0
    );

    Shader vertexShader1(device, "pass_1_vert.spv", Shader::Stage::VERTEX);
    Shader fragmentShader1(device, "pass_1_frag.spv", Shader::Stage::FRAGMENT);
    std::vector<Shader*> shaders1 = {&vertexShader1, &fragmentShader1};
    Pipeline pipeline1(
        device,
        &subpass1,
        &descriptor1,
        vertexInput1,
        &renderpass,
        shaders1
    );

    std::vector<Pipeline*> pipelines = {&pipeline0, &pipeline1};
    std::vector<Shader*> shaders;
    for (const auto &s : shaders0) shaders.push_back(s);
    for (const auto &s : shaders1) shaders.push_back(s);

    device.finalize(indexBuffer,vertexBuffer,pipelines);

    // Main loop.
    size_t counter=0;
    glm::mat4 model(1.0f);
    glm::mat4 view(1.0f);
    glm::mat4 proj(1.0f);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        UniformBufferObject uboUpdate = {};
        model = glm::rotate(glm::mat4(1.0f), 0.005f * glm::radians(90.0f)*counter, glm::vec3(0.0f,0.0f,1.0f));
        view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        proj = glm::perspective(glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f);
        proj[1][1] *= -1;
        uboUpdate.MV = view * model;
        uboUpdate.MVP_model = proj * view * model;
        uboUpdate.MVP_light = proj * view;

        ubo.update(&uboUpdate);

        device.draw();

        counter++;
    }

    // Tidy up.
    glfwDestroyWindow(window);
    glfwTerminate();
}
