#include "pipeline.h"

Pipeline::Pipeline(
    Descriptor *pDescriptor,
    const VertexInput &i_vertexInput,
    const size_t subpass,
    const VkExtent2D extent,
    // const VkRenderPass &renderPass,
    const Renderpass &renderpass,
    const std::vector<Shader> &shaders,
    const Device &device
)
{
    // m_descriptor = pDescriptor;
    m_vertexInput = i_vertexInput;
    m_subpass = subpass;

    // std::vector<VkDescriptorPoolSize> &poolSizes=m_descriptor->m_descriptorPoolSizes;

    // VkDescriptorPoolCreateInfo poolInfo = {};
    // poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    // poolInfo.pPoolSizes = poolSizes.data();
    // poolInfo.maxSets = static_cast<uint32_t>(m_descriptor->m_swapchainSize);

    // if (vkCreateDescriptorPool(device.m_device, &poolInfo, nullptr, &m_descriptor->m_descriptorPool) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to create descriptor pool.");
    // }

    // std::vector<VkDescriptorSetLayoutBinding> &bindings = m_descriptor->m_descriptorSetBindings;

    // VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    // layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    // layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    // layoutInfo.pBindings = bindings.data();

    // if (vkCreateDescriptorSetLayout(device.m_device, &layoutInfo, nullptr, &m_descriptor->m_descriptorSetLayout) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to create descriptor set layout.");
    // }

    // // Create descriptor sets.
    // const size_t &size = m_descriptor->m_swapchainSize;
    // std::vector<VkDescriptorSetLayout> layouts(size, m_descriptor->m_descriptorSetLayout);
    // VkDescriptorSetAllocateInfo allocInfo = {};
    // allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // allocInfo.descriptorPool = m_descriptor->m_descriptorPool;
    // allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    // allocInfo.pSetLayouts = layouts.data();

    // m_descriptor->m_descriptorSets.resize(size);
    // if(vkAllocateDescriptorSets(device.m_device, &allocInfo, m_descriptor->m_descriptorSets.data())!=VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to allocate descriptor sets.");
    // } 

    // for (size_t i = 0; i < size; i++)
    // {
    //     for (auto &set : m_descriptor->m_writeDescriptorSet[i]) set.dstSet=m_descriptor->m_descriptorSets[i];

    //     vkUpdateDescriptorSets(device.m_device,
    //         static_cast<uint32_t>(m_descriptor->m_writeDescriptorSet[i].size()),
    //         m_descriptor->m_writeDescriptorSet[i].data(), 0, nullptr);
    // }

    m_descriptor = pDescriptor;
    m_descriptor->allocateDescriptorPool();
    m_descriptor->allocateDescriptorSets();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (m_vertexInput.m_bindingDescription.stride>0)
    {
        // Set up input to vertex shader.
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &m_vertexInput.m_bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertexInput.m_attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = m_vertexInput.m_attributeDescriptions.data();
    }

    // Set up input assembly.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Set up the viewport.
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extent.width;
    viewport.height = (float) extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Set up scissor.
    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = extent;

    // Combine viewport and scissor.
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Set up the rasterizer.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Set up multisampling.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Set up colour blending.
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pDescriptor->m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device.m_device, &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout.");
    }

    // Set up depth testing.
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    std::vector<VkPipelineShaderStageCreateInfo> shadersCreateInfo;
    for (const auto &s : shaders) shadersCreateInfo.push_back(s.m_createInfo);

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shadersCreateInfo.size();
    pipelineInfo.pStages = shadersCreateInfo.data();

    // Fixed function stages.
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = renderpass.m_renderPass;
    pipelineInfo.subpass = m_subpass;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // Add the depth stencil.
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device.m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline.");
    }
        
    // for (auto &m : m_shaderModules)
    // {
    //     vkDestroyShaderModule(m_device, m, nullptr);
    // }
}
