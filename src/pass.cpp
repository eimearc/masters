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
    if (m_colorAttachments.size()!=other.m_colorAttachments.size())
        return false;
    if (!std::equal(
            m_colorAttachments.begin(),m_colorAttachments.end(),
            other.m_colorAttachments.begin()))
        return false;
    if (m_colorReferences.size()!=other.m_colorReferences.size())
        return false;
    if (!std::equal(
            m_colorReferences.begin(), m_colorReferences.end(),
            other.m_colorReferences.begin(), referenceEqual))
        return false;

    if (m_depthAttachments.size()!=other.m_depthAttachments.size())
        return false;
    if (!std::equal(
            m_depthAttachments.begin(),m_depthAttachments.end(),
            other.m_depthAttachments.begin()))
        return false;
    if (m_depthReferences.size()!=other.m_depthReferences.size())
        return false;
    if (!std::equal(
            m_depthReferences.begin(), m_depthReferences.end(),
            other.m_depthReferences.begin(), referenceEqual))
        return false;

    if (m_inputAttachments.size()!=other.m_inputAttachments.size())
        return false;
    if (!std::equal(
            m_inputAttachments.begin(),m_inputAttachments.end(),
            other.m_inputAttachments.begin()))
        return false;
    if (m_inputReferences.size()!=other.m_inputReferences.size())
        return false;
    if (!std::equal(
            m_inputReferences.begin(), m_inputReferences.end(),
            other.m_inputReferences.begin(), referenceEqual))
        return false;

    if (m_index!=other.m_index) return false;

    return true;
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

bool Subpass::referenceEqual(
    const VkAttachmentReference &a,
    const VkAttachmentReference &b)
{
    bool result=true;
    result &= (a.attachment == b.attachment);
    result &= (a.layout == b.layout);
    return result;
}

Renderpass::Renderpass(Renderpass &&other) noexcept
{
    *this=std::move(other);
}

Renderpass& Renderpass::operator=(Renderpass &&other) noexcept
{
    if (*this==other) return *this;
    m_attachments=other.m_attachments;
    m_clearValues=other.m_clearValues;
    m_device=other.m_device;
    m_renderPass=other.m_renderPass;
    m_subpasses=other.m_subpasses;
    other.reset();
    return *this;
}

void Renderpass::reset()
{
    m_attachments.resize(0);
    m_clearValues.resize(0);
    m_device=VK_NULL_HANDLE;
    m_renderPass=VK_NULL_HANDLE;
    m_subpasses.resize(0);
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

    attachmentDescriptions.resize(attachments.size()); // TODO: Use a set of all attachments from all subpasses?
    m_clearValues.resize(attachments.size());
    for (const auto &a : attachments)
    {
        attachmentDescriptions[a->index()] = a->description();
        m_clearValues[a->index()] = a->clearValue(); // TODO: Is this needed?
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
    if (m_attachments.size()!=other.m_attachments.size())
        return false;
    if (!std::equal(
            m_attachments.begin(), m_attachments.end(),
            other.m_attachments.begin()))
        return false;
    if (m_device!=other.m_device) return false;
    if (m_renderPass!=other.m_renderPass) return false;
    if (!std::equal(
            m_subpasses.begin(), m_subpasses.end(),
            other.m_subpasses.begin()))
        return false;
    return true;
}


Renderpass::~Renderpass() noexcept
{
    if (m_renderPass!=VK_NULL_HANDLE)
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}