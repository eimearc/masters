#include "framebuffer.h"

#include "swapchain.h"

Framebuffer::Framebuffer(
    const Device &device,
    const Renderpass &renderpass,
    const Swapchain &swapchain) // This should be part of attachment creation.
{
    m_device = device.m_device;
    const size_t swapchainSize = swapchain.m_images.size();
    m_framebuffers.resize(swapchainSize);

    // For each image in the swapchain.
    for (size_t i = 0; i < swapchainSize; i++)
    {
        std::vector<VkImageView> imageViews(renderpass.m_attachments.size());
        imageViews[0] = swapchain.m_imageViews[i];
        for (size_t j = 1; j < renderpass.m_attachments.size(); j++)
        {
            auto attachment = renderpass.m_attachments[j];
            const uint32_t &index = attachment.m_index;
            imageViews[index]=attachment.m_imageView;
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass.m_renderPass;
        framebufferInfo.attachmentCount = imageViews.size();
        framebufferInfo.pAttachments = imageViews.data();
        framebufferInfo.width = swapchain.m_extent.width;
        framebufferInfo.height = swapchain.m_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &(m_framebuffers)[i]) != VK_SUCCESS)
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