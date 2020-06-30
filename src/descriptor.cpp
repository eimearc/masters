#include "descriptor.h"

Descriptor::Descriptor(size_t size, size_t numAttachments)
{
    m_size = size;
    m_numAttachments = numAttachments;
    m_writeDescriptorSet = std::vector<std::vector<VkWriteDescriptorSet>>(m_size, std::vector<VkWriteDescriptorSet>());
    m_descriptorBufferInfo.resize(m_size);
    m_descriptorTextureSamplerInfo.resize(m_size);
    m_descriptorInputAttachmentInfo.resize(size*numAttachments);
}

void Descriptor::addDescriptorPoolSize(const VkDescriptorType type)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = static_cast<uint32_t>(m_size);
    m_descriptorPoolSizes.push_back(poolSize);
}

void Descriptor::addDescriptorSetBinding(const VkDescriptorType type, uint32_t binding, VkShaderStageFlagBits stage)
{
    addDescriptorPoolSize(type);

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = stage;
    layoutBinding.pImmutableSamplers = nullptr;
    m_descriptorSetBindings.push_back(layoutBinding);   
}

void Descriptor::addWriteDescriptorSetBuffer(
    std::vector<VkBuffer> buffers, VkDeviceSize offset, VkDeviceSize range,
    uint32_t binding, VkDescriptorType type, size_t startIndex)
{
    for (size_t i = 0; i < m_size; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffers[startIndex+i];
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        m_descriptorBufferInfo[i] = bufferInfo; // Overwriting?
        VkWriteDescriptorSet descriptor;
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstBinding = binding;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorType = type;
        descriptor.descriptorCount = 1;
        descriptor.pBufferInfo = &m_descriptorBufferInfo[i];
        descriptor.pNext=nullptr;
        m_writeDescriptorSet[i].push_back(descriptor);
    }
}

void Descriptor::addWriteDescriptorSetTextureSampler(VkImageView textureView, VkSampler textureSampler, uint32_t binding)
{
    for (size_t i = 0; i < m_size; ++i)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureView;
        imageInfo.sampler = textureSampler;
        m_descriptorTextureSamplerInfo[i] = imageInfo;
        VkWriteDescriptorSet descriptor;
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstBinding = binding;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor.descriptorCount = 1;
        descriptor.pImageInfo = &m_descriptorTextureSamplerInfo[i];
        descriptor.pNext=nullptr;
        m_writeDescriptorSet[i].push_back(descriptor);
    }
}

void Descriptor::addWriteDescriptorSetInputAttachment(std::vector<VkImageView> imageViews, uint32_t binding)
{
    static size_t index=0;
    for (size_t i = 0; i < m_size; ++i)
    {
        VkDescriptorImageInfo &imageInfo = m_descriptorInputAttachmentInfo[index];
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageViews[i];
        imageInfo.sampler = VK_NULL_HANDLE;
        VkWriteDescriptorSet descriptor;
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstBinding = binding;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        descriptor.descriptorCount = 1;
        descriptor.pImageInfo = &m_descriptorInputAttachmentInfo[index]; //change
        descriptor.pNext=nullptr;
        m_writeDescriptorSet[i].push_back(descriptor);

        index++;
    }
}

void VertexInput::addVertexAttributeVec3(const uint32_t &location, const uint32_t &offset)
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset=offset;
    m_attributeDescriptions.push_back(desc);
}

void VertexInput::addVertexAttributeVec2(const uint32_t &location, const uint32_t &offset)
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32_SFLOAT;
    desc.offset=offset;
    m_attributeDescriptions.push_back(desc);
}

void VertexInput::setBindingDescription(uint32_t stride)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_bindingDescription=bindingDescription;
}