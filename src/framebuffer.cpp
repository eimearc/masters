#include "framebuffer.h"

Framebuffer::Framebuffer(
    const Device &device,
    const Renderpass &renderpass) // This should be part of attachment creation.
{
    m_device = device.device();
    const size_t &swapchainSize = device.swapchainSize();
    const auto &swapchainImageViews = device.swapchainImageViews();
    const auto &extent = device.extent();
    m_framebuffers.resize(swapchainSize);

    // For each image in the swapchain.
    for (size_t i = 0; i < swapchainSize; i++)
    {
        std::vector<VkImageView> imageViews(renderpass.m_attachments.size());
        // imageViews[0] = swapchain.m_imageViews[i];
        imageViews[0] = swapchainImageViews[i];
        std::cout << imageViews[0] << std::endl;
        std::cout << renderpass.m_attachments.size() << std::endl;
        for (size_t j = 1; j < renderpass.m_attachments.size(); j++)
        {
            auto attachment = renderpass.m_attachments[j];
            const uint32_t &index = attachment.m_index;
            imageViews[index]=attachment.m_imageView;
            std::cout << "Other image view: " << imageViews[index] << std::endl;
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass.m_renderPass;
        framebufferInfo.attachmentCount = imageViews.size();
        framebufferInfo.pAttachments = imageViews.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        std::cout << imageViews.size() << std::endl;
        for (const auto &v: imageViews) std::cout << "\tImageView: " << v << std::endl;

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