#ifndef PIPELINE
#define PIPELINE

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "descriptor.h"

class Pipeline
{
    public:
    Pipeline()=default;
    Pipeline(
        const std::vector<std::string> &shaders, //Instead, make shader module.
        Descriptor *pDescriptor,
        const VertexInput &vertexInput,
        const size_t subpass);

    std::vector<std::string> m_shaders;
    Descriptor* m_descriptor;
    VertexInput m_vertexInput;
    uint32_t m_subpass;

    private:
};

#endif