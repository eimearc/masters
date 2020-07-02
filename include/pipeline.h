#ifndef PIPELINE
#define PIPELINE

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "descriptor.h"
#include "device.h"

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(
        Descriptor *pDescriptor,
        const VertexInput &vertexInput,
        const size_t subpass,
        const VkExtent2D extent,
        const VkRenderPass &renderPass,
        const std::vector<VkPipelineShaderStageCreateInfo> &shaders,
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

class Shader
{
    public:
    Shader()=default;
    
};

#endif