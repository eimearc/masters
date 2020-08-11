#ifndef EVK_EXAMPLES_BENCH_TRIANGLE_H_
#define EVK_EXAMPLES_BENCH_TRIANGLE_H_

#include "evulkan.h"

#include "../util.h"

using namespace evk;

class TriangleBench
{
    public:
    TriangleBench()=default;
    ~TriangleBench()=default;

    TriangleBench(GLFWwindow *window, size_t numThreads)
    {
        const uint32_t swapchainSize = 2;

        device = Device(
            numThreads, deviceExtensions, swapchainSize, validationLayers
        );

        WindowResize r;
        createSurfaceGLFW(device,window,r);

        vertices=setupVerts();
        indices={0,1,2};

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
        renderpass = Renderpass(device, subpasses);

        vertexInput = VertexInput(sizeof(Vertex));
        vertexInput.setVertexAttributeVec3(0,offsetof(Vertex,pos));
        vertexInput.setVertexAttributeVec3(1,offsetof(Vertex,color));

        indexBuffer = StaticBuffer(
            device, indices.data(), sizeof(indices[0]), indices.size(),
            Buffer::INDEX
        );
        vertexBuffer = StaticBuffer(
            device, vertices.data(), sizeof(vertices[0]), vertices.size(),
            Buffer::VERTEX
        );

        vertexShader = Shader(device, "shader_vert.spv", Shader::Stage::VERTEX);
        fragmentShader = Shader(device, "shader_frag.spv", Shader::Stage::FRAGMENT);
        std::vector<Shader*> shaders = {&vertexShader,&fragmentShader};

        pipeline = Pipeline(device, subpass, vertexInput, renderpass, shaders);
        std::vector<Pipeline*> pipelines = {&pipeline};
        
        device.finalize(indexBuffer,vertexBuffer,pipelines);
    };

    // Main loop.
    void draw()
    {
        device.draw();
    }

    private:
    Device device;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
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