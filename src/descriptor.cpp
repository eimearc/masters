#include "descriptor.h"

Descriptor::Descriptor(
    const Device &device,
    const size_t swapchainSize,
    const size_t numAttachments)
{
    m_device = device.m_device;
    m_swapchainSize = swapchainSize;
    m_numAttachments = numAttachments;
    m_writeDescriptorSet = std::vector<std::vector<VkWriteDescriptorSet>>(m_swapchainSize, std::vector<VkWriteDescriptorSet>());
    m_descriptorBufferInfo.resize(m_swapchainSize);
    m_descriptorTextureSamplerInfo.resize(m_swapchainSize);
    m_descriptorInputAttachmentInfo.resize(m_swapchainSize*m_numAttachments);
}

void Descriptor::allocateDescriptorPool()
{
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(m_descriptorPoolSizes.size());
    poolInfo.pPoolSizes = m_descriptorPoolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapchainSize);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

void Descriptor::allocateDescriptorSets()
{
    std::vector<VkDescriptorSetLayoutBinding> &bindings = m_descriptorSetBindings;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    // Create descriptor sets.
    const size_t &size = m_swapchainSize;
    std::vector<VkDescriptorSetLayout> layouts(size, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(size);
    if(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets.");
    } 

    for (size_t i = 0; i < size; i++)
    {
        for (auto &set : m_writeDescriptorSet[i]) set.dstSet=m_descriptorSets[i];

        vkUpdateDescriptorSets(m_device,
            static_cast<uint32_t>(m_writeDescriptorSet[i].size()),
            m_writeDescriptorSet[i].data(), 0, nullptr);
    }
}

void Descriptor::addDescriptorPoolSize(const VkDescriptorType type)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = static_cast<uint32_t>(m_swapchainSize);
    m_descriptorPoolSizes.push_back(poolSize);
}

void Descriptor::addUniformBuffer(
    const uint32_t binding,
    const Buffer &buffer,
    const VkShaderStageFlagBits shaderStage,
    const VkDeviceSize bufferSize)
{
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, shaderStage);
    addWriteDescriptorSetBuffer(buffer.m_buffers, bufferSize, binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

void Descriptor::addInputAttachment(
    const uint32_t binding,
    const std::vector<VkImageView> &imageViews,
    const VkShaderStageFlagBits shaderStage)
{
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, binding, shaderStage);
    addWriteDescriptorSetInputAttachment(imageViews, binding);
}

void Descriptor::addTextureSampler(
    const uint32_t binding,
    const Texture &texture,
    const VkShaderStageFlagBits shaderStage)
{
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding, shaderStage);
    addWriteDescriptorSetTextureSampler(texture, binding);
}

void Descriptor::addDescriptorSetBinding(
    const VkDescriptorType type,
    uint32_t binding,
    VkShaderStageFlagBits stage)
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
    std::vector<VkBuffer> buffers, VkDeviceSize range,
    uint32_t binding, VkDescriptorType type)
{
    for (size_t i = 0; i < m_swapchainSize; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffers[i];
        bufferInfo.offset = 0;
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

void Descriptor::addWriteDescriptorSetTextureSampler(const Texture &texture, uint32_t binding)
{
    for (size_t i = 0; i < m_swapchainSize; ++i)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.m_imageView;
        imageInfo.sampler = texture.m_imageSampler;
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
    for (size_t i = 0; i < m_swapchainSize; ++i)
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

void Descriptor::destroy()
{
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}