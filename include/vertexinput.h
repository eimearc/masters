#ifndef EVK_VERTEX_INPUT_H_
#define EVK_VERTEX_INPUT_H_

#include <vector>
#include <vulkan/vulkan.h>

class VertexInput
{
    public:
    VertexInput()=default;
    VertexInput(uint32_t stride);

    bool operator==(const VertexInput&) const;
    bool operator!=(const VertexInput&) const;

    void setVertexAttributeVec2(uint32_t location, uint32_t offset);
    void setVertexAttributeVec3(uint32_t location, uint32_t offset);

    private:
    static bool pred(
        const VkVertexInputAttributeDescription &a,
        const VkVertexInputAttributeDescription &b
    );
    void setAttributeDescription(
        uint32_t location,
        VkVertexInputAttributeDescription desc
    );
    void setBindingDescription(uint32_t stride);

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions() const { return m_attributeDescriptions; };
    VkVertexInputBindingDescription bindingDescription() const {return m_bindingDescription; };    
    
    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    VkVertexInputBindingDescription m_bindingDescription;

    friend class Pipeline;

    // Tests.
    friend class VertexInputTest_ctor_Test;
};

#endif