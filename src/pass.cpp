#include "pass.h"

Subpass::Subpass(
    const uint32_t index,
    const std::vector<evk::SubpassDependency> &dependencies,
    const std::vector<Attachment*> &colorAttachments,
    const std::vector<Attachment*> &depthAttachments,
    const std::vector<Attachment*> &inputAttachments)
{
    m_index = index;
    for (const auto &d : dependencies) addDependency(d.srcSubpass, d.dstSubpass);
    m_colorAttachments=colorAttachments;
    m_depthAttachments=depthAttachments;
    m_inputAttachments=inputAttachments;

    for (const auto &c : m_colorAttachments) m_colorReferences.push_back(c->colorReference());
    for (const auto &d : m_depthAttachments) m_depthReferences.push_back(d->depthReference());
    for (const auto &i : m_inputAttachments) m_inputReferences.push_back(i->inputReference());

    m_description = {};
    m_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    m_description.colorAttachmentCount=m_colorReferences.size();
    m_description.pColorAttachments=m_colorReferences.data();
    m_description.pDepthStencilAttachment=m_depthReferences.data();
    m_description.inputAttachmentCount=m_inputReferences.size();
    m_description.pInputAttachments=m_inputReferences.data();
}

bool Subpass::operator==(const Subpass &other) const
{
    bool result=true;
    result &= std::equal(
        m_colorAttachments.begin(), m_colorAttachments.end(),
        other.m_colorAttachments.begin()
    );
    result &= std::equal(
        m_depthAttachments.begin(), m_depthAttachments.end(),
        other.m_depthAttachments.begin()
    );
    result &= (m_index==other.m_index);
    result &= std::equal(
        m_inputAttachments.begin(), m_inputAttachments.end(),
        other.m_inputAttachments.begin()
    );
    return result;
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

Renderpass::Renderpass(Renderpass &&other) noexcept
{
    *this=std::move(other);
}

Renderpass& Renderpass::operator=(Renderpass &&other) noexcept
{
    if (*this==other) return *this;
    m_attachments=other.m_attachments;
    other.m_attachments.resize(0);
    m_clearValues=other.m_clearValues;
    other.m_clearValues.resize(0);
    m_device=other.m_device;
    other.m_device=VK_NULL_HANDLE;
    m_renderPass=other.m_renderPass;
    other.m_renderPass=VK_NULL_HANDLE;
    m_subpasses=other.m_subpasses;
    other.m_subpasses.resize(0);
    return *this;
}

Renderpass::Renderpass(
    const Device &device,
    const std::vector<Attachment*> &attachments,
    std::vector<Subpass*> &subpasses)
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
        attachmentDescriptions[a->index()] = a->description();
        m_clearValues[a->index()] = a->clearValue();
    }

    for (const auto &sp : subpasses) subpassDescriptions.push_back(sp->description()); 

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
        for (const auto &d : sp->dependencies()) dependencies.push_back(d);
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


bool Renderpass::operator==(const Renderpass &other) const
{
    bool result=true;
    result &= std::equal(
        m_attachments.begin(), m_attachments.end(),
        other.m_attachments.begin()
    );
    result &= (m_device==other.m_device);
    result &= (m_renderPass==other.m_renderPass);
    result &= std::equal(
        m_subpasses.begin(), m_subpasses.end(),
        other.m_subpasses.begin()
    );
    return result;
}


Renderpass::~Renderpass() noexcept
{
    if (m_renderPass!=VK_NULL_HANDLE)
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}