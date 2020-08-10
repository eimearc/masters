#include "device.h"

#include "pass.h"

namespace evk {

Device::Framebuffer::Framebuffer(
    Device &device,
    Renderpass &renderpass) // This should be part of attachment creation.
{
    m_device = &device;
    m_framebuffers.resize(device.swapchainSize());
    m_renderpass = &renderpass;
    m_swapchainSize = device.swapchainSize();

    setup();
}

void Device::Framebuffer::recreate()
{
    for (auto &framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device->m_device->m_device, framebuffer, nullptr); // TODO: Tidy
        framebuffer=VK_NULL_HANDLE;
    }
    auto &attachments = m_renderpass->attachments();
    for (auto &a : attachments) a->recreate(*m_device);
    setup();
}

void Device::Framebuffer::setup()
{
    auto &attachments = m_renderpass->attachments();
    const auto &extent = m_device->extent();
    const auto &numAttachments = attachments.size();
    const auto &swapchainImageViews = m_device->swapchainImageViews();

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
        if (vkCreateFramebuffer(m_device->device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) // TODO: Tidy.
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

bool Device::Framebuffer::operator==(const Framebuffer &other) const noexcept
{
    if (m_device!=other.m_device) return false;
    if (!std::equal(
            m_framebuffers.begin(), m_framebuffers.end(),
            other.m_framebuffers.begin()
        )) return false;
    return true;
}

bool Device::Framebuffer::operator!=(const Framebuffer &other) const noexcept
{
    return !(*this==other);
}

Device::Framebuffer::~Framebuffer() noexcept
{
    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device->device(), framebuffer, nullptr);
    }
}

} // namespace evk