#ifndef EVK_PIPELINE
#define EVK_PIPELINE

#include "descriptor.h"
#include "device.h"
#include "shader.h"
#include "pass.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(
        const Subpass &subpass,
        Descriptor *pDescriptor,
        const VertexInput &vertexInput,
        Renderpass *pRenderpass,
        const std::vector<Shader> &shaders,
        bool writeDepth
    );

    void setup(
        Device &device
    );

    void destroy();

    VkDevice m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    Descriptor* m_descriptor;
    VertexInput m_vertexInput;
    uint32_t m_subpass;
    std::vector<Shader> m_shaders;
    Renderpass *m_renderpass;

    private:
    bool m_writeDepth;
};

#endif