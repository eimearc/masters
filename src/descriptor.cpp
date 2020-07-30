#include "descriptor.h"

Descriptor::Descriptor(Descriptor &&other) noexcept
{
    *this=std::move(other);
}

Descriptor& Descriptor::operator=(Descriptor &&other) noexcept
{
    if (*this==other) return *this;
    m_bufferInfo=std::move(other.m_bufferInfo);
    m_device=other.m_device;
    m_inputAttachmentInfo=std::move(other.m_inputAttachmentInfo);
    m_pool=other.m_pool;
    m_poolSizes=other.m_poolSizes;
    m_setLayouts=other.m_setLayouts;
    m_sets=other.m_sets;
    m_swapchainSize=other.m_swapchainSize;
    m_textureSamplerInfo=std::move(other.m_textureSamplerInfo);
    m_writeSetFragment=other.m_writeSetFragment;
    m_writeSetVertex=other.m_writeSetVertex;
    other.reset();
    return *this;
}

void Descriptor::reset()
{
    m_bufferInfo.resize(0);
    m_device=VK_NULL_HANDLE;
    m_inputAttachmentInfo.resize(0);
    m_pool=VK_NULL_HANDLE;
    m_poolSizes.resize(0);
    m_setLayouts.resize(0);
    m_sets.resize(0);
    m_swapchainSize=0;
    m_textureSamplerInfo.resize(0);
    m_writeSetFragment.resize(0);
    m_writeSetVertex.resize(0);
}

Descriptor::Descriptor(
    const Device &device,
    const size_t swapchainSize)
{
    m_device = device.device();
    m_swapchainSize = swapchainSize;
    m_writeSetVertex = std::vector<VkWriteDescriptorSet>(); // TODO: One per attachment?.
    m_writeSetFragment = std::vector<VkWriteDescriptorSet>(); // TODO: One per attachment?.

    // TODO: Tidy below.
    m_poolSizes.resize(3);
    initializePoolSize(Type::INPUT_ATTACHMENT);
    initializePoolSize(Type::TEXTURE_SAMPLER);
    initializePoolSize(Type::UNIFORM_BUFFER);
}

void Descriptor::initializePoolSize(Type type)
{
    uint32_t index = static_cast<uint32_t>(type);
    m_poolSizes[index].descriptorCount=0;
    m_poolSizes[index].type=descriptorType(type);
}

bool Descriptor::operator==(const Descriptor &other) const
{
    bool result = true;
    result &= (m_device==other.m_device);
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
    removeEmptyPoolSizes();

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

void Descriptor::removeEmptyPoolSizes()
{
    auto isEmpty = [](const VkDescriptorPoolSize &x){
        return x.descriptorCount==0;
    };
    m_poolSizes.erase(std::remove_if(
        m_poolSizes.begin(), m_poolSizes.end(), isEmpty),
        m_poolSizes.end());
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

void Descriptor::addUniformBuffer(
    const uint32_t binding,
    const Buffer &buffer,
    const Shader::Stage stage)
{
    addDescriptorSetBinding(
        Type::UNIFORM_BUFFER, binding, stage
    );
    addWriteSetBuffer(
        buffer.buffer(), buffer.size(), binding,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage
    );
}

void Descriptor::addInputAttachment(
    const uint32_t binding,
    const Attachment &attachment,
    const Shader::Stage stage)
{
    addDescriptorSetBinding(
        Type::INPUT_ATTACHMENT, binding, stage
    );
    addWriteSetInputAttachment(attachment.view(), binding, stage);
}

void Descriptor::addTextureSampler(
    const uint32_t binding,
    const Texture &texture,
    const Shader::Stage stage)
{
    addDescriptorSetBinding(
        Type::TEXTURE_SAMPLER, binding, stage
    );
    addWriteSetTextureSampler(texture, binding, stage);
}

void Descriptor::addDescriptorSetBinding(
    Type type,
    uint32_t binding,
    Shader::Stage stage)
{
    addPoolSize(type);

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType(type);
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = Shader::stageFlags(stage);
    layoutBinding.pImmutableSamplers = nullptr;
    m_setBindings.push_back(layoutBinding);   
}

VkDescriptorType Descriptor::descriptorType(Type type)
{
    switch(type)
    {
        case Type::INPUT_ATTACHMENT:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case Type::TEXTURE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case Type::UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

void Descriptor::addPoolSize(Type type)
{
    uint32_t index=static_cast<uint32_t>(type);
    VkDescriptorPoolSize &poolSize = m_poolSizes[index];
    poolSize.descriptorCount += m_swapchainSize;
}

void Descriptor::addWriteSetBuffer(
    VkBuffer buffer,
    VkDeviceSize range,
    uint32_t binding,
    VkDescriptorType type,
    Shader::Stage stage)
{
    m_bufferInfo.push_back(
        std::make_unique<VkDescriptorBufferInfo>()
    );

    auto &bufferInfo = m_bufferInfo.back();
    bufferInfo->buffer = buffer;
    bufferInfo->offset = 0;
    bufferInfo->range = range;

    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = type;
    descriptor.descriptorCount = 1;
    descriptor.pBufferInfo = bufferInfo.get();
    descriptor.pNext=nullptr;

    addWriteSet(descriptor,stage);
}

void Descriptor::addWriteSetTextureSampler(
    const Texture &texture,
    uint32_t binding,
    Shader::Stage stage)
{
    m_textureSamplerInfo.push_back(
        std::make_unique<VkDescriptorImageInfo>()
    );

    auto &imageInfo = m_textureSamplerInfo.back();
    imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo->imageView = texture.view();
    imageInfo->sampler = texture.sampler();
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor.descriptorCount = 1;
    descriptor.pImageInfo = imageInfo.get();
    descriptor.pNext=nullptr;

    addWriteSet(descriptor,stage);
}

void Descriptor::addWriteSetInputAttachment(
    VkImageView imageView,
    uint32_t binding,
    Shader::Stage stage)
{
    m_inputAttachmentInfo.push_back(
        std::make_unique<VkDescriptorImageInfo>()
    );

    auto &imageInfo = m_inputAttachmentInfo.back();
    imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo->imageView = imageView;
    imageInfo->sampler = VK_NULL_HANDLE;
    VkWriteDescriptorSet descriptor;
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstBinding = binding;
    descriptor.dstArrayElement = 0;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor.descriptorCount = 1;
    descriptor.pImageInfo = imageInfo.get();
    descriptor.pNext=nullptr;

    addWriteSet(descriptor,stage);
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
    for (auto &l: m_setLayouts)
        vkDestroyDescriptorSetLayout(m_device, l, nullptr);
    if (m_pool!=VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_device, m_pool, nullptr);
}