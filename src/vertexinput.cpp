#include "vertexinput.h"

VertexInput::VertexInput(uint32_t stride)
{
    setBindingDescription(stride);
}

bool VertexInput::operator==(const VertexInput &other) const
{
    if (m_attributeDescriptions.size()!=other.m_attributeDescriptions.size())
        return false;
    if (!std::equal(
            m_attributeDescriptions.begin(), m_attributeDescriptions.end(),
            other.m_attributeDescriptions.begin(), VertexInput::pred))
        return false;
    if (m_bindingDescription.binding!=other.m_bindingDescription.binding)
        return false;
    if (m_bindingDescription.inputRate!=other.m_bindingDescription.inputRate)
        return false;
    if (m_bindingDescription.stride!=other.m_bindingDescription.stride)
        return false;
    return true;
}

bool VertexInput::operator!=(const VertexInput &other) const
{
    return !(*this==other);
}

bool VertexInput::pred(
    const VkVertexInputAttributeDescription &a,
    const VkVertexInputAttributeDescription &b
)
{
    bool result=true;
    result &= (a.binding==b.binding);
    result &= (a.format==b.format);
    result &= (a.location==b.location);
    result &= (a.offset==b.offset);
    return result;
}

void VertexInput::setVertexAttributeVec3(uint32_t location, uint32_t offset)
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset=offset;
    setAttributeDescription(location, desc);
}

void VertexInput::setVertexAttributeVec2(uint32_t location, uint32_t offset)
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32_SFLOAT;
    desc.offset=offset;
    setAttributeDescription(location, desc);
}

void VertexInput::setAttributeDescription(
    uint32_t location,
    VkVertexInputAttributeDescription desc
)
{
    if (m_attributeDescriptions.size()<=location)
        m_attributeDescriptions.resize(location+1);
    m_attributeDescriptions[location]=desc;
}

void VertexInput::setBindingDescription(uint32_t stride)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_bindingDescription=bindingDescription;
}