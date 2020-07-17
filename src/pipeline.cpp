#include "pipeline.h"

Pipeline::Pipeline(
    Device &device,
    const Subpass &subpass,
    Descriptor *pDescriptor,
    const VertexInput &vertexInput,
    Renderpass *pRenderpass,
    const std::vector<Shader> &shaders,
    bool writeDepth
)
{
    m_vertexInput = vertexInput;
    m_subpass = subpass.m_index;
    m_descriptor = pDescriptor;
    m_shaders = shaders;
    m_renderpass = pRenderpass;
    m_writeDepth=writeDepth;

    setup(device);
}

void Pipeline::setup(Device &device)
{
    m_device = device.device();

    m_descriptor->allocateDescriptorPool();
    m_descriptor->allocateDescriptorSets();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (m_vertexInput.m_bindingDescription.stride>0) // TODO: remove this?
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

    const auto &extent = device.extent(); 

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Set up multisampling.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};// TODO: Must equal colorAttachmentCount.
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = 0xf;
    // if (m_writeDepth) // GBuffer
    // {
    //     colorBlendAttachment.blendEnable = VK_FALSE;
    //     colorBlendAttachment.colorWriteMask = 0xf;
    // }
    // else // Lighting
    // {
    //     colorBlendAttachment.blendEnable = VK_TRUE; // TODO: switch this back on when blending.
    //     colorBlendAttachment.colorWriteMask = 0xf;
    //     colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    //     colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO: Should be one?
    //     colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // TODO: Configure depending on operation.
    //     colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    //     colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO: Should be one?
    //     colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;   
    // }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    const auto &setLayouts = m_descriptor->setLayouts();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout.");
    }

    // Set up depth testing.
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    if (m_writeDepth) // G-Buffer
    {
        depthStencil.depthTestEnable = true;
        depthStencil.depthWriteEnable = true;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = false;
        depthStencil.stencilTestEnable = true;
        depthStencil.front.passOp = VK_STENCIL_OP_REPLACE;
        depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencil.front.compareMask = 0xff;
        depthStencil.front.writeMask = 0xff;
        depthStencil.front.reference = 1;
        depthStencil.back = depthStencil.front;
    }
    else // Lighting
    {
        depthStencil.depthTestEnable = true;
        depthStencil.depthWriteEnable = false;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = false;
        depthStencil.stencilTestEnable = true;
        depthStencil.front.passOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencil.front.compareOp = VK_COMPARE_OP_EQUAL;
        depthStencil.front.compareMask = 0xff;
        depthStencil.front.writeMask = 0x0;
        depthStencil.front.reference = 1;
        depthStencil.back = depthStencil.front;
    }
    

    std::vector<VkPipelineShaderStageCreateInfo> shadersCreateInfo;
    for (const auto &s : m_shaders) shadersCreateInfo.push_back(s.m_createInfo);

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shadersCreateInfo.size();
    pipelineInfo.pStages = shadersCreateInfo.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = m_renderpass->renderpass();
    pipelineInfo.subpass = m_subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline.");
    }
}

void Pipeline::destroy()
{   
    vkDestroyPipelineLayout(m_device, m_layout, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
}