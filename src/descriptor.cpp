#include "evulkan_core.h"

void evk::Instance::addDescriptorPoolSize(const VkDescriptorType type)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
    m_descriptorPoolSizes.push_back(poolSize);
}

void evk::Instance::addDescriptorSetBinding(const VkDescriptorType type, uint32_t binding, VkShaderStageFlagBits stage)
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = stage;
    layoutBinding.pImmutableSamplers = nullptr;
    m_descriptorSetBindings.push_back(layoutBinding);   
}

void evk::Instance::addWriteDescriptorSetBuffer(
    std::vector<VkBuffer> buffers, VkDeviceSize offset, VkDeviceSize range,
    uint32_t binding, VkDescriptorType type, size_t startIndex)
{
    if (m_writeDescriptorSet.size()==0)
        m_writeDescriptorSet = std::vector<std::vector<VkWriteDescriptorSet>>(m_swapChainImages.size(), std::vector<VkWriteDescriptorSet>());
    m_descriptorBufferInfo.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffers[startIndex+i];
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        m_descriptorBufferInfo[i] = bufferInfo;
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

void evk::Instance::addWriteDescriptorSetTextureSampler(VkImageView textureView, VkSampler textureSampler, uint32_t binding)
{
    if (m_writeDescriptorSet.size()==0)
        m_writeDescriptorSet = std::vector<std::vector<VkWriteDescriptorSet>>(m_swapChainImages.size(), std::vector<VkWriteDescriptorSet>());
    m_descriptorTextureSamplerInfo.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_swapChainImages.size(); ++i)
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

void evk::Instance::createDescriptorSets()
{
    for (auto &p : m_evkpipelines)
    {
        auto &desc = p.m_descriptor;
        std::vector<VkDescriptorPoolSize> &poolSizes=desc.m_descriptorPoolSizes;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(desc.m_size);

        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &desc.m_descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool.");
        }

        std::vector<VkDescriptorSetLayoutBinding> &bindings = desc.m_descriptorSetBindings;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &desc.m_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout.");
        }

        // Create descriptor sets.
        const size_t &size = desc.m_size;
        std::vector<VkDescriptorSetLayout> layouts(size, desc.m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = desc.m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
        allocInfo.pSetLayouts = layouts.data();

        desc.m_descriptorSets.resize(size);
        if(vkAllocateDescriptorSets(m_device, &allocInfo, desc.m_descriptorSets.data())!=VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets.");
        } 

        for (size_t i = 0; i < size; i++)
        {
            for (auto &set : desc.m_writeDescriptorSet[i]) set.dstSet=desc.m_descriptorSets[i];

            std::cout << desc.m_writeDescriptorSet[i][0].pBufferInfo[0].buffer << std::endl;

            vkUpdateDescriptorSets(m_device,
                static_cast<uint32_t>(desc.m_writeDescriptorSet[i].size()),
                desc.m_writeDescriptorSet[i].data(), 0, nullptr);
        }
    }
}