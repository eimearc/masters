#include "framebuffer.h"

Framebuffer::Framebuffer(
    const Device &device,
    const Renderpass &renderpass) // This should be part of attachment creation.
{
    m_device = device.device();
    const auto &attachments = renderpass.attachments();
    const auto &numAttachments = attachments.size();
    const auto &swapchainSize = device.swapchainSize();
    const auto &swapchainImageViews = device.swapchainImageViews();
    const auto &extent = device.extent();
    m_framebuffers.resize(swapchainSize);

    std::vector<VkImageView> imageViews(numAttachments);
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderpass.renderpass();
    framebufferInfo.attachmentCount = numAttachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < swapchainSize; ++i)
    {
        imageViews[0] = swapchainImageViews[i];
        for (size_t j = 1; j < numAttachments; j++)
        {
            const auto &attachment = attachments[j];
            const uint32_t &index = attachment->m_index;
            imageViews[index]=attachment->m_imageView;
        }
        framebufferInfo.pAttachments = imageViews.data();
        auto &framebuffer = m_framebuffers[i];
        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer.");
        }
    }
}

void Framebuffer::destroy()
{
    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
}