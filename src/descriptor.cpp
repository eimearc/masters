#include "evulkan_core.h"

void evk::Instance::createDescriptorSets()
{
    // Create descriptor pool.
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size());

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool.");
    }

    // Create descriptor set layout.
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }

    // Create descriptor sets.
    const size_t &size = m_swapChainImages.size();
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
        // Set up the UBO for the shader.
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

// void evkCreateDescriptorPool(
//     VkDevice device,
//     const EVkDescriptorPoolCreateInfo *pCreateInfo,
//     VkDescriptorPool *pDescriptorPool
// )
// {
//     std::array<VkDescriptorPoolSize, 2> poolSizes = {};
//     poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     poolSizes[0].descriptorCount = static_cast<uint32_t>(pCreateInfo->swapchainImages.size());
//     poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//     poolSizes[1].descriptorCount = static_cast<uint32_t>(pCreateInfo->swapchainImages.size());

//     VkDescriptorPoolCreateInfo poolInfo = {};
//     poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//     poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//     poolInfo.pPoolSizes = poolSizes.data();
//     poolInfo.maxSets = static_cast<uint32_t>(pCreateInfo->swapchainImages.size());

//     if (vkCreateDescriptorPool(device, &poolInfo, nullptr, pDescriptorPool) != VK_SUCCESS)
//     {
//         throw std::runtime_error("failed to create descriptor pool.");
//     }
// }

// void evkCreateDescriptorSets(
//     VkDevice device,
//     const EVkDescriptorSetCreateInfo *pCreateInfo,
//     std::vector<VkDescriptorSet> *pDescriptorSets
// )
// {
//     const size_t &size = pCreateInfo->swapchainImages.size();
//     std::vector<VkDescriptorSetLayout> layouts(size, pCreateInfo->descriptorSetLayout);
//     VkDescriptorSetAllocateInfo allocInfo = {};
//     allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//     allocInfo.descriptorPool = pCreateInfo->descriptorPool;
//     allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
//     allocInfo.pSetLayouts = layouts.data();

//     pDescriptorSets->resize(size);
//     if(vkAllocateDescriptorSets(device, &allocInfo, pDescriptorSets->data())!=VK_SUCCESS)
//     {
//         throw std::runtime_error("failed to allocate descriptor sets.");
//     }

//     for (size_t i = 0; i < size; i++)
//     {
//         // Set up the UBO for the shader.
//         VkDescriptorBufferInfo bufferInfo = {};
//         bufferInfo.buffer = pCreateInfo->uniformBuffers[i];
//         bufferInfo.offset = 0;
//         bufferInfo.range = sizeof(UniformBufferObject);

//         std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

//         descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[0].dstSet = (*pDescriptorSets)[i];
//         descriptorWrites[0].dstBinding = 0;
//         descriptorWrites[0].dstArrayElement = 0;
//         descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         descriptorWrites[0].descriptorCount = 1;
//         descriptorWrites[0].pBufferInfo = &bufferInfo;

//         vkUpdateDescriptorSets(device,
//             static_cast<uint32_t>(descriptorWrites.size()),
//             descriptorWrites.data(), 0, nullptr);
//     }
// }