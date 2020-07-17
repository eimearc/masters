#include "pass.h"

Subpass::Subpass(
    const uint32_t index,
    const std::vector<evk::SubpassDependency> &dependencies,
    const std::vector<Attachment> &colorAttachments,
    const std::vector<Attachment> &depthAttachments,
    const std::vector<Attachment> &inputAttachments)
{
    m_index = index;
    for (const auto &d : dependencies) addDependency(d.srcSubpass, d.dstSubpass);
    m_colorAttachments=colorAttachments;
    m_depthAttachments=depthAttachments;
    m_inputAttachments=inputAttachments;

    for (const auto &c : m_colorAttachments) m_colorReferences.push_back(c.m_colorReference);
    for (const auto &d : m_depthAttachments) m_depthReferences.push_back(d.m_depthReference);
    for (const auto &i : m_inputAttachments) m_inputReferences.push_back(i.m_inputReference);

    m_description = {};
    m_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    m_description.colorAttachmentCount=m_colorReferences.size();
    m_description.pColorAttachments=m_colorReferences.data();
    m_description.pDepthStencilAttachment=m_depthReferences.data();
    m_description.inputAttachmentCount=m_inputReferences.size();
    m_description.pInputAttachments=m_inputReferences.data();
}

void Subpass::addDependency(uint32_t srcSubpass, uint32_t dstSubpass)
{
    VkSubpassDependency dependency;
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    m_dependencies.push_back(dependency);
}

Renderpass::Renderpass(
    const Device &device,
    const std::vector<Attachment> &attachments,
    const std::vector<Subpass> &subpasses)
{
    m_device = device.device();
    m_subpasses = subpasses;
    m_attachments = attachments;

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpassDescriptions;

    attachmentDescriptions.resize(attachments.size()); // Use a set of all attachments from all subpasses?
    m_clearValues.resize(attachments.size());
    for (const auto &a : attachments)
    {
        attachmentDescriptions[a.m_index] = a.m_description;
        m_clearValues[a.m_index] = a.m_clearValue;
    }

    for (const auto &sp : subpasses) subpassDescriptions.push_back(sp.m_description); 

    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies.push_back(dependency);

    for (const auto &sp : subpasses)
    {
        for (const auto &d : sp.m_dependencies) dependencies.push_back(d);
    }

    // Create Render Pass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachmentDescriptions.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();
    if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass.");
    }
}

Renderpass::~Renderpass() noexcept
{
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}