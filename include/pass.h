#ifndef PASS
#define PASS

#include <vulkan/vulkan.h>
#include "evulkan_util.h"
#include "attachment.h"
#include <vector>

class Subpass
{
    public:
    Subpass()=default;
    Subpass(
        const std::vector<evk::SubpassDependency> &dependencies,
        const std::vector<Attachment> &colorAttachments,
        const std::vector<Attachment> &depthAttachments,
        const std::vector<Attachment> &inputAttachments
    );

    VkSubpassDescription m_description;
    std::vector<VkSubpassDependency> m_dependencies;
    std::vector<VkAttachmentReference> m_colorReferences;
    std::vector<VkAttachmentReference> m_depthReferences;
    std::vector<VkAttachmentReference> m_inputReferences;

    private:
    void addDependency(uint32_t srcSubpass, uint32_t dstSubpass);
    std::vector<Attachment> m_colorAttachments;
    std::vector<Attachment> m_depthAttachments;
    std::vector<Attachment> m_inputAttachments;
};

class Renderpass
{
    public:
    Renderpass()=default;
    Renderpass(
        const std::vector<Attachment> &attachments,
        const std::vector<Subpass> &subpasses,
        const Device &device
    );
    void destroy();

    VkRenderPass m_renderPass;
    std::vector<Subpass> m_subpasses;

    private:
    VkDevice m_device;
};

#endif