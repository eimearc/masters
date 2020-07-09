#ifndef PASS
#define PASS

#include <vulkan/vulkan.h>
#include "util.h"
#include "attachment.h"
#include <vector>

class Subpass
{
    public:
    Subpass()=default;
    Subpass(
        const uint32_t index,
        const std::vector<evk::SubpassDependency> &dependencies,
        const std::vector<Attachment> &colorAttachments,
        const std::vector<Attachment> &depthAttachments,
        const std::vector<Attachment> &inputAttachments
    );

    uint32_t m_index;
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
    std::vector<VkClearValue> m_clearValues;
    std::vector<Attachment> m_attachments;

    private:
    VkDevice m_device;
};

#endif