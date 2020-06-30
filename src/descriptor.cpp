#include "evulkan_core.h"

void evk::Instance::createDescriptorSets(std::vector<Descriptor*> descriptors)
{
    for (Descriptor *desc : descriptors)
    {
        std::vector<VkDescriptorPoolSize> &poolSizes=desc->m_descriptorPoolSizes;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(desc->m_numAttachments*desc->m_size);

        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &desc->m_descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool.");
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(desc->m_numAttachments);
        layoutInfo.pBindings = desc->m_descriptorSetBindings.data();

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &desc->m_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout.");
        }
    }
}

void evk::Instance::finish()
{
    for (auto &p : m_evkpipelines)
    {
        Descriptor *desc = &p.m_descriptor;
        desc->m_descriptorSets.resize(desc->m_size);

        // Create descriptor sets.
        for (size_t i = 0; i < desc->m_numAttachments; i++)
        {
            std::vector<VkDescriptorSetLayout> layouts(desc->m_size, desc->m_descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = desc->m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = layouts.data();

            if(vkAllocateDescriptorSets(m_device, &allocInfo, &desc->m_descriptorSets[i])!=VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets.");
            }

            for (auto &set : desc->m_writeDescriptorSet[i])
            {
                set.dstSet=desc->m_descriptorSets[i];
            }

            vkUpdateDescriptorSets(m_device,
                static_cast<uint32_t>(desc->m_writeDescriptorSet[i].size()),
                desc->m_writeDescriptorSet[i].data(), 0, nullptr);
        }
    }
}