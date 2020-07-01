#include "pipeline.h"

Pipeline::Pipeline(
    const std::vector<std::string> &shaders,
    Descriptor *pDescriptor,
    const VertexInput &vertexInput,
    const size_t subpass)
{
    m_shaders = shaders;
    m_descriptor = pDescriptor;
    m_vertexInput = vertexInput;
    m_subpass = subpass;
}