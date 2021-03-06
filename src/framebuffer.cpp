#include "device.h"

#include "evk_assert.h"
#include "pass.h"

namespace evk {

Device::Framebuffer::Framebuffer(
    Device &device,
    Renderpass &renderpass
)
{
    m_device = &device;
    m_framebuffers.resize(device.swapchainSize());
    m_renderpass = &renderpass;
    m_swapchainSize = device.swapchainSize();

    setup();
}

void Device::Framebuffer::recreate() noexcept
{
    for (auto &framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device->device(), framebuffer, nullptr);
        framebuffer=VK_NULL_HANDLE;
    }
    auto &attachments = m_renderpass->attachments();
    for (auto &a : attachments) a->recreate(*m_device);
    setup();
}

void Device::Framebuffer::setup() noexcept
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
        auto result =  vkCreateFramebuffer(
            m_device->device(), &framebufferInfo, nullptr, &framebuffer
        );
        EVK_ASSERT(result,"failed to create framebuffer");
    }
}

Device::Framebuffer::Framebuffer(Framebuffer &&other) noexcept
{
    *this=std::move(other);
}

Device::Framebuffer& Device::Framebuffer::operator=(
    Framebuffer &&other
) noexcept
{
    if (*this==other) return *this;
    m_device=other.m_device;
    m_framebuffers=other.m_framebuffers;
    m_renderpass=other.m_renderpass;
    m_swapchainSize=other.m_swapchainSize;
    other.reset();
    return *this;
}

void Device::Framebuffer::reset() noexcept
{
    m_device=VK_NULL_HANDLE;
    m_framebuffers.resize(0);
    m_renderpass=nullptr;
    m_swapchainSize=0;
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