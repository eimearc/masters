#ifndef EVK_VERTEX_INPUT_H_
#define EVK_VERTEX_INPUT_H_

#include "util.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace evk {

/**
 * @class VertexInput
 * @brief VertexInput describes vertex input information.
 * 
 * VertexInput is then bound to a Pipeline and can be accessed in the vertex
 * shader.
 * 
 * @example
 * VertexInput vertexInput(sizeof(Vertex));
 * vertexInput.setVertexAttributeVec3(0,offsetof(Vertex,pos));
 * vertexInput.setVertexAttributeVec3(1,offsetof(Vertex,color));
 * vertexInput.setVertexAttributeVec2(2,offsetof(Vertex,texCoord));
 * 
 * ...
 * 
 * Pipeline pipeline(
 *  device, &subpass, &descriptor, vertexInput, &renderpass, shaders
 * );
 * 
 * // Vertex shader
 * layout(location = 0) in vec3 inPosition;
 * layout(location = 1) in vec3 inColor;
 * layout(location = 2) in vec2 inTexCoord;
 **/
class VertexInput
{
    public:
    VertexInput()=default;
    
    /**
     * Constructs VertexInput with a specified stride.
     * @param[in] stride the stride of each vertex element.
     **/
    VertexInput(uint32_t stride) noexcept;

    bool operator==(const VertexInput&) const noexcept;
    bool operator!=(const VertexInput&) const noexcept;

    /**
     * Sets the Vertex attribute at a specified location for a 2-part vector.
     * @param[in] location the location of the attribute.
     * @param[in] offset the offset within the Vertex structure.
     **/
    void setVertexAttributeVec2(uint32_t location, uint32_t offset) noexcept;

    /**
     * Sets the Vertex attribute at a specified location for a 3-part vector.
     * @param[in] location the location of the attribute.
     * @param[in] offset the offset within the Vertex structure.
     **/
    void setVertexAttributeVec3(uint32_t location, uint32_t offset) noexcept;

    private:
    using AttributeDescriptions =
        std::vector<VkVertexInputAttributeDescription>;
        
    static bool pred(
        const VkVertexInputAttributeDescription &a,
        const VkVertexInputAttributeDescription &b
    ) noexcept;

    AttributeDescriptions attributeDescriptions() const noexcept
    { 
        return m_attributeDescriptions;
    };
    VkVertexInputBindingDescription bindingDescription() const noexcept
    {
        return m_bindingDescription;
    };

    void setAttributeDescription(
        uint32_t location,
        VkVertexInputAttributeDescription desc
    ) noexcept;
    void setBindingDescription(uint32_t stride) noexcept;  
    
    AttributeDescriptions m_attributeDescriptions;
    VkVertexInputBindingDescription m_bindingDescription;

    friend class Pipeline;

    // Tests.
    FRIEND_TEST(VertexInputTest,ctor);
};

} // namespace evk

#endif