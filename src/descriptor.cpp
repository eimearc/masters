#include "descriptor.h"

Descriptor::Descriptor(Descriptor &&other) noexcept
{
    *this=std::move(other);
}

Descriptor& Descriptor::operator=(Descriptor &&other) noexcept
{
    if (*this==other) return *this;
    m_bufferInfo=other.m_bufferInfo;
    other.m_bufferInfo={};
    m_device=other.m_device;
    other.m_device=VK_NULL_HANDLE;
    m_inputAttachmentInfo=other.m_inputAttachmentInfo;
    other.m_inputAttachmentInfo={};
    m_numAttachments=other.m_numAttachments;
    other.m_numAttachments=0;
    m_numInputAttachments=other.m_numInputAttachments;
    other.m_numInputAttachments=0;
    m_numImageSamplers=other.m_numImageSamplers;
    other.m_numImageSamplers=0;
    m_numUniformBuffers=other.m_numUniformBuffers;
    other.m_numUniformBuffers=0;
    m_pool=other.m_pool;
    other.m_pool=VK_NULL_HANDLE;
    m_poolSizes=other.m_poolSizes;
    other.m_poolSizes.resize(0);
    m_setLayouts=other.m_setLayouts;
    other.m_setLayouts.resize(0);
    m_sets=other.m_sets;
    other.m_sets.resize(0);
    m_swapchainSize=other.m_swapchainSize;
    other.m_swapchainSize=0;
    m_textureSamplerInfo=other.m_textureSamplerInfo;
    other.m_textureSamplerInfo={};
    m_writeSetFragment=other.m_writeSetFragment;
    other.m_writeSetFragment.resize(0);
    m_writeSetVertex=other.m_writeSetVertex;
    other.m_writeSetVertex.resize(0);
    return *this;
}

Descriptor::Descriptor(
    const Device &device,
    const size_t swapchainSize,
    const size_t numAttachments)
{
    assert(numAttachments>0); // TODO: remove?

    m_device = device.device();
    m_swapchainSize = swapchainSize;
    m_numAttachments = numAttachments;
    m_writeSetVertex = std::vector<VkWriteDescriptorSet>(); // TODO: One per attachment?.
    m_writeSetFragment = std::vector<VkWriteDescriptorSet>(); // TODO: One per attachment?.
    m_inputAttachmentInfo.resize(m_numAttachments);
}

bool Descriptor::operator==(const Descriptor &other) const
{
    bool result = true;
    result &= (m_device==other.m_device);
    result &= (m_numAttachments==other.m_numAttachments);
    result &= (m_numInputAttachments==other.m_numInputAttachments);
    result &= (m_numImageSamplers==other.m_numImageSamplers);
    result &= (m_numUniformBuffers==other.m_numUniformBuffers);
    result &= (m_pool==other.m_pool);
    result &= std::equal(
        m_setLayouts.begin(), m_setLayouts.end(),
        other.m_setLayouts.begin()  
    );
    result &= std::equal(
        m_sets.begin(), m_sets.end(),
        other.m_sets.begin()  
    );
    return result;
}

void Descriptor::finalize()
{
    allocateDescriptorPool();
    allocateDescriptorSets();
}

void Descriptor::allocateDescriptorPool()
{
    if (m_numInputAttachments>0)
        addPoolSize(
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            m_swapchainSize*m_numInputAttachments*2);
    if (m_numUniformBuffers>0)
        addPoolSize(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            m_swapchainSize*m_numUniformBuffers*2);
    if (m_numImageSamplers>0)
        addPoolSize(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            m_swapchainSize*m_numImageSamplers*2);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
    poolInfo.pPoolSizes = m_poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapchainSize*2);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

