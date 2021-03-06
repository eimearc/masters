#include "vertexinput.h"

namespace evk {

VertexInput::VertexInput(uint32_t stride) noexcept
{
    setBindingDescription(stride);
}

bool VertexInput::operator==(const VertexInput &other) const noexcept
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

bool VertexInput::operator!=(const VertexInput &other) const noexcept
{
    return !(*this==other);
}

bool VertexInput::pred(
    const VkVertexInputAttributeDescription &a,
    const VkVertexInputAttributeDescription &b
) noexcept
{
    if (a.binding!=b.binding) return false;
    if (a.format!=b.format) return false;
    if (a.location!=b.location) return false;
    if (a.offset!=b.offset) return false;
    return true;
}

void VertexInput::setVertexAttributeVec3(
    uint32_t location,
    uint32_t offset
) noexcept
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset=offset;
    setAttributeDescription(location, desc);
}

void VertexInput::setVertexAttributeVec2(
    uint32_t location,
    uint32_t offset
) noexcept
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
) noexcept
{
    if (m_attributeDescriptions.size()<=location)
        m_attributeDescriptions.resize(location+1);
    m_attributeDescriptions[location]=desc;
}

void VertexInput::setBindingDescription(uint32_t stride) noexcept
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_bindingDescription=bindingDescription;
}

} // namespace evk