#ifndef DESC
#define DESC

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>

class Descriptor
{
    public:
    Descriptor()=default;
    Descriptor(size_t size, size_t numAttachments=1);

    void addUniformBuffer(
        const uint32_t binding,
        const std::vector<VkBuffer> &uniformBuffers,
        const VkShaderStageFlagBits shaderStage,
        const VkDeviceSize bufferSize
    );

    void addInputAttachment(
        const uint32_t binding,
        const std::vector<VkImageView> &imageViews,
        const VkShaderStageFlagBits shaderStage
    );

    void addTextureSampler(
        const uint32_t binding,
        const VkImageView &textureImageView,
        const VkSampler &textureSampler,
        const VkShaderStageFlagBits shaderStage
    );

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;

    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
    std::vector<VkDescriptorSetLayoutBinding> m_descriptorSetBindings;
    std::vector<std::vector<VkWriteDescriptorSet>> m_writeDescriptorSet;

    std::vector<VkDescriptorBufferInfo> m_descriptorBufferInfo;
    std::vector<VkDescriptorImageInfo> m_descriptorTextureSamplerInfo;
    std::vector<VkDescriptorImageInfo> m_descriptorInputAttachmentInfo;

    size_t m_size;
    size_t m_numAttachments;

    private:
    void addDescriptorPoolSize(const VkDescriptorType type);
    void addDescriptorSetBinding(const VkDescriptorType type, uint32_t binding, VkShaderStageFlagBits stage);
    void addWriteDescriptorSetTextureSampler(VkImageView textureView, VkSampler textureSampler, uint32_t binding);
    void addWriteDescriptorSetBuffer(
    std::vector<VkBuffer> buffers, VkDeviceSize offset, VkDeviceSize range,
    uint32_t binding, VkDescriptorType type, size_t startIndex);
    void addWriteDescriptorSetInputAttachment(std::vector<VkImageView> imageViews, uint32_t binding);
};

class VertexInput
{
    public:
    VertexInput()=default;
    
    void addVertexAttributeVec3(const uint32_t &location, const uint32_t &offset);
    void addVertexAttributeVec2(const uint32_t &location, const uint32_t &offset);
    void setBindingDescription(uint32_t stride);

    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    VkVertexInputBindingDescription m_bindingDescription;
};

#endif