// TODO: Handle case where no descriptors have been added.
void Descriptor::allocateDescriptorSets()
{
    m_setLayouts.resize(2);

    std::vector<VkDescriptorSetLayoutBinding> vertexBindings;
    std::vector<VkDescriptorSetLayoutBinding> fragmentBindings;

    for (const auto &b : m_setBindings)
    {
        if (b.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT) fragmentBindings.push_back(b); // TODO: Change.
        else vertexBindings.push_back(b);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(vertexBindings.size());
    layoutInfo.pBindings = vertexBindings.data();
    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_setLayouts[0]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    layoutInfo.bindingCount = static_cast<uint32_t>(fragmentBindings.size());
    layoutInfo.pBindings = fragmentBindings.data();
    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_setLayouts[1]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    // Create descriptor sets.
    m_sets.resize(2);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = m_setLayouts.size();
    allocInfo.pSetLayouts = m_setLayouts.data();
    if(vkAllocateDescriptorSets(m_device, &allocInfo, m_sets.data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets.");
    } 

    for (auto &ds : m_writeSetVertex) ds.dstSet=m_sets[0];
    for (auto &ds : m_writeSetFragment) ds.dstSet=m_sets[1];

    std::vector<VkWriteDescriptorSet> writes;
    for (auto &ds : m_writeSetVertex) writes.push_back(ds);
    for (auto &ds : m_writeSetFragment) writes.push_back(ds);

    vkUpdateDescriptorSets(m_device,
        static_cast<uint32_t>(writes.size()),
        writes.data(), 0, nullptr);
}

void Descriptor::addPoolSize(
    const VkDescriptorType type,
    const size_t count)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = static_cast<uint32_t>(count);
    m_poolSizes.push_back(poolSize);
}

void Descriptor::addUniformBuffer(
    const uint32_t binding,
    const Buffer &buffer,
    const Shader::Stage stage,
    const VkDeviceSize bufferSize)
{
    addDescriptorSetBinding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, stage
    );
    addWriteSetBuffer(buffer.buffer(), bufferSize, binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage);
}

void Descriptor::addInputAttachment(
    const uint32_t binding,
    const Attachment &attachment,
    const Shader::Stage stage)
{
    addDescriptorSetBinding(
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, binding, stage
    );
    addWriteSetInputAttachment(attachment.view(), binding, stage);
}

void Descriptor::addTextureSampler(
    const uint32_t binding,
    const Texture &texture,
    const Shader::Stage stage)
{
    addDescriptorSetBinding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding, stage
    );
    addWriteSetTextureSampler(texture, binding, stage);
}

void Descriptor::addDescriptorSetBinding(
    const VkDescriptorType type,
    uint32_t binding,
    Shader::Stage stage)
{
    switch (type)
    {
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        m_numInputAttachments++;
        break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        m_numUniformBuffers++;
        break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        m_numImageSamplers++;
        break;
    default:
        break;
    }

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = Shader::stageFlags(stage);
    layoutBinding.pImmutableSamplers = nullptr;
    m_setBindings.push_back(layoutBinding);   
}

void Descriptor::addWriteSetBuffer(
    VkBuffer buffer,
    VkDeviceSize range,
    uint32_t binding,
    VkDescriptorType type,
    Shader::Stage stage)
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = range;
    m_bufferInfo = bufferInfo; // TODO: Overwriting?
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = type;
    descriptor.descriptorCount = 1;
    descriptor.pBufferInfo = &m_bufferInfo;
    descriptor.pNext=nullptr;

    addWriteSet(descriptor,stage);
}

void Descriptor::addWriteSetTextureSampler(
    const Texture &texture,
    uint32_t binding,
    Shader::Stage stage)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.view();
    imageInfo.sampler = texture.sampler();
    m_textureSamplerInfo = imageInfo;
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor.descriptorCount = 1;
    descriptor.pImageInfo = &m_textureSamplerInfo;
    descriptor.pNext=nullptr;

    addWriteSet(descriptor,stage);
}

void Descriptor::addWriteSetInputAttachment(
    VkImageView imageView,
    uint32_t binding,
    Shader::Stage stage)
{
    static size_t index=0;

    VkDescriptorImageInfo &imageInfo = m_inputAttachmentInfo[index];
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = VK_NULL_HANDLE;
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor.descriptorCount = 1;
    descriptor.pImageInfo = &m_inputAttachmentInfo[index]; // TODO: change
    descriptor.pNext=nullptr;

    addWriteSet(descriptor,stage);

    index++;
}

void Descriptor::addWriteSet(
    VkWriteDescriptorSet writeSet,
    Shader::Stage stage)
{
    switch(stage)
    {
        case Shader::Stage::FRAGMENT:
            m_writeSetFragment.push_back(writeSet);
            break;
        case Shader::Stage::VERTEX:
            m_writeSetVertex.push_back(writeSet);
            break;
        // TODO: Handle support for geometry shader.
    }
}

Descriptor::~Descriptor() noexcept
{
    for (auto &l: m_setLayouts) vkDestroyDescriptorSetLayout(m_device, l, nullptr);
    vkDestroyDescriptorPool(m_device, m_pool, nullptr);
}