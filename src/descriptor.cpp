#include "evulkan_core.h"

void evkCreateDescriptorPool(
    VkDevice device,
    const EVkDescriptorPoolCreateInfo *pCreateInfo,
    VkDescriptorPool *pDescriptorPool
)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(pCreateInfo->swapchainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(pCreateInfo->swapchainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(pCreateInfo->swapchainImages.size());

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, pDescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

void evkCreateDescriptorSets(
    VkDevice device,
    const EVkDescriptorSetCreateInfo *pCreateInfo,
    std::vector<VkDescriptorSet> *pDescriptorSets
)
{
    const size_t &size = pCreateInfo->swapchainImages.size();
    std::vector<VkDescriptorSetLayout> layouts(size, pCreateInfo->descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pCreateInfo->descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    pDescriptorSets->resize(size);
    if(vkAllocateDescriptorSets(device, &allocInfo, pDescriptorSets->data())!=VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets.");
    }

    for (size_t i = 0; i < size; i++)
    {
        // Set up the UBO for the shader.
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = pCreateInfo->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = (*pDescriptorSets)[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}