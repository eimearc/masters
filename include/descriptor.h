#ifndef EVK_DESCRIPTOR
#define EVK_DESCRIPTOR

#include "attachment.h"
#include "buffer.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include "texture.h"

class Descriptor
{
    public:
    Descriptor()=default;
    Descriptor(const Descriptor&)=delete;
    Descriptor& operator=(const Descriptor&)=delete;
    Descriptor(Descriptor&&)=delete;
    Descriptor& operator=(Descriptor&&)=delete;
    ~Descriptor() noexcept;

    Descriptor(
        const Device &device,
        const size_t swapchainSize,
        const size_t numAttachments
    );

    void addUniformBuffer(
        const uint32_t binding,
        const Buffer &buffer,
        const ShaderStage shaderStage,
        const VkDeviceSize bufferSize
    );

    void addInputAttachment(
        const uint32_t binding,
        const Attachment &attachment,
        const ShaderStage shaderStage
    );

    void addTextureSampler(
        const uint32_t binding,
        const Texture &texture,
        const ShaderStage shaderStage
    );

    void allocateDescriptorPool();
    void allocateDescriptorSets();

    std::vector<VkDescriptorSetLayout> setLayouts() const { return m_descriptorSetLayouts; };
    std::vector<VkDescriptorSet> sets() const { return m_descriptorSets; };

    private:
    VkDevice m_device;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkWriteDescriptorSet> m_writeDescriptorSetVertex;
    std::vector<VkWriteDescriptorSet> m_writeDescriptorSetFragment;
    std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
    std::vector<VkDescriptorSetLayoutBinding> m_descriptorSetBindings;
    VkDescriptorBufferInfo m_descriptorBufferInfo;
    VkDescriptorImageInfo m_descriptorTextureSamplerInfo;
    std::vector<VkDescriptorImageInfo> m_descriptorInputAttachmentInfo;
    size_t m_swapchainSize;
    size_t m_numAttachments;
    size_t numUniformBuffers=0;
    size_t numInputAttachments=0;
    size_t numImageSamplers=0;

    void addDescriptorPoolSize(const VkDescriptorType type, const size_t count);
    void addDescriptorSetBinding(const VkDescriptorType type, uint32_t binding, VkShaderStageFlagBits stage);
    
    void addWriteDescriptorSetTextureSampler(const Texture &texture, uint32_t binding, VkShaderStageFlagBits stage);
    void addWriteDescriptorSetBuffer(
        VkBuffer buffer, VkDeviceSize range,
        uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage);
    void addWriteDescriptorSetInputAttachment(VkImageView imageView, uint32_t binding, VkShaderStageFlagBits stage);
};

#endif