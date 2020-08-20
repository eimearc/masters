#include "evulkan/evulkan.h"

#include "flags.h"
#include "grid.h"
#include "../util.h"

using namespace evk;

struct UniformBufferObject
{
    glm::mat4 MVP_model;
    glm::mat4 MVP_light;
    glm::mat4 MV;
};

int main(int argc, char **argv)
{
    gflags::SetUsageMessage(
        "A program for using multipass Vulkan over multiple threads."
    );
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    const uint32_t swapchainSize = 2;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    createGrid(FLAGS_num_cubes, vertices, indices);

    Device device(
        numThreads, deviceExtensions, swapchainSize, validationLayers
    );

    WindowResize r;
    createSurfaceGLFW(device, window, r);

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment colorAttachment(device, 1, Attachment::Type::COLOR);
    Attachment depthAttachment(device, 2, Attachment::Type::DEPTH);

    std::vector<Attachment*> colorAttachments = {&colorAttachment};
    std::vector<Attachment*> depthAttachments = {&depthAttachment};
    std::vector<Attachment*> inputAttachments;
    std::vector<Subpass::Dependency> dependencies;

    Subpass subpass0(
        0, dependencies, colorAttachments, depthAttachments, inputAttachments
    );

    colorAttachments = {&framebufferAttachment};
    depthAttachments.resize(0);
    inputAttachments = {&colorAttachment, &depthAttachment};
    dependencies = {0}; // Require previous subpass to complete before this one.

    Subpass subpass1(
        1, dependencies, colorAttachments, depthAttachments, inputAttachments
    );

    std::vector<Subpass*> subpasses = {&subpass0, &subpass1};
    Renderpass renderpass(device,subpasses);

    DynamicBuffer ubo(device, sizeof(UniformBufferObject), Buffer::Type::UBO);

    Descriptor descriptor0(device, swapchainSize);
    descriptor0.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);

    Descriptor descriptor1(device, swapchainSize);
    descriptor1.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);
    descriptor1.addInputAttachment(0, colorAttachment, Shader::Stage::FRAGMENT);
    descriptor1.addInputAttachment(1, depthAttachment, Shader::Stage::FRAGMENT);

    VertexInput vertexInput0(sizeof(Vertex));
    vertexInput0.setVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput0.setVertexAttributeVec3(1,offsetof(Vertex,normal));

    VertexInput vertexInput1(sizeof(Vertex));
    vertexInput1.setVertexAttributeVec3(0,offsetof(Vertex,pos));

    StaticBuffer indexBuffer(
        device, indices.data(), sizeof(indices[0]), indices.size(),
        Buffer::Type::INDEX
    );
    StaticBuffer vertexBuffer(
        device, vertices.data(), sizeof(vertices[0]), vertices.size(),
        Buffer::Type::VERTEX
    );

    Shader vertexShader0(device, "pass_0_vert.spv", Shader::Stage::VERTEX);
    Shader fragmentShader0(device, "pass_0_frag.spv", Shader::Stage::FRAGMENT);
    std::vector<Shader*> shaders0 = {&vertexShader0, &fragmentShader0};

    Pipeline pipeline0(
        device, subpass0, descriptor0, vertexInput0, renderpass, shaders0
    );

    Shader vertexShader1(device, "pass_1_vert.spv", Shader::Stage::VERTEX);
    Shader fragmentShader1(device, "pass_1_frag.spv", Shader::Stage::FRAGMENT);
    std::vector<Shader*> shaders1 = {&vertexShader1, &fragmentShader1};

    Pipeline pipeline1(
        device, subpass1, descriptor1, vertexInput1, renderpass, shaders1
    );
    std::vector<Pipeline*> pipelines = {&pipeline0, &pipeline1};

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
        model = glm::rotate(
            glm::mat4(1.0f), 0.005f * glm::radians(90.0f)*counter,
            glm::vec3(0.0f,0.0f,1.0f)
        );
        view = glm::lookAt(
            glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        proj = glm::perspective(
            glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f
        );
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
