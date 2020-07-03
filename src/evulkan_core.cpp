#include "evulkan_core.h"

#include <set>
#include <iostream>

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

void createImage(
    const VkDevice &device,
    const VkPhysicalDevice &physicalDevice,
    const VkExtent2D &extent,
    const VkFormat &format,
    const VkImageTiling &tiling,
    const VkImageUsageFlags &usage,
    const VkMemoryPropertyFlags &properties,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, pImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice,
        memRequirements.memoryTypeBits,
        properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, pImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory.");
    }
    vkBindImageMemory(device, *pImage, *pImageMemory, 0);
}

void createImageView(
    const VkDevice &device,
    const VkImage &image,
    const VkFormat &format,
    const VkImageAspectFlags &aspectMask,
    VkImageView *pImageView)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, pImageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
}

void evk::Instance::createFramebuffers(
    const std::vector<Attachment> &attachments,
    const Renderpass &renderpass,
    const Swapchain &swapchain) // This should be part of attachment creation.
{
    const size_t numImages = swapchain.m_swapChainImages.size();
    m_framebuffers.resize(numImages);

    for (size_t i = 0; i < numImages; i++)
    {
        std::vector<VkImageView> imageViews(attachments.size());
        for (const auto &a : attachments)
        {
            const uint32_t &index = a.m_index;
            imageViews[index]=a.m_imageViews[i];
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass.m_renderPass;
        framebufferInfo.attachmentCount = imageViews.size();
        framebufferInfo.pAttachments = imageViews.data();
        framebufferInfo.width = swapchain.m_swapChainExtent.width;
        framebufferInfo.height = swapchain.m_swapChainExtent.height;
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

    // auto &attachment = m_evkattachments[evk::FRAMEBUFFER_ATTACHMENT];
    // for (auto &view : attachment.imageViews) vkDestroyImageView(m_device, view, nullptr);

    // for (auto &a : m_evkattachments)
    // {
    //     attachment = a.second;
    //     if (a.first != evk::FRAMEBUFFER_ATTACHMENT)
    //     {
    //         for (auto &view : attachment.imageViews) vkDestroyImageView(m_device, view, nullptr);
    //         for (auto &image : attachment.images) vkDestroyImage(m_device, image, nullptr);
    //         for (auto &memory : attachment.imageMemories) vkFreeMemory(m_device, memory, nullptr); 
    //     }
    // }

    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    // for (auto &pipeline : m_pipelines) vkDestroyPipeline(m_device, pipeline, nullptr);
    // for (auto &layout : m_pipelineLayouts) vkDestroyPipelineLayout(m_device, layout, nullptr);

    // vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    // vkDestroySwapchainKHR(m_device, swapchain.m_swapChain, nullptr);

    // for (auto &buffer : m_buffers) vkDestroyBuffer(m_device, buffer, nullptr);
    // for (auto &memory : m_bufferMemories) vkFreeMemory(m_device, memory, nullptr);

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

    // vkDestroyDevice(m_device, nullptr);

    // if (m_validationLayers.size() > 0)
    // {
    //     DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    // }

    // vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
    // vkDestroyInstance(m_vkInstance, nullptr);

    // glfwDestroyWindow(m_window);
    // glfwTerminate();
}