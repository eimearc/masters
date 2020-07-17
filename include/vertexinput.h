#ifndef EVK_VERTEX_INPUT_H_
#define EVK_VERTEX_INPUT_H_

#include <vector>
#include <vulkan/vulkan.h>

class VertexInput
{
    public:
    VertexInput()=default;
    VertexInput(uint32_t stride);

    void addVertexAttributeVec3(uint32_t location, uint32_t offset);
    void addVertexAttributeVec2(uint32_t location, uint32_t offset);

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions() const { return m_attributeDescriptions; };
    VkVertexInputBindingDescription bindingDescription() const {return m_bindingDescription; };    

    private:
    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    VkVertexInputBindingDescription m_bindingDescription;

    void setBindingDescription(uint32_t stride);
};

#endif