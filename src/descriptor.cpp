#include "evulkan_core.h"

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

            vkUpdateDescriptorSets(m_device,
                static_cast<uint32_t>(desc.m_writeDescriptorSet[i].size()),
                desc.m_writeDescriptorSet[i].data(), 0, nullptr);
        }
    }
}