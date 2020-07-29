#ifndef EVK_DESCRIPTOR_H_
#define EVK_DESCRIPTOR_H_

#include "attachment.h"
#include "buffer.h"
#include <iostream>
#include "shader.h"
#include <vector>
#include <vulkan/vulkan.h>
#include "texture.h"

class Descriptor
{
    public:
    Descriptor()=default;
    Descriptor(const Descriptor&)=delete;
    Descriptor& operator=(const Descriptor&)=delete;
    Descriptor(Descriptor&&) noexcept;
    Descriptor& operator=(Descriptor&&) noexcept;
    ~Descriptor() noexcept;

    Descriptor(
        const Device &device,
        const size_t swapchainSize,
        const size_t numAttachments
    );

    bool operator==(const Descriptor&) const;

    void addInputAttachment(
        const uint32_t binding,
        const Attachment &attachment,
        const Shader::Stage shaderStage
    );
    void addTextureSampler(
        const uint32_t binding,
        const Texture &texture,
        const Shader::Stage shaderStage
    );
    void addUniformBuffer(
        const uint32_t binding,
        const Buffer &buffer,
        const Shader::Stage shaderStage,
        const VkDeviceSize bufferSize
    );
    
    private:
    enum class Type{
        INPUT_ATTACHMENT,
        TEXTURE_SAMPLER,
        UNIFORM_BUFFER  
    };

    void addDescriptorSetBinding(
        Type type,
        uint32_t binding,
        Shader::Stage stage
    );
    void addPoolSize(Type type);
    void addWriteSetTextureSampler(
        const Texture &texture,
        uint32_t binding,
        Shader::Stage stage
    );
    void addWriteSetBuffer(
        VkBuffer buffer,
        VkDeviceSize range,
        uint32_t binding,
        VkDescriptorType type,
        Shader::Stage stage
    );
    void addWriteSetInputAttachment(
        VkImageView imageView,
        uint32_t binding,
        Shader::Stage stage
    );
    void addWriteSet(
        VkWriteDescriptorSet writeSet,
        Shader::Stage stage
    );
    void allocateDescriptorPool();
    void allocateDescriptorSets();
    VkDescriptorType descriptorType(Type type);
    void finalize();
    void initializePoolSize(Type type);
    void removeEmptyPoolSizes();

    std::vector<VkDescriptorSetLayout> setLayouts() const { return m_setLayouts; };
    std::vector<VkDescriptorSet> sets() const { return m_sets; };

    VkDescriptorBufferInfo m_bufferInfo;
    VkDevice m_device;
    std::vector<VkDescriptorImageInfo> m_inputAttachmentInfo;
    size_t m_numAttachments;
    size_t m_numInputAttachments=0;
    size_t m_numImageSamplers=0;
    size_t m_numUniformBuffers=0;
    VkDescriptorPool m_pool=VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> m_poolSizes;
    std::vector<VkDescriptorSetLayout> m_setLayouts;
    std::vector<VkDescriptorSetLayoutBinding> m_setBindings;
    std::vector<VkDescriptorSet> m_sets;
    size_t m_swapchainSize;
    VkDescriptorImageInfo m_textureSamplerInfo;
    std::vector<VkWriteDescriptorSet> m_writeSetFragment;
    std::vector<VkWriteDescriptorSet> m_writeSetVertex;

    friend class Pipeline;
    friend class Device;
};

#endif