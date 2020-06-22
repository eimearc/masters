#include "evulkan_core.h"

#include <set>
#include <iostream>

void evk::Instance::addVertexAttributeVec3(const uint32_t &location, const uint32_t &offset)
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset=offset;
    m_attributeDescriptions.push_back(desc);
}

void evk::Instance::addVertexAttributeVec2(const uint32_t &location, const uint32_t &offset)
{
    VkVertexInputAttributeDescription desc;
    desc.binding=0;
    desc.location=location;
    desc.format=VK_FORMAT_R32G32_SFLOAT;
    desc.offset=offset;
    m_attributeDescriptions.push_back(desc);
}

void evk::Instance::setBindingDescription(uint32_t stride)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_bindingDescription=bindingDescription;
}

void evk::Instance::createCommandPools()
{
    m_commandPools.resize(m_numThreads);
    for (auto &commandPool : m_commandPools)
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool.");
        }
    }
}

void evk::Instance::createImageView(const ImageViewCreateInfo *pCreateInfo, VkImageView *pImageView)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = pCreateInfo->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = pCreateInfo->format;
    viewInfo.subresourceRange.aspectMask = pCreateInfo->aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &viewInfo, nullptr, pImageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
}

void evk::Instance::addSubpass(
    const std::vector<VkAttachmentReference> &colorAttachments,
    const std::vector<VkAttachmentReference> &inputAttachments)
{
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount=colorAttachments.size();
    subpass.pColorAttachments=colorAttachments.data();
    subpass.inputAttachmentCount=inputAttachments.size();
    subpass.pInputAttachments=inputAttachments.data();
    m_subpasses.push_back(subpass);
}

void evk::Instance::addAttachment(VkFormat format, VkImageLayout finalLayout)
{
    VkAttachmentDescription attachment = {};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = finalLayout;
    m_attachments.push_back(attachment);
}

void evk::Instance::addDependency(uint32_t srcSubpass, uint32_t dstSubpass)
{
    VkSubpassDependency dependency;
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    m_dependencies.push_back(dependency);
}

