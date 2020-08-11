#ifndef EVK_EXAMPLES_BENCH_MULTIPASS_H_
#define EVK_EXAMPLES_BENCH_MULTIPASS_H_

#include "evulkan.h"
#include "../util.h"
#include "../multipass/grid.h"

using namespace evk;

class MultipassBench
{
    public:
    struct UniformBufferObject
    {
        glm::mat4 MVP_model;
        glm::mat4 MVP_light;
        glm::mat4 MV;
    };

    MultipassBench(GLFWwindow *window, size_t numThreads)
    {
        const uint32_t swapchainSize = 2;

        createGrid(100, vertices, indices);

        device = Device(
            numThreads, deviceExtensions, swapchainSize, validationLayers
        );

        WindowResize r;
        createSurfaceGLFW(device, window, r);

        framebufferAttachment = Attachment(device, 0, Attachment::Type::FRAMEBUFFER);
        colorAttachment = Attachment(device, 1, Attachment::Type::COLOR);
        depthAttachment = Attachment(device, 2, Attachment::Type::DEPTH);

        std::vector<Attachment*> colorAttachments = {&colorAttachment};
        std::vector<Attachment*> depthAttachments = {&depthAttachment};
        std::vector<Attachment*> inputAttachments;
        std::vector<Subpass::Dependency> dependencies;

        subpass0 = Subpass(
            0, dependencies, colorAttachments, depthAttachments, inputAttachments
        );

        colorAttachments = {&framebufferAttachment};
        depthAttachments.resize(0);
        inputAttachments = {&colorAttachment, &depthAttachment};
        dependencies = {0};

        subpass1 = Subpass(
            1, dependencies, colorAttachments, depthAttachments, inputAttachments
        );

        std::vector<Subpass*> subpasses = {&subpass0, &subpass1};
        renderpass = Renderpass(device,subpasses);

        ubo = DynamicBuffer(device, sizeof(UniformBufferObject));

        descriptor0 = Descriptor(device, swapchainSize);
        descriptor0.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);

        descriptor1 = Descriptor(device, swapchainSize);
        descriptor1.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);
        descriptor1.addInputAttachment(0, colorAttachment, Shader::Stage::FRAGMENT);
        descriptor1.addInputAttachment(1, depthAttachment, Shader::Stage::FRAGMENT);

        vertexInput0 = VertexInput(sizeof(Vertex));
        vertexInput0.setVertexAttributeVec3(0,offsetof(Vertex,pos));
        vertexInput0.setVertexAttributeVec3(1,offsetof(Vertex,normal));

        vertexInput1 = VertexInput(sizeof(Vertex));
        vertexInput1.setVertexAttributeVec3(0,offsetof(Vertex,pos));

        indexBuffer = StaticBuffer(
            device, indices.data(), sizeof(indices[0]), indices.size(),
            Buffer::INDEX
        );
        vertexBuffer = StaticBuffer(
            device, vertices.data(), sizeof(vertices[0]), vertices.size(),
            Buffer::VERTEX
        );

        vertexShader0 = Shader(device, "pass_0_vert.spv", Shader::Stage::VERTEX);
        fragmentShader0 = Shader(device, "pass_0_frag.spv", Shader::Stage::FRAGMENT);
        std::vector<Shader*> shaders0 = {&vertexShader0, &fragmentShader0};

        pipeline0 = Pipeline(
            device, subpass0, descriptor0, vertexInput0, renderpass, shaders0
        );

        vertexShader1 = Shader(device, "pass_1_vert.spv", Shader::Stage::VERTEX);
        fragmentShader1 = Shader(device, "pass_1_frag.spv", Shader::Stage::FRAGMENT);
        std::vector<Shader*> shaders1 = {&vertexShader1, &fragmentShader1};

        pipeline1 = Pipeline(
            device, subpass1, descriptor1, vertexInput1, renderpass, shaders1
        );
        std::vector<Pipeline*> pipelines = {&pipeline0, &pipeline1};

        device.finalize(indexBuffer,vertexBuffer,pipelines);
    }

    void draw()
    {
        static size_t counter=0;
        glm::mat4 model(1.0f);
        glm::mat4 view(1.0f);
        glm::mat4 proj(1.0f);

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

    size_t numVerts()
    {
        return vertices.size();
    }

    private:
    Device device;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Attachment framebufferAttachment;
    Attachment depthAttachment;
    Attachment colorAttachment;
    Subpass subpass0;
    Subpass subpass1;
    Renderpass renderpass;
    DynamicBuffer ubo;
    Descriptor descriptor0;
    Descriptor descriptor1;
    VertexInput vertexInput0;
    VertexInput vertexInput1;
    StaticBuffer indexBuffer;
    StaticBuffer vertexBuffer;
    Shader vertexShader0;
    Shader fragmentShader0;
    Shader vertexShader1;
    Shader fragmentShader1;
    Pipeline pipeline0;
    Pipeline pipeline1;
};

#endif
