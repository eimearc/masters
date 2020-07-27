#ifndef EVK_PIPELINE_H_
#define EVK_PIPELINE_H_

#include "descriptor.h"
#include "device.h"
#include "shader.h"
#include "pass.h"
#include "util.h"
#include <vulkan/vulkan.h>

#include "vertexinput.h"

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(const Pipeline&)=delete;
    Pipeline& operator=(const Pipeline&)=delete;
    Pipeline(Pipeline&&)=delete;
    Pipeline& operator=(Pipeline&&)=delete;
    ~Pipeline() noexcept;
    Pipeline(
        Device &device,
        const Subpass &subpass,
        Descriptor *pDescriptor,
        const VertexInput &vertexInput,
        Renderpass *pRenderpass,
        const std::vector<Shader*> &shaders,
        bool writeDepth
    );

    VkDevice m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    Descriptor* m_descriptor;
    VertexInput m_vertexInput;
    uint32_t m_subpass;
    std::vector<Shader*> m_shaders;
    Renderpass *m_renderpass;

    private:
    bool m_writeDepth;

    void setup(
        Device &device
    );
};

#endif