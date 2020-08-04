#ifndef EVK_VERTEX_INPUT_H_
#define EVK_VERTEX_INPUT_H_

#include <vector>
#include <vulkan/vulkan.h>

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
    VertexInput(uint32_t stride);

    bool operator==(const VertexInput&) const;
    bool operator!=(const VertexInput&) const;

    /**
     * Sets the Vertex attribute at a specified location for a 2-part vector.
     * @param[in] location the location of the attribute.
     * @param[in] offset the offset within the Vertex structure.
     **/
    void setVertexAttributeVec2(uint32_t location, uint32_t offset);

    /**
     * Sets the Vertex attribute at a specified location for a 3-part vector.
     * @param[in] location the location of the attribute.
     * @param[in] offset the offset within the Vertex structure.
     **/
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