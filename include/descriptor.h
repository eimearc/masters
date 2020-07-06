#ifndef DESC
#define DESC

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>

#include "attachment.h"
#include "buffer.h"
#include "texture.h"

class Descriptor
{
    public:
    Descriptor()=default;
    Descriptor(
        const Device &device,
        const size_t swapchainSize,
        const size_t numAttachments
    );

    void addUniformBuffer(
        const uint32_t binding,
        const Buffer &buffer,
        const VkShaderStageFlagBits shaderStage,
        const VkDeviceSize bufferSize
    );

    void addInputAttachment(
        const uint32_t binding,
        const Attachment &attachment,
        const VkShaderStageFlagBits shaderStage
    );

    void addTextureSampler(
        const uint32_t binding,
        const Texture &texture,
        const VkShaderStageFlagBits shaderStage
    );

    void allocateDescriptorPool();
    void allocateDescriptorSets();

    void destroy();

    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;

    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
    std::vector<VkDescriptorSetLayoutBinding> m_descriptorSetBindings;
    std::vector<std::vector<VkWriteDescriptorSet>> m_writeDescriptorSet;

    std::vector<VkDescriptorBufferInfo> m_descriptorBufferInfo;
    std::vector<VkDescriptorImageInfo> m_descriptorTextureSamplerInfo;
    std::vector<VkDescriptorImageInfo> m_descriptorInputAttachmentInfo;

    size_t m_swapchainSize;
    size_t m_numAttachments;

    private:
    void addDescriptorPoolSize(const VkDescriptorType type);
    void addDescriptorSetBinding(const VkDescriptorType type, uint32_t binding, VkShaderStageFlagBits stage);
    void addWriteDescriptorSetTextureSampler(const Texture &texture, uint32_t binding);
    void addWriteDescriptorSetBuffer(
        std::vector<VkBuffer> buffers, VkDeviceSize range,
        uint32_t binding, VkDescriptorType type);
    void addWriteDescriptorSetInputAttachment(std::vector<VkImageView> imageViews, uint32_t binding);

    VkDevice m_device;
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