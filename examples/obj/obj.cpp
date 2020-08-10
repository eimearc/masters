#include "evulkan.h"

#include "flags.h"
#include "../util.h"

using namespace evk; // TODO: Remove.

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

int main(int argc, char **argv)
{
    gflags::SetUsageMessage(
        "A program demonstrating how to use OBJs and textures in Vulkan."
    );
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow *window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    const uint32_t numThreads = static_cast<uint32_t>(FLAGS_num_threads);
    const uint32_t swapchainSize = 2;

    Device device(
        numThreads, deviceExtensions, swapchainSize, validationLayers
    );

    WindowResize r;
    createSurfaceGLFW(device, window, r);
    
    std::vector<Vertex> v;
    std::vector<uint32_t> in;
    evk::loadOBJ("viking_room.obj", v, in);

    Texture texture(device, "viking_room.png");

    Descriptor descriptor(device, swapchainSize);
    descriptor.addTextureSampler(1, texture, Shader::Stage::FRAGMENT);

    Attachment framebufferAttachment(device, 0, Attachment::Type::FRAMEBUFFER);
    Attachment depthAttachment(device, 1, Attachment::Type::DEPTH);

    std::vector<Attachment*> colorAttachments = {&framebufferAttachment};
    std::vector<Attachment*> depthAttachments = {&depthAttachment};
    std::vector<Attachment*> inputAttachments;
    std::vector<Subpass::Dependency> dependencies;
    
    Subpass subpass(
        0, dependencies, colorAttachments, depthAttachments, inputAttachments
    );

    std::vector<Subpass*> subpasses = {&subpass};
    Renderpass renderpass(device,subpasses);

    UniformBufferObject uboUpdate = {};
    uboUpdate.model=glm::mat4(1.0f);
    uboUpdate.model=glm::rotate(
        glm::mat4(1.0f), 0.001f * glm::radians(90.0f)*0,
        glm::vec3(0.0f,0.0f,1.0f)
    );
    uboUpdate.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    uboUpdate.proj = glm::perspective(
        glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f
    );
    uboUpdate.proj[1][1] *= -1;
    DynamicBuffer ubo(device, &uboUpdate, sizeof(uboUpdate), 1, Buffer::UBO);
    descriptor.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);

    VertexInput vertexInput(sizeof(Vertex));
    vertexInput.setVertexAttributeVec3(0,offsetof(Vertex,pos));
    vertexInput.setVertexAttributeVec3(1,offsetof(Vertex,color));
    vertexInput.setVertexAttributeVec2(2,offsetof(Vertex,texCoord));

    StaticBuffer indexBuffer(
        device, in.data(), sizeof(in[0]), in.size(), Buffer::INDEX
    );
    StaticBuffer vertexBuffer(
        device, v.data(), sizeof(v[0]), v.size(), Buffer::VERTEX
    );

    Shader vertexShader(device, "shader_vert.spv", Shader::Stage::VERTEX);
    Shader fragmentShader(device, "shader_frag.spv", Shader::Stage::FRAGMENT);
    std::vector<Shader*> shaders = {&vertexShader,&fragmentShader};

    Pipeline pipeline(
        device, &subpass, descriptor, vertexInput, renderpass, shaders
    );

    std::vector<Pipeline*> pipelines = {&pipeline};

    device.finalize(indexBuffer,vertexBuffer,pipelines);

    // Main loop.
    size_t counter=0;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        UniformBufferObject uboUpdate = {};
        uboUpdate.model=glm::mat4(1.0f);
        uboUpdate.model=glm::rotate(
            glm::mat4(1.0f), 0.001f * glm::radians(90.0f)*counter,
            glm::vec3(0.0f,0.0f,1.0f)
        );
        uboUpdate.view = glm::lookAt(
            glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        uboUpdate.proj = glm::perspective(
            glm::radians(45.0f), 800 / (float) 600 , 0.1f, 10.0f
        );
        uboUpdate.proj[1][1] *= -1;
        ubo.update(&uboUpdate);

        device.draw();

        counter++;
    }
}
