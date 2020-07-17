#include "descriptor.h"

Descriptor::Descriptor(
    const Device &device,
    const size_t swapchainSize,
    const size_t numAttachments)
{
    m_device = device.device();
    m_swapchainSize = swapchainSize;
    m_numAttachments = numAttachments;
    m_writeDescriptorSetVertex = std::vector<VkWriteDescriptorSet>(); // One per attachment?.
    m_writeDescriptorSetFragment = std::vector<VkWriteDescriptorSet>(); // One per attachment?.
    m_descriptorInputAttachmentInfo.resize(m_numAttachments);
}

void Descriptor::allocateDescriptorPool()
{
    m_descriptorPoolSizes.resize(0);
    if (numInputAttachments>0) addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, m_swapchainSize*numInputAttachments*2);
    if (numUniformBuffers>0) addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_swapchainSize*numUniformBuffers*2);
    if (numImageSamplers>0) addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_swapchainSize*numImageSamplers*2);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(m_descriptorPoolSizes.size());
    poolInfo.pPoolSizes = m_descriptorPoolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapchainSize*2);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

void Descriptor::allocateDescriptorSets()
{
    m_descriptorSetLayouts.resize(2);

    std::vector<VkDescriptorSetLayoutBinding> vertexBindings;
    std::vector<VkDescriptorSetLayoutBinding> fragmentBindings;

    for (const auto &b : m_descriptorSetBindings)
    {
        if (b.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT) fragmentBindings.push_back(b);
        else vertexBindings.push_back(b);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(vertexBindings.size());
    layoutInfo.pBindings = vertexBindings.data();
    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayouts[0]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    layoutInfo.bindingCount = static_cast<uint32_t>(fragmentBindings.size());
    layoutInfo.pBindings = fragmentBindings.data();
    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayouts[1]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    // Create descriptor sets.
    m_descriptorSets.resize(2);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = m_descriptorSetLayouts.size();
    allocInfo.pSetLayouts = m_descriptorSetLayouts.data();
    if(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets.");
    } 

    for (auto &ds : m_writeDescriptorSetVertex) ds.dstSet=m_descriptorSets[0];
    for (auto &ds : m_writeDescriptorSetFragment) ds.dstSet=m_descriptorSets[1];

    std::vector<VkWriteDescriptorSet> writes;
    for (auto &ds : m_writeDescriptorSetVertex) writes.push_back(ds);
    for (auto &ds : m_writeDescriptorSetFragment) writes.push_back(ds);

    vkUpdateDescriptorSets(m_device,
        static_cast<uint32_t>(writes.size()),
        writes.data(), 0, nullptr);
}

void Descriptor::addDescriptorPoolSize(const VkDescriptorType type, const size_t count)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = static_cast<uint32_t>(count);
    m_descriptorPoolSizes.push_back(poolSize);
}

void Descriptor::addUniformBuffer(
    const uint32_t binding,
    const Buffer &buffer,
    const ShaderStage stage,
    const VkDeviceSize bufferSize)
{
    auto shaderStage = shaderStageFlags(stage);
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, shaderStage);
    addWriteDescriptorSetBuffer(buffer.m_buffer, bufferSize, binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderStage);
}

void Descriptor::addInputAttachment(
    const uint32_t binding,
    const Attachment &attachment,
    const ShaderStage stage)
{
    auto shaderStage = shaderStageFlags(stage);
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, binding, shaderStage);
    addWriteDescriptorSetInputAttachment(attachment.m_imageView, binding, shaderStage);
}

void Descriptor::addTextureSampler(
    const uint32_t binding,
    const Texture &texture,
    const ShaderStage stage)
{
    auto shaderStage = shaderStageFlags(stage);
    addDescriptorSetBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding, shaderStage);
    addWriteDescriptorSetTextureSampler(texture, binding, shaderStage);
}

void Descriptor::addDescriptorSetBinding(
    const VkDescriptorType type,
    uint32_t binding,
    VkShaderStageFlagBits stage)
{
    switch (type)
    {
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        numInputAttachments++;
        break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        numUniformBuffers++;
        break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        numImageSamplers++;
        break;
    default:
        break;
    }

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = stage;
    layoutBinding.pImmutableSamplers = nullptr;
    m_descriptorSetBindings.push_back(layoutBinding);   
}

void Descriptor::addWriteDescriptorSetBuffer(
    VkBuffer buffer, VkDeviceSize range,
    uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = range;
    m_descriptorBufferInfo = bufferInfo; // Overwriting?
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = type;
    descriptor.descriptorCount = 1;
    descriptor.pBufferInfo = &m_descriptorBufferInfo;
    descriptor.pNext=nullptr;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) m_writeDescriptorSetFragment.push_back(descriptor);
    else m_writeDescriptorSetVertex.push_back(descriptor);
}

void Descriptor::addWriteDescriptorSetTextureSampler(const Texture &texture, uint32_t binding, VkShaderStageFlagBits stage)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.m_imageView;
    imageInfo.sampler = texture.m_imageSampler;
    m_descriptorTextureSamplerInfo = imageInfo;
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor.descriptorCount = 1;
    descriptor.pImageInfo = &m_descriptorTextureSamplerInfo;
    descriptor.pNext=nullptr;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) m_writeDescriptorSetFragment.push_back(descriptor);
    else m_writeDescriptorSetVertex.push_back(descriptor);
}

void Descriptor::addWriteDescriptorSetInputAttachment(VkImageView imageView, uint32_t binding, VkShaderStageFlagBits stage)
{
    static size_t index=0;

    VkDescriptorImageInfo &imageInfo = m_descriptorInputAttachmentInfo[index];
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = VK_NULL_HANDLE;
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor.descriptorCount = 1;
    descriptor.pImageInfo = &m_descriptorInputAttachmentInfo[index]; //change
    descriptor.pNext=nullptr;
    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) m_writeDescriptorSetFragment.push_back(descriptor);
    else m_writeDescriptorSetVertex.push_back(descriptor);

    index++;
}

VertexInput::VertexInput(uint32_t stride)
{
    setBindingDescription(stride);
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

Descriptor::~Descriptor() noexcept
{
    for (auto &l: m_descriptorSetLayouts) vkDestroyDescriptorSetLayout(m_device, l, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}