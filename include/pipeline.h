#ifndef PIPELINE
#define PIPELINE

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "descriptor.h"
#include "device.h"
#include "shader.h"
#include "pass.h"

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(
        Descriptor *pDescriptor,
        const VertexInput &vertexInput,
        const size_t subpass,
        const VkExtent2D extent,
        // const VkRenderPass &renderPass,
        const Renderpass &renderpass,
        const std::vector<Shader> &shaders,
        const Device &device
    );

    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    std::vector<std::string> m_shaders;
    Descriptor* m_descriptor;
    VertexInput m_vertexInput;
    uint32_t m_subpass;

    private:
};

#endif