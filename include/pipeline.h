#ifndef EVK_PIPELINE
#define EVK_PIPELINE

#include "descriptor.h"
#include "device.h"
#include "shader.h"
#include "swapchain.h"
#include "pass.h"
#include "util.h"
#include <vulkan/vulkan.h>

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(
        const Device &device,
        const Subpass &subpass,
        Descriptor *pDescriptor,
        const VertexInput &vertexInput,
        const Swapchain &swapchain,
        const Renderpass &renderpass,
        const std::vector<Shader> &shaders
    );

    void destroy();

    VkDevice m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    std::vector<std::string> m_shaders;
    Descriptor* m_descriptor;
    VertexInput m_vertexInput;
    uint32_t m_subpass;

    private:
};

#endif