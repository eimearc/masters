#ifndef EVK_DESCRIPTOR_H_
#define EVK_DESCRIPTOR_H_

#include "attachment.h"
#include "buffer.h"
#include <iostream>
#include "shader.h"
#include <vector>
#include <vulkan/vulkan.h>
#include "texture.h"

namespace evk {

/**
 * @class Descriptor
 * @brief A Descriptor describes a resource that will be accessed in a Shader.
 * 
 * A Descriptor is used to describe a resource that will be used in either the 
 * vertex or fragment shader. Such resources include an InputAttachment, a 
 * TextureSampler and a UniformBuffer. Each of these has an associated
 * binding, which represents the order in which they are accessed and
 * bound to the Shader. Each one also has a specified Stage which
 * represents the Shader::Stage at which the resource will be
 * bound and accessed.
 * 
 * InputAttachment: an Attachment as an input to the Shader.
 * TextureSampler: used to sample a Texture object bound to the Shader.
 * UniformBuffer: a Uniform Buffer object bound to the Shader.
 * 
 * The Descriptor is then bound to a Pipeline.
 * 
 * @example
 * Descriptor descriptor(device, swapchainSize);
 * descriptor.addTextureSampler(1, texture, Shader::Stage::FRAGMENT);
 * descriptor.addUniformBuffer(0, ubo, Shader::Stage::VERTEX);
 * descriptor.addInputAttachment(0, colorAttachment, Shader::Stage::FRAGMENT);
 * 
 * ...
 * 
 * Pipeline pipeline(
 *  device, &subpass, &descriptor, vertexInput, &renderpass, shaders
 * );
 **/ 
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
        const size_t swapchainSize
    );

    bool operator==(const Descriptor&) const;

    /**
     * Adds an input attachment binding to the descriptor.
     * @param[in] binding where the Attachment will be bound.
     * @param[in] attachment the Attachment to bind.
     * @param[in] shaderStage the stage to bind the Attachment to.
     **/
    void addInputAttachment(
        const uint32_t binding,
        const Attachment &attachment,
        const Shader::Stage shaderStage
    );

    /**
     * Adds a texture sampler binding to the descriptor.
     * @param[in] binding where the Texture will be bound.
     * @param[in] texture the Texture to bind.
     * @param[in] shaderStage the stage to bind the Texture to.
     **/
    void addTextureSampler(
        const uint32_t binding,
        const Texture &texture,
        const Shader::Stage shaderStage
    );

    /**
     * Adds a uniform buffer object (UBO) binding to the descriptor.
     * @param[in] binding where the UBO will be bound.
     * @param[in] buffer the UBO to bind.
     * @param[in] shaderStage the stage to bind the UBO to.
     **/
    void addUniformBuffer(
        const uint32_t binding,
        const Buffer &buffer,
        const Shader::Stage shaderStage
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
    void reset();

    std::vector<VkDescriptorSetLayout> setLayouts() const { return m_setLayouts; };
    std::vector<VkDescriptorSet> sets() const { return m_sets; };

    std::vector<std::unique_ptr<VkDescriptorBufferInfo>> m_bufferInfo;
    VkDevice m_device=VK_NULL_HANDLE;
    std::vector<std::unique_ptr<VkDescriptorImageInfo>> m_inputAttachmentInfo;
    VkDescriptorPool m_pool=VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> m_poolSizes;
    std::vector<VkDescriptorSetLayout> m_setLayouts;
    std::vector<VkDescriptorSetLayoutBinding> m_setBindings;
    std::vector<VkDescriptorSet> m_sets;
    size_t m_swapchainSize=0;
    std::vector<std::unique_ptr<VkDescriptorImageInfo>> m_textureSamplerInfo;
    std::vector<VkWriteDescriptorSet> m_writeSetFragment;
    std::vector<VkWriteDescriptorSet> m_writeSetVertex;

    friend class Pipeline;
    friend class Device;

    // Testing.
    friend class DescriptorTest_ctor_Test;
    friend class DescriptorTest_multipleUniformBuffers_Test;
};

} // namespace evk

#endif