void evk::Instance::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;

    addAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    addAttachment(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Swap chain image color attachment
    VkAttachmentDescription backBufferAttachment = {};
    backBufferAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    backBufferAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    backBufferAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    backBufferAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    backBufferAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    backBufferAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    backBufferAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    backBufferAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments.push_back(backBufferAttachment);

    for (auto &a : m_attachments) attachments.push_back(a);



    // Subpasses

    // References.
    std::vector<VkAttachmentReference> colorAttachments = {{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }};
    std::vector<VkAttachmentReference> inputAttachments;
    inputAttachments.push_back({ 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    inputAttachments.push_back({ 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    addSubpass(colorAttachments,inputAttachments);

    // First subpass - fill colour and depth.
    VkAttachmentReference colorReference = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount=1;
    subpass.pColorAttachments=&colorReference;
    subpass.pDepthStencilAttachment =&depthReference;
    subpasses.push_back(subpass);

    // Add other subpasses.
    for (auto &sp : m_subpasses) subpasses.push_back(sp);


    // Dependencies

    addDependency(0,1); // Add dependency between stage 0 and stage 1.

    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies.push_back(dependency);

    for (auto &dep : m_dependencies) dependencies.push_back(dep);

    dependency={};
    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies.push_back(dependency);



    // Create Render Pass

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass.");
    }
}

VkFormat evk::Instance::findDepthFormat(
    const EVkRenderPassCreateInfo *pCreateInfo)
{
    return findSupportedFormat(
        pCreateInfo,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat evk::Instance::findSupportedFormat(
    const EVkRenderPassCreateInfo *pCreateInfo,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(pCreateInfo->physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format.");
}

void evk::Instance::registerVertexShader(const std::string &vertShader)
{   
    auto vertShaderCode = readFile(vertShader);
    VkShaderModule vertShaderModule;
    createShaderModule(m_device, vertShaderCode, &vertShaderModule);
    m_shaderModules.push_back(vertShaderModule);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    m_shaders.push_back(vertShaderStageInfo);
}

void evk::Instance::registerFragmentShader(const std::string &fragShader)
{   
    auto fragShaderCode = readFile(fragShader);
    VkShaderModule fragShaderModule;
    createShaderModule(m_device, fragShaderCode, &fragShaderModule);
    m_shaderModules.push_back(fragShaderModule);
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    m_shaders.push_back(fragShaderStageInfo);
}

void evk::Instance::createGraphicsPipeline()
{
    // Set up input to vertex shader.
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &m_bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();

    // Set up input assembly.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Set up the viewport.
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_swapChainExtent.width;
    viewport.height = (float) m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Set up scissor.
    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = m_swapChainExtent;

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
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_graphicsPipelineLayout) != VK_SUCCESS)
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

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = m_shaders.size();
    pipelineInfo.pStages = m_shaders.data();

    // Fixed function stages.
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    
    // Pipeline layout.
    pipelineInfo.layout = m_graphicsPipelineLayout;

    // Render pass and sub-pass.
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0; // TODO: Allow subpasses.

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // Add the depth stencil.
    pipelineInfo.pDepthStencilState = &depthStencil;

    m_pipelines.resize(2);
    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipelines[0]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline.");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo2 = pipelineInfo; 
    pipelineInfo2.subpass=1;
    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo2, nullptr, &m_pipelines[1]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline.");
    } 
        
    for (auto &m : m_shaderModules)
    {
        vkDestroyShaderModule(m_device, m, nullptr);
    }
}

void evk::Instance::createDepthResources()
{
    EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = m_swapChainImageFormat;
    renderPassInfo.physicalDevice = m_physicalDevice;
    VkFormat depthFormat = findDepthFormat(&renderPassInfo);

    ImageCreateInfo createInfo;
    createInfo.width = m_swapChainExtent.width;
    createInfo.height = m_swapChainExtent.height;
    createInfo.format = depthFormat;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    createInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createImage(&createInfo, &m_depthImage, &m_depthImageMemory);
    ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.image=m_depthImage;
    imageViewCreateInfo.format=depthFormat;
    imageViewCreateInfo.aspectFlags=VK_IMAGE_ASPECT_DEPTH_BIT;
    createImageView(&imageViewCreateInfo, &m_depthImageView);

    // Create gBuffer TODO: move
    createInfo={};
    createInfo.width = m_swapChainExtent.width;
    createInfo.height = m_swapChainExtent.height;
    createInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createImage(&createInfo, &m_backBufferImage, &m_backBufferMemory);
    imageViewCreateInfo={};
    imageViewCreateInfo.image=m_backBufferImage;
    imageViewCreateInfo.format=VK_FORMAT_R8G8B8A8_UNORM;
    imageViewCreateInfo.aspectFlags=VK_IMAGE_ASPECT_COLOR_BIT;
    createImageView(&imageViewCreateInfo, &m_backBufferView);
}

void evk::Instance::createImage(
    const ImageCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = pCreateInfo->width;
    imageInfo.extent.height = pCreateInfo->height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = pCreateInfo->format;
    imageInfo.tiling = pCreateInfo->tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = pCreateInfo->usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(m_device, &imageInfo, nullptr, pImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        m_physicalDevice,
        memRequirements.memoryTypeBits,
        pCreateInfo->properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, pImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory.");
    }
    vkBindImageMemory(m_device, *pImage, *pImageMemory, 0);
}

void evk::Instance::createFramebuffers()
{
    const size_t numImages = m_swapChainImages.size();
    m_framebuffers.resize(numImages);

    for (size_t i = 0; i < numImages; i++)
    {
        std::array<VkImageView,3> attachments =
        {
            m_swapChainImageViews[i],
            m_backBufferView,
            m_depthImageView       
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &(m_framebuffers)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer.");
        }
    }
}

void evk::Instance::cleanup()
{
    if (vkDeviceWaitIdle(m_device)!=VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for vkDeviceWaitIdle");
    }
    vkDestroyImageView(m_device, m_depthImageView, nullptr);
    vkDestroyImage(m_device, m_depthImage, nullptr);
    vkFreeMemory(m_device, m_depthImageMemory, nullptr);

    vkDestroyImageView(m_device, m_backBufferView, nullptr);
    vkDestroyImage(m_device, m_backBufferImage, nullptr);
    vkFreeMemory(m_device, m_backBufferMemory, nullptr);

    vkDestroySampler(m_device, m_textureSampler, nullptr);
    vkDestroyImageView(m_device, m_textureImageView, nullptr);
    vkDestroyImage(m_device, m_textureImage, nullptr); // TODO: Ensure only happens when texture is there.
    vkFreeMemory(m_device, m_textureImageMemory, nullptr);

    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    vkDestroyPipeline(m_device, m_pipelines[0], nullptr);
    vkDestroyPipeline(m_device, m_pipelines[1], nullptr);
    vkDestroyPipelineLayout(m_device, m_graphicsPipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    for (auto &buffer : m_buffers) vkDestroyBuffer(m_device, buffer, nullptr);
    for (auto &memory : m_bufferMemories) vkFreeMemory(m_device, memory, nullptr);

    for (size_t i = 0; i < m_maxFramesInFlight; ++i)
    {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_fencesInFlight[i], nullptr);
    }

    for (int i = 0; i < m_commandPools.size(); ++i)
    {
        vkFreeCommandBuffers(m_device, m_commandPools[i], 1, &m_secondaryCommandBuffers[i]);
        vkDestroyCommandPool(m_device, m_commandPools[i], nullptr);
    }

    vkDestroyDevice(m_device, nullptr);

    if (m_validationLayers.size() > 0)
    {
        DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}