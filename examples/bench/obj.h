#ifndef EVK_EXAMPLES_BENCH_OBJ_H_
#define EVK_EXAMPLES_BENCH_OBJ_H_

#include "evulkan.h"
#include "../util.h"

using namespace evk;

class ObjBench
{
    public:
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    ObjBench(GLFWwindow *window, size_t numThreads)
    {
        const uint32_t swapchainSize = 2;

        device = Device(
            numThreads, deviceExtensions, swapchainSize, validationLayers
        );

        WindowResize r;
        createSurfaceGLFW(device, window, r);
        
        evk::loadOBJ("viking_room.obj", vertices, indices);

        texture = Texture(device, "viking_room.png");

        descriptor = Descriptor(device, swapchainSize);
        descriptor.addTextureSampler(1, texture, Shader::Stage::FRAGMENT);

        framebufferAttachment = Attachment(device, 0, Attachment::Type::FRAMEBUFFER);
        depthAttachment = Attachment(device, 1, Attachment::Type::DEPTH);

        std::vector<Attachment*> colorAttachments = {&framebufferAttachment};
        std::vector<Attachment*> depthAttachments = {&depthAttachment};
        std::vector<Attachment*> inputAttachments;
        std::vector<Subpass::Dependency> dependencies;
        
        subpass = Subpass(
            0, dependencies, colorAttachments, depthAttachments, inputAttachments
        );

        std::vector<Subpass*> subpasses = {&subpass};
        renderpass = Renderpass(device,subpasses);

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
        ubo = DynamicBuffer(device, &uboUpdate, sizeof(uboUpdate), 1, Buffer::UBO);
        descriptor.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);

        vertexInput = VertexInput(sizeof(Vertex));
        vertexInput.setVertexAttributeVec3(0,offsetof(Vertex,pos));
        vertexInput.setVertexAttributeVec3(1,offsetof(Vertex,color));
        vertexInput.setVertexAttributeVec2(2,offsetof(Vertex,texCoord));

        indexBuffer = StaticBuffer(
            device, indices.data(), sizeof(indices[0]), indices.size(),
            Buffer::INDEX
        );
        vertexBuffer = StaticBuffer(
            device, vertices.data(), sizeof(vertices[0]), vertices.size(),
            Buffer::VERTEX
        );

        vertexShader = Shader(device, "obj_vert.spv", Shader::Stage::VERTEX);
        fragmentShader = Shader(device, "obj_frag.spv", Shader::Stage::FRAGMENT);
        std::vector<Shader*> shaders = {&vertexShader,&fragmentShader};

        pipeline = Pipeline(
            device, subpass, descriptor, vertexInput, renderpass, shaders
        );

        std::vector<Pipeline*> pipelines = {&pipeline};

        device.finalize(indexBuffer,vertexBuffer,pipelines);
    }

    void draw()
    {
        static size_t counter=0;

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

    size_t numVerts()
    {
        return vertices.size();
    }

    private:
    Device device;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Descriptor descriptor;
    DynamicBuffer ubo;
    Texture texture;
    Attachment framebufferAttachment;
    Attachment depthAttachment;
    Subpass subpass;
    Renderpass renderpass;
    VertexInput vertexInput;
    StaticBuffer indexBuffer;
    StaticBuffer vertexBuffer;
    Shader vertexShader;
    Shader fragmentShader;
    Pipeline pipeline;
};

#endif