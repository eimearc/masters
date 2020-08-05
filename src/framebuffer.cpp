#include "device.h"

#include "pass.h"

namespace evk {

Device::Framebuffer::Framebuffer(
    const VkDevice &device,
    size_t swapchainSize,
    const std::vector<VkImageView> &swapchainImageViews,
    VkExtent2D extent,
    Renderpass &renderpass) // This should be part of attachment creation.
{
    m_device = device;
    m_framebuffers.resize(swapchainSize);
    m_renderpass = &renderpass;
    m_swapchainSize = swapchainSize;

    setup(extent, swapchainImageViews);
}

void Device::Framebuffer::recreate(
    VkExtent2D extent,
    const std::vector<VkImageView> &swapchainImageViews
)
{
    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    setup(extent, swapchainImageViews);
}

void Device::Framebuffer::setup(
    VkExtent2D extent,
    const std::vector<VkImageView> &swapchainImageViews
)
{
    const auto &attachments = m_renderpass->attachments();
    const auto &numAttachments = attachments.size();

    std::vector<VkImageView> imageViews(numAttachments);
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderpass->renderpass();
    framebufferInfo.attachmentCount = numAttachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < m_swapchainSize; ++i)
    {
        imageViews[0] = swapchainImageViews[i];
        for (size_t j = 1; j < numAttachments; j++)
        {
            const auto &attachment = attachments[j];
            const uint32_t &index = attachment->index();
            imageViews[index]=attachment->view();
        }
        framebufferInfo.pAttachments = imageViews.data();
        auto &framebuffer = m_framebuffers[i];
        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer.");
        }
    }
}

Device::Framebuffer::Framebuffer(Framebuffer &&other) noexcept
{
    *this=std::move(other);
}

Device::Framebuffer& Device::Framebuffer::operator=(Framebuffer &&other) noexcept
{
    if (*this==other) return *this;
    m_device=other.m_device;
    other.m_device=VK_NULL_HANDLE;
    m_framebuffers=other.m_framebuffers;
    other.m_framebuffers.resize(0);
    return *this;
}

bool Device::Framebuffer::operator==(const Framebuffer &other)
{
    bool result = true;
    result &= (m_device==other.m_device);
    result &= std::equal(
        m_framebuffers.begin(), m_framebuffers.end(),
        other.m_framebuffers.begin()
    );
    return result;
}

Device::Framebuffer::~Framebuffer() noexcept
{
    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
}

} // namespace evk