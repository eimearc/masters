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

void evk::Instance::createImageView(const ImageViewCreateInfo *pCreateInfo, VkImageView *pImageView)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = pCreateInfo->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = pCreateInfo->format;
    viewInfo.subresourceRange.aspectMask = pCreateInfo->aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &viewInfo, nullptr, pImageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
}

VkFormat evk::Instance::findDepthFormat(
    const EVkRenderPassCreateInfo *pCreateInfo)
{
    return findSupportedFormat(
        pCreateInfo,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat evk::Instance::findSupportedFormat(
    const EVkRenderPassCreateInfo *pCreateInfo,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(pCreateInfo->physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format.");
}

void evk::Instance::createImage(
    const ImageCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = pCreateInfo->width;
    imageInfo.extent.height = pCreateInfo->height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = pCreateInfo->format;
    imageInfo.tiling = pCreateInfo->tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = pCreateInfo->usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(m_device, &imageInfo, nullptr, pImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        m_physicalDevice,
        memRequirements.memoryTypeBits,
        pCreateInfo->properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, pImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory.");
    }
    vkBindImageMemory(m_device, *pImage, *pImageMemory, 0);
}

void evk::Instance::createFramebuffers(const std::vector<Attachment> &attachments, const Renderpass &renderpass) // This should be part of attachment creation.
{
    const size_t numImages = m_swapChainImages.size();
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
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
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

    vkDestroySampler(m_device, m_textureSampler, nullptr);
    vkDestroyImageView(m_device, m_textureImageView, nullptr);
    vkDestroyImage(m_device, m_textureImage, nullptr); // TODO: Ensure only happens when texture is there.
    vkFreeMemory(m_device, m_textureImageMemory, nullptr);

    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    // for (auto &pipeline : m_pipelines) vkDestroyPipeline(m_device, pipeline, nullptr);
    // for (auto &layout : m_pipelineLayouts) vkDestroyPipelineLayout(m_device, layout, nullptr);

    // vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